#pragma once

#include "../util.h"

#include <SFML/Graphics.hpp>

class ProgressBar {
 private:
  const int PADDING = 2;

  float max_ = 0;
  float current_ = 0;

  sf::FloatRect dimensions_;
  sf::FloatRect fillDimensions_;

  /**
   * Gets the current progress bar fill percentage,
   * between 0 and 1 inclusive
   *
   * @return Current fill percentage
   */
  float fillPercentage() {
    float pct = current_ / max_;
    util::clamp<float>(pct, 0, 1);
    return pct;
  }

  /**
   * Updates the inner rectangle according to the
   * current outer dimensions and fill percentage
   */
  void updateFillDimensions() {
    fillDimensions_ = dimensions_;
    fillDimensions_.width *= fillPercentage();
  }

 public:
  ProgressBar() {}
  ProgressBar(int max) : max_(max), current_(max) {}

  void setMax(float max) {
    max_ = max;
    current_ = max;
  }

  /**
   * Sets a new progress bar value
   *
   * @param value New progress bar value
   */
  void setValue(float value) {
    util::clamp<float>(value, 0, max_);
    updateFillDimensions();
  }

  /**
   * Shrinks the progress bar value
   *
   * @param value Magnitude of shrink
   */
  void shrink(float value) {
    current_ -= value;
    util::clamp<float>(value, 0, max_);
    updateFillDimensions();
  }

  void fillBar() { setValue(max_); }

  /**
   * Gets the progress bar dimensions
   *
   * @return dim Dimensions of progress bar
   */
  sf::FloatRect getDimensions() { return dimensions_; }

  /**
   * Sets the progress bar dimensions
   *
   * @param dim New dimensions of progress bar
   */
  void setDimensions(sf::FloatRect dim) {
    dimensions_ = dim;
    updateFillDimensions();
  }

  /**
   * Sets the progress bar dimensions
   *
   * @param x New x coordinate of progress bar
   * @param y New y coordinate of progress bar
   * @param w New width of progress bar
   * @param h New height of progress bar
   */
  void setDimensions(int x, int y, int w, int h) {
    sf::FloatRect dim(x + PADDING, y + PADDING, w - 2 * PADDING,
                      h - 2 * PADDING);
    setDimensions(dim);
  }

  /**
   * Gets the position of the progress bar
   *
   * @return Progress bar position
   */
  sf::Vector2f getPosition() {
    sf::Vector2f position(dimensions_.left - PADDING,
                          dimensions_.top - PADDING);
    return position;
  }

  /**
   * Sets position of the progress bar from the top left corner
   *
   * @param p New position of progress bar
   */
  void setPosition(sf::Vector2f p) { setPosition(p.x, p.y); }

  /**
   * Sets position of the progress bar from the top left corner
   *
   * @param x New x coordinate of progress bar
   * @param y New y coordinate of progress bar
   */
  void setPosition(int x, int y) {
    dimensions_.left = x + PADDING;
    dimensions_.top = y + PADDING;
  }

  /**
   * Renders the progress bar
   *
   * @param window Window to render to
   */
  void render(sf::RenderTarget& window);
};
