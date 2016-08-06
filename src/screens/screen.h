#pragma once

#include <SFML/Graphics.hpp>

#include <string>

/**
 * Class representing displayable, updatable game logic
 */
class Screen {
 protected:
  int width_;
  int height_;

 public:
  Screen(int width, int height) : width_(width), height_(height){};

  /**
   * Gets width of screen
   *
   * @return Width of screen
   */
  int width() { return width_; }

  /**
   * Gets height of screen
   *
   * @return Height of screen
   */
  int height() { return height_; }

  /**
   * SDL event handler, called by main game loop
   *
   * @param event Event to handle
   */
  virtual void handleEvent(sf::Event &event) = 0;

  /**
   * Called regularly to update the screen
   *
   * @param frames Number of frames since start
   * @return Whether the screen is still running
   */
  virtual bool update(sf::Time &time) = 0;

  /**
   * Renders the screen
   *
   * @param interpolation Fraction of time between regular updates
   */
  virtual void render(sf::RenderTarget &target) = 0;
};
