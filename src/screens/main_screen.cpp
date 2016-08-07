#include "main_screen.h"

#include "../engine.h"
#include "../state.h"
#include "../util.h"
#include "pause_menu.h"

#include <SDL.h>

#include <functional>
#include <iostream>
#include <set>

MainScreen::MainScreen() {
  heroHealth_.setMax(100);
  heroHealth_.setValue(70);
  heroHealth_.setDimensions(16, 16, 64, 16);

  // Load the game script
  GameState::lua().Load("assets/scripts/game.lua");
  GameState::lua()["init"]();
  GameState::markInitialized();
}

bool MainScreen::fixMovement(const std::unique_ptr<Sprite> &sprite,
                             sf::Vector2f moveDelta) {
  auto dim = sprite->getDimensions();
  auto oldDim = dim;
  dim.left += moveDelta.x;
  dim.top += moveDelta.y;
  if (!GameState::positionWalkable(sprite, dim)) {
    dim.left -= moveDelta.x;
  }
  if (!GameState::positionWalkable(sprite, dim)) {
    dim.left += moveDelta.x;
    dim.top -= moveDelta.y;
  }
  sprite->setDimensions(dim);
  return dim != oldDim;
}

sf::FloatRect MainScreen::updateGravity(const std::unique_ptr<Sprite> &sprite) {
  auto dim = sprite->getDimensions();
  if (GameState::map()->isLadder(dim)) {
    sprite->zeroVelocity(/*stopJump = */ true);
  } else {
    // Falling
    sprite->updateVelocity();
    float positionOfBelow = GameState::map()->positionOfTileBelow(dim);
    float positionOfAbove = GameState::map()->positionOfTileAbove(dim);

    sf::Vector2f jumpDelta(0, sprite->velocity());
    dim = sprite->getDimensions();
    float top = dim.top;
    top += jumpDelta.y;
    bool clamped = util::clamp<float>(top, positionOfAbove, positionOfBelow);
    if (clamped) {
      sprite->zeroVelocity(true);
    }
    if ((int)top == (int)positionOfBelow) {
      sprite->allowJump();
    } else {
      sprite->forbidJump();
    }
    dim.top = top;
  }
  return dim;
}

void MainScreen::handleEvent(sf::Event &event) {
  visual::DialogManager::handleEvent(event);

  if (event.type == sf::Event::KeyPressed) {
    if (event.key.code == sf::Keyboard::P) {
      Engine::pushScreen(new PauseMenuScreen());
    } else if (event.key.code == sf::Keyboard::T) {
      heroHealth_.shrink(5);
    }
  }
}

bool MainScreen::update(sf::Time &time) {
  // Used for getting ticks in Lua
  time_ = time;

  GameState::map()->update(time_);
  GameState::hero()->update(time_);
  for (const auto &sprite : GameState::sprites()) {
    sprite.second->update(time_);
  }
  GameState::lua()["update"]();

  if (visual::DialogManager::update(time_)) {
    return true;
  } else {
    const auto &dialog = visual::DialogManager::closedDialog();
    if (dialog != nullptr && dialog->callbackFunc != "") {
      int choice = dialog->getChoice();
      GameState::lua()[dialog->callbackFunc.c_str()](choice);
    }
    visual::DialogManager::clearClosedDialog();
  }

  sf::Vector2f moveDelta;
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
    moveDelta.x -= GameState::heroMoveSpeed();
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
    moveDelta.x += GameState::heroMoveSpeed();
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
    if (GameState::map()->isLadder(GameState::hero()->getDimensions())) {
      moveDelta.y += GameState::heroMoveSpeed();
    }
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
    if (GameState::map()->isLadder(GameState::hero()->getDimensions()) &&
        !GameState::hero()->jumping()) {
      moveDelta.y -= GameState::heroMoveSpeed();
    }
  }
  fixMovement(GameState::hero(), moveDelta);

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::W) ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
    GameState::hero()->startJump(1);
  }
  fixMovement(GameState::hero(), moveDelta);

  // Change character direction
  if (moveDelta.x > 0) {
    GameState::hero()->setDirection(SPRITE_RIGHT);
  }
  if (moveDelta.x < 0) {
    GameState::hero()->setDirection(SPRITE_LEFT);
  }
  if (moveDelta.y > 0) {
    GameState::hero()->setDirection(SPRITE_DOWN);
  }
  if (moveDelta.y < 0) {
    GameState::hero()->setDirection(SPRITE_UP);
  }

  auto dim = updateGravity(GameState::hero());

  if (GameState::positionWalkable(GameState::hero(), dim)) {
    GameState::hero()->setDimensions(dim);

    GameState::runTileEvent();
  }

  for (const auto &sprite : GameState::sprites()) {
    auto dim = updateGravity(sprite.second);
    if (GameState::positionWalkable(sprite.second, dim)) {
      // TODO(jsvana): figure out whether or not the sprite
      // collided with another sprite
      sprite.second->setDimensions(dim);
    }
  }

  return true;
}

void MainScreen::render(sf::RenderTarget &window) {
  GameState::map()->render(window, GameState::camera());

  GameState::hero()->render(window, GameState::camera());
  auto mapPos = GameState::map()->getPosition();
  auto cameraPad = cameraPadding();
  auto windowSize = window.getSize();
  auto dim = GameState::hero()->getDimensions();
  int xDiff;
  xDiff = (dim.left + dim.width - GameState::camera().x + cameraPad.x) -
          (mapPos.x + windowSize.x);
  if (xDiff > 0) {
    GameState::camera().x -= xDiff;
  }
  xDiff = (dim.left - GameState::camera().x - cameraPad.x) + mapPos.x;
  if (xDiff < 0) {
    GameState::camera().x += xDiff;
  }
  int yDiff;
  yDiff = (dim.top + dim.height - GameState::camera().y + cameraPad.y) -
          (mapPos.y + windowSize.y);
  if (yDiff > 0) {
    GameState::camera().y -= yDiff;
  }
  yDiff = (dim.top - GameState::camera().y - cameraPad.y) + mapPos.y;
  if (yDiff < 0) {
    GameState::camera().y += yDiff;
  }

  for (const auto &sprite : GameState::sprites()) {
    sprite.second->render(window, GameState::camera());
  }

  heroHealth_.render(window);

  visual::DialogManager::render(window);
}
