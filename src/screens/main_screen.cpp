#include "main_screen.h"

#include "../engine.h"
#include "../state.h"
#include "../util.h"
#include "../visual/console.h"
#include "pause_menu.h"

#include <glog/logging.h>

#include <functional>
#include <iostream>
#include <set>

MainScreen::MainScreen() {
  heroHealth_.setMax(100);
  heroHealth_.setValue(70);
  heroHealth_.setDimensions(16, 16, 64, 16);

  // Load the game script
  GameState::chai().eval_file("assets/scripts/game.chai");
  auto init = GameState::chai().eval<std::function<void()>>("init");
  init();
  GameState::markInitialized();

  visual::Console::initialize();
}

bool MainScreen::fixMovement(const std::unique_ptr<entities::Sprite>& sprite,
                             sf::Vector2f moveDelta) {
  auto dim = sprite->getDimensions();
  const auto oldDim = dim;
  dim.left += moveDelta.x;
  if (!GameState::positionWalkable(sprite, dim)) {
    dim.left = oldDim.left;
  }
  // dim.top += moveDelta.y;
  if (!GameState::positionWalkable(sprite, dim)) {
    dim.top = oldDim.top;
  }
  // This could probably still cause weirdness
  sprite->setDimensions(dim);
  return dim != oldDim;
}

sf::FloatRect MainScreen::updateGravity(
    const std::unique_ptr<entities::Sprite>& sprite) {
  auto dim = sprite->getDimensions();
  if (GameState::map()->isLadder(dim)) {
    sprite->zeroVelocity(/*stopJump = */ true);
  } else {
    sprite->updateVelocity();
    float below = GameState::densePositionBelow(sprite);
    float above = GameState::densePositionAbove(sprite);

    sf::Vector2f jumpDelta(0, sprite->velocity());
    dim = sprite->getDimensions();
    float top = dim.top;
    top += jumpDelta.y;
    bool clamped = util::clamp<float>(top, above, below);
    if (clamped) {
      sprite->zeroVelocity(true);
    }
    if ((int)top == (int)below) {
      sprite->allowJump();
    } else {
      sprite->forbidJump();
    }
    dim.top = top;
  }
  return dim;
}

void MainScreen::handleEvent(sf::Event& event) {
  if (visual::Console::visible()) {
    visual::Console::handleEvent(event);
    return;
  }

  visual::DialogManager::handleEvent(event);

  if (event.type == sf::Event::KeyPressed) {
    if (event.key.code == sf::Keyboard::P) {
      Engine::pushScreen(new PauseMenuScreen());
    } else if (event.key.code == sf::Keyboard::T) {
      heroHealth_.shrink(5);
    } else if (event.key.code == sf::Keyboard::C) {
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
        visual::Console::show();
      }
    }
  }
}

bool MainScreen::update(sf::Time& time) {
  // Used for getting ticks in Lua
  time_ = time;

  const auto chaiUpdate =
      GameState::chai().eval<std::function<void()>>("update");
  chaiUpdate();
  GameState::map()->update(time_);
  GameState::hero()->update(time_);
  for (const auto& sprite : GameState::sprites()) {
    sprite->update(time_);
  }

  if (visual::Console::visible()) {
    visual::Console::update(time);
    return true;
  }

  if (visual::DialogManager::update(time_)) {
    return true;
  } else {
    const auto& dialog = visual::DialogManager::closedDialog();
    if (dialog != nullptr && dialog->callbackFunc) {
      dialog->callbackFunc(dialog->getChoice());
    }
    visual::DialogManager::clearClosedDialog();
  }

  auto startDim = GameState::hero()->getDimensions();

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

  // Change character direction
  if (moveDelta.x > 0) {
    GameState::hero()->setDirection(entities::SpriteDirection::RIGHT);
  }
  if (moveDelta.x < 0) {
    GameState::hero()->setDirection(entities::SpriteDirection::LEFT);
  }
  if (moveDelta.y > 0) {
    GameState::hero()->setDirection(entities::SpriteDirection::DOWN);
  }
  if (moveDelta.y < 0) {
    GameState::hero()->setDirection(entities::SpriteDirection::UP);
  }

  auto dim = updateGravity(GameState::hero());

  if (GameState::positionWalkable(GameState::hero(), dim)) {
    GameState::hero()->setDimensions(dim);

    GameState::runTileEvent();
  }

  auto currentDim = GameState::hero()->getDimensions();
  moveDelta.x = currentDim.left - startDim.left;
  moveDelta.y = currentDim.top - startDim.top;

  // Scrolling
  auto mapPos = GameState::map()->getPosition();
  auto cameraPad = cameraPadding();
  if ((moveDelta.x > 0 &&
       dim.left + dim.width - GameState::camera().x + cameraPad.x >=
           mapPos.x + SCREEN_WIDTH) ||
      (moveDelta.x < 0 &&
       dim.left - GameState::camera().x - cameraPad.x < mapPos.x)) {
    float x = GameState::camera().x + moveDelta.x;
    util::clamp<float>(x, 0, GameState::map()->pixelWidth() - SCREEN_WIDTH);
    GameState::camera().x = x;
  }
  if ((moveDelta.y > 0 &&
       dim.top + dim.height - GameState::camera().y + cameraPad.y >=
           mapPos.y + SCREEN_HEIGHT) ||
      (moveDelta.y < 0 &&
       dim.top - GameState::camera().y - cameraPad.y < mapPos.y)) {
    float y = GameState::camera().y + moveDelta.y;
    util::clamp<float>(y, 0, GameState::map()->pixelHeight() - SCREEN_HEIGHT);
    GameState::camera().y = y;
  }

  for (const auto& sprite : GameState::sprites()) {
    auto dim = updateGravity(sprite);
    if (GameState::positionWalkable(sprite, dim)) {
      // TODO(jsvana): figure out whether or not the sprite
      // collided with another sprite
      sprite->setDimensions(dim);
    }
  }

  return true;
}

void MainScreen::render(sf::RenderTarget& window) {
  GameState::map()->render(window, GameState::camera());
  GameState::hero()->render(window, GameState::camera());
  for (const auto& sprite : GameState::sprites()) {
    sprite->render(window, GameState::camera());
  }
  heroHealth_.render(window);

  visual::DialogManager::render(window);

  if (visual::Console::visible()) {
    visual::Console::render(window);
  }
}
