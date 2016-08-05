#include "main_screen.h"

#include "../screen_manager.h"
#include "../state.h"
#include "../util.h"
#include "pause_menu.h"

#include <SDL.h>

#include <functional>
#include <iostream>
#include <set>

extern SDL_Renderer *renderer;

MainScreen::MainScreen(int width, int height) : Screen(width, height) {
  ticks_ = 0;

  // Load the game script
  GameState::lua().Load("assets/scripts/game.lua");
  GameState::lua()["init"]();
}

void MainScreen::handleEvent(const SDL_Event &) {}

bool MainScreen::fixMovement(Sprite *sprite, Point moveDelta) {
  Rect dim = sprite->getDimensions();
  Rect oldDim = dim;
  dim.move(moveDelta.x, moveDelta.y);
  if (!GameState::positionWalkable(sprite, dim)) {
    dim.move(-moveDelta.x, 0);
  }
  if (!GameState::positionWalkable(sprite, dim)) {
    dim.move(moveDelta.x, -moveDelta.y);
  }
  sprite->setDimensions(dim);
  return dim != oldDim;
}

Rect MainScreen::updateGravity(Sprite *sprite, Point &moveDelta) {
  Rect dim = sprite->getDimensions();
  if (GameState::map()->isLadder(dim)) {
    sprite->zeroVelocity(/*stopJump = */ true);
  } else {
    // Falling
    sprite->updateVelocity();
    Point jumpDelta(0, sprite->velocity());
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
      moveDelta.move(jumpDelta);
    }
  }
  return dim;
}

bool MainScreen::update(unsigned long ticks) {
  // Used for getting ticks in Lua
  ticks_ = ticks;

  GameState::map()->update(ticks);
  GameState::hero()->update(ticks);
  for (auto &sprite : GameState::sprites()) {
    sprite.second->update(ticks);
  }
  GameState::lua()["update"]();

  auto state = SDL_GetKeyboardState(nullptr);

  if (DialogManager::update(ticks)) {
    return true;
  } else {
    Dialog *dialog = DialogManager::closedDialog();
    if (dialog != nullptr && dialog->callbackFunc != "") {
      int choice = dialog->getChoice();
      GameState::lua()[dialog->callbackFunc.c_str()](choice);
    }
    DialogManager::clearClosedDialog();
  }

  // Movement input
  Point moveDelta;
  if (state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT]) {
    moveDelta.x -= GameState::heroMoveSpeed();
  }
  if (state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT]) {
    moveDelta.x += GameState::heroMoveSpeed();
  }

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

  if (state[SDL_SCANCODE_P] || state[SDL_SCANCODE_ESCAPE]) {
    ScreenManager::push(new PauseMenuScreen(width_, height_));
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
    Point interactionPoint =
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

  Rect dim = updateGravity(GameState::hero(), moveDelta);

  if (GameState::positionWalkable(GameState::hero(), dim)) {
    GameState::hero()->setDimensions(dim);

    GameState::runTileEvent();

    // Scrolling
    Point mapPos = GameState::map()->getPosition();
    Point cameraPad = cameraPadding();
    if ((moveDelta.x > 0 &&
         dim.x + dim.w - GameState::camera().x + cameraPad.x >=
             mapPos.x + width_) ||
        (moveDelta.x < 0 &&
         dim.x - GameState::camera().x - cameraPad.x < mapPos.x)) {
      GameState::camera().move(moveDelta.x, 0);
    }
    if ((moveDelta.y > 0 &&
         dim.y + dim.h - GameState::camera().y + cameraPad.y >=
             mapPos.y + height_) ||
        (moveDelta.y < 0 &&
         dim.y - GameState::camera().y - cameraPad.y < mapPos.y)) {
      GameState::camera().move(0, moveDelta.y);
    }
  }

  for (auto &sprite : GameState::sprites()) {
    Rect dim = updateGravity(sprite.second);
    if (GameState::positionWalkable(sprite.second, dim)) {
      sprite.second->setDimensions(dim);
    }
  }

  return true;
}

void MainScreen::render(float) {
  GameState::map()->render(GameState::camera());
  GameState::hero()->render(GameState::camera());
  for (const auto &sprite : GameState::sprites()) {
    sprite.second->render(GameState::camera());
  }
  DialogManager::render();
}
