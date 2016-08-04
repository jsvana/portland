#pragma once

#include "../dialog.h"
#include "../entities/sprite.h"
#include "../entities/npc.h"
#include "../map.h"
#include "../state.h"
#include "../text.h"
#include "../util.h"
#include "screen.h"

#include "../lib/selene/include/selene.h"

#include <SDL.h>

#include <functional>
#include <stack>
#include <string>
#include <unordered_map>

/**
 * Class to handle the main game logic
 */
class MainScreen : public Screen {
 private:
  // Once character hits these points screen will move instead of character
  const Point CAMERA_PADDING_TILES{6, 2};

  unsigned long ticks_;

  const float MAX_JUMP_HOLD_DURATION = 20;
  const float MIN_JUMP_HOLD_DURATION = 10;
  const float JUMP_HOLD_INCREASE = 5;
  bool jumpHeld_ = false;
  float jumpHoldDuration_;

  // Camera to handle player movement
  BoundedPoint camera_;

  /**
   * Gets the dimensions in pixels of the camera padding
   *
   * @return Pixel dimensions of camera padding
   */
  Point cameraPadding() {
    Point p(CAMERA_PADDING_TILES.x * GameState::map()->tileWidth(), CAMERA_PADDING_TILES.y * GameState::map()->tileHeight());
    return p;
  }

  /**
   * Take a rect and move it according to moveDelta. If one
   * component of the move fails, only use the working one.
   *
   * If (x, y) fails, try (x, 0) and (0, y). If nothing works
   * move becomes a no-op.
   *
   * @param dim Rectangle to move
   * @param moveDelta Distance to try to move the rectangle
   * @return Whether the rectangle was moved
   */
  bool fixMovement(Rect &dim, Point moveDelta);

 public:
  MainScreen(int width, int height);

  /**
   * @see Screen::handleEvent
   */
  void handleEvent(const SDL_Event &event);

  /**
   * @see Screen::update
   */
  bool update(unsigned long frames);

  /**
   * @see Screen::render
   */
  void render(float interpolation);
};
