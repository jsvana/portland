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
  const sf::Vector2f CAMERA_PADDING_TILES{6, 2};

  sf::Time time_;

  const sf::Time MAX_JUMP_HOLD_DURATION = sf::milliseconds(30);
  const sf::Time MIN_JUMP_HOLD_DURATION = sf::milliseconds(0);
  sf::Time jumpHoldDuration_;

  ProgressBar heroHealth_;

  // Camera to handle player movement
  sf::Vector2f camera_;

  /**
   * Gets the dimensions in pixels of the camera padding
   *
   * @return Pixel dimensions of camera padding
   */
  sf::Vector2f cameraPadding() {
    sf::Vector2f p(CAMERA_PADDING_TILES.x * GameState::map()->tileWidth(),
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
  bool fixMovement(const std::unique_ptr<entities::Sprite>& sprite,
                   sf::Vector2f moveDelta);

  /**
   * Takes a sprite and updates its gravity
   *
   * @param sprite Sprite to update
   * @return Updated sprite dimensions
   */
  sf::FloatRect updateGravity(const std::unique_ptr<entities::Sprite>& sprite);

 public:
  MainScreen();

  /**
   * @see Screen::handleEvent
   */
  void handleEvent(sf::Event& event);

  /**
   * @see Screen::update
   */
  bool update(sf::Time& time);

  /**
   * @see Screen::render
   */
  void render(sf::RenderTarget& window);
};
