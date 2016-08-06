#pragma once

#include "../state.h"
#include "../util.h"
#include "../visual/dialog.h"
#include "../visual/progress_bar.h"
#include "screen.h"

#include <SFML/Graphics.hpp>

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

  sf::Time time_;

  const float MAX_JUMP_HOLD_DURATION = 20;
  const float MIN_JUMP_HOLD_DURATION = 10;
  const float JUMP_HOLD_INCREASE = 5;
  bool jumpHeld_ = false;
  float jumpHoldDuration_;

  ProgressBar heroHealth_;

  // Camera to handle player movement
  BoundedPoint camera_;

  /**
   * Gets the dimensions in pixels of the camera padding
   *
   * @return Pixel dimensions of camera padding
   */
  Point cameraPadding() {
    Point p(CAMERA_PADDING_TILES.x * GameState::map()->tileWidth(),
            CAMERA_PADDING_TILES.y * GameState::map()->tileHeight());
    return p;
  }

  /**
   * Take a sprite and move it according to moveDelta. If one
   * component of the move fails, only use the working one.
   *
   * If (x, y) fails, try (x, 0) and (0, y). If nothing works
   * move becomes a no-op.
   *
   * @param sprite Sprite to move
   * @param moveDelta Distance to try to move the sprite
   * @return Whether the sprite was moved
   */
  bool fixMovement(std::shared_ptr<Sprite> sprite, sf::Vector2f moveDelta);

  /**
   * Takes a sprite and updates its gravity, also updates
   * a previous move if necessary.
   *
   * @param sprite Sprite to update
   * @param moveDelta Previous move amount
   * @return Updated sprite dimensions
   */
  sf::FloatRect updateGravity(std::shared_ptr<Sprite> sprite, sf::Vector2f &moveDelta);

  /**
   * Helper for updateGravity that passes an initial
   * movement of (0, 0).
   *
   * @param sprite Sprite to update
   * @return Updated sprite dimensions
   */
  sf::FloatRect updateGravity(std::shared_ptr<Sprite> sprite) {
    sf::Vector2f moveDelta;
    return updateGravity(sprite, moveDelta);
  }

 public:
  MainScreen();

  /**
   * @see Screen::handleEvent
   */
  void handleEvent(sf::Event &event);

  /**
   * @see Screen::update
   */
  bool update(sf::Time &time);

  /**
   * @see Screen::render
   */
  void render(sf::RenderTarget &window);
};
