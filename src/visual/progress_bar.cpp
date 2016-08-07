#include "progress_bar.h"

void ProgressBar::render(sf::RenderTarget &window) {
  sf::RectangleShape background;
  background.setPosition(dimensions_.left - PADDING, dimensions_.top - PADDING);
  background.setSize(sf::Vector2f(dimensions_.width + 2 * PADDING,
                                  dimensions_.height + 2 * PADDING));
  background.setFillColor(sf::Color::White);
  window.draw(background);

  sf::RectangleShape secondBackground;
  secondBackground.setPosition(dimensions_.left, dimensions_.top);
  secondBackground.setSize(sf::Vector2f(dimensions_.width, dimensions_.height));
  secondBackground.setFillColor(sf::Color::Black);
  window.draw(secondBackground);

  sf::RectangleShape fill;
  fill.setSize(sf::Vector2f(fillDimensions_.width, fillDimensions_.height));
  fill.setPosition(fillDimensions_.left, fillDimensions_.top);
  fill.setFillColor(sf::Color::Red);
  window.draw(fill);
}
