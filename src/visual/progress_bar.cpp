#include "progress_bar.h"

void ProgressBar::render(sf::RenderTarget &window) {
  sf::Shape::Rectangle background(dimensions_.x - PADDING,
                                  dimensions_.y - PADDING,
                                  dimensions_.x + dimensions_.w + 2 * PADDING,
                                  dimensions_.y + dimensions_.h + 2 * PADDING);
  background.setFillColor(sf::Color::White);
  window.draw(background);

  sf::Shape::Rectangle secondBackground(dimensions_.x, dimensions_.y,
                                        dimensions_.x + dimensions_.w,
                                        dimensions_.y + dimensions_.h);
  secondBackground.setFillColor(sf::Color::Black);
  window.draw(secondBackground);

  sf::Shape::Rectangle fill(fillDimensions_.x, fillDimensions_.y,
                            fillDimensions_.x + fillDimensions_.w,
                            fillDimensions_.y + fillDimensions_.h);
  fill.setFillColor(sf::Color::Red);
  window.draw(fill);
}
