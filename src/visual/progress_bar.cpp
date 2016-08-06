#include "progress_bar.h"

void ProgressBar::render(sf::RenderTarget &window) {
  sf::RectangleShape background;
  background.setSize(
      sf::Vector2f(dimensions_.x - PADDING, dimensions_.y - PADDING));
  background.setPosition(dimensions_.x + dimensions_.w + 2 * PADDING,
                         dimensions_.y + dimensions_.h + 2 * PADDING);
  background.setFillColor(sf::Color::White);
  window.draw(background);

  sf::RectangleShape secondBackground;
  secondBackground.setSize(sf::Vector2f(dimensions_.x, dimensions_.y));
  secondBackground.setPosition(dimensions_.x + dimensions_.w,
                               dimensions_.y + dimensions_.h);
  secondBackground.setFillColor(sf::Color::Black);
  window.draw(secondBackground);

  sf::RectangleShape fill;
  fill.setSize(sf::Vector2f(fillDimensions_.x + fillDimensions_.w,
                            fillDimensions_.y + fillDimensions_.h));
  fill.setPosition(fillDimensions_.x, fillDimensions_.y);
  fill.setFillColor(sf::Color::Red);
  window.draw(fill);
}
