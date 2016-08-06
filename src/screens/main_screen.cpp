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

bool MainScreen::fixMovement(std::shared_ptr<Sprite> sprite,
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

sf::FloatRect MainScreen::updateGravity(std::shared_ptr<Sprite> sprite,
                                        sf::Vector2f &moveDelta) {
  auto dim = sprite->getDimensions();
  if (GameState::map()->isLadder(dim)) {
    sprite->zeroVelocity(/*stopJump = */ true);
  } else {
    // Falling
    sprite->updateVelocity();
    sf::Vector2f jumpDelta(0, sprite->velocity());
    bool moved = fixMovement(sprite, jumpDelta);
    dim = sprite->getDimensions();
    if (!moved) {
      bool stopJump = true;
      if (jumpDelta.y < 0) {
        stopJump = false;
      }
      sprite->zeroVelocity(stopJump);

      if (stopJump) {
        dim = GameState::map()->snapRectToTileBelow(dim);
      }
    } else {
      moveDelta += jumpDelta;
    }
  }
  return dim;
}

void MainScreen::handleEvent(sf::Event &event) {
  visual::DialogManager::handleEvent(event);

  if (event.type != sf::Event::KeyPressed) {
    return;
  }

  if (event.key.code == sf::Keyboard::P) {
    Engine::pushScreen(new PauseMenuScreen());
  } else if (event.key.code == sf::Keyboard::T) {
    heroHealth_.shrink(5);
  }
}

bool MainScreen::update(sf::Time &time) {
  // Used for getting ticks in Lua
  time_ = time;

  GameState::map()->update(time_);
  GameState::hero()->update(time_);
  for (auto &sprite : GameState::sprites()) {
    sprite.second->update(time_);
  }
  GameState::lua()["update"]();

  auto state = SDL_GetKeyboardState(nullptr);

  if (visual::DialogManager::update(time_)) {
    return true;
  } else {
    auto dialog = visual::DialogManager::closedDialog();
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
  fixMovement(GameState::hero(), moveDelta);

  bool shouldJump = false;
  if (state[SDL_SCANCODE_W] || state[SDL_SCANCODE_UP] ||
      state[SDL_SCANCODE_SPACE]) {
    if (GameState::map()->isLadder(GameState::hero()->getDimensions())) {
      moveDelta.y -= GameState::heroMoveSpeed();
    } else if (jumpHeld_) {
      jumpHoldDuration_ += JUMP_HOLD_INCREASE;
      if (jumpHoldDuration_ >= MAX_JUMP_HOLD_DURATION) {
        jumpHoldDuration_ = MAX_JUMP_HOLD_DURATION;
        shouldJump = true;
        jumpHeld_ = false;
      }
    } else {
      jumpHeld_ = true;
      jumpHoldDuration_ = MIN_JUMP_HOLD_DURATION;
    }
  }

  if (!state[SDL_SCANCODE_W] && !state[SDL_SCANCODE_UP] &&
      !state[SDL_SCANCODE_SPACE]) {
    if (jumpHeld_) {
      shouldJump = true;
      jumpHeld_ = false;
    }
  }

  if (shouldJump && !GameState::hero()->jumping()) {
    GameState::hero()->startJump(jumpHoldDuration_ / MAX_JUMP_HOLD_DURATION);
  }

  if (state[SDL_SCANCODE_S] || state[SDL_SCANCODE_DOWN]) {
    if (GameState::map()->isLadder(GameState::hero()->getDimensions())) {
      moveDelta.y += GameState::heroMoveSpeed();
    }
  }

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

  // Check for interactions
  if (state[SDL_SCANCODE_SPACE]) {
    sf::Vector2f interactionPoint =
        GameState::map()->pixelToMap(GameState::hero()->getPosition());
    SpriteDirection dir = GameState::hero()->getDirection();
    if (dir == SPRITE_LEFT) {
      interactionPoint.x -= 1;
    } else if (dir == SPRITE_RIGHT) {
      interactionPoint.x += 1;
    } else if (dir == SPRITE_UP) {
      interactionPoint.y -= 1;
    } else if (dir == SPRITE_DOWN) {
      interactionPoint.y += 1;
    }

    // TODO(jsvana): should probably use a quadtree here
    for (auto &sprite : GameState::sprites()) {
      auto spritePos =
          GameState::map()->pixelToMap(sprite.second->getPosition());
      if (spritePos == interactionPoint && sprite.second->callbackFunc != "") {
        GameState::lua()[sprite.second->callbackFunc.c_str()](sprite.first);
        break;
      }
    }
  }

  // Walkability check
  if (moveDelta.x != 0 || moveDelta.y != 0) {
    fixMovement(GameState::hero(), moveDelta);
  }

  auto dim = updateGravity(GameState::hero(), moveDelta);

  if (GameState::positionWalkable(GameState::hero(), dim)) {
    GameState::hero()->setDimensions(dim);

    GameState::runTileEvent();
  }

  for (auto &sprite : GameState::sprites()) {
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
