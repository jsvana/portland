#pragma once

#include "../util.h"
#include "../visual/text.h"
#include "screen.h"

#include <SFML/Graphics.hpp>

#include <string>

/**
 * Opening game screen. Displays a message and exits.
 */
class OpeningScreen : public Screen {
 private:
  sf::Font font;

  sf::Text titleText;
  sf::Text enterText;

 public:
  OpeningScreen();

  /**
   * @see Screen::handleEvent
   */
  void handleEvent(sf::Event &event);

  /**
   * @see Screen::update
   */
  bool update(sf::Time &elapsed);

  /**
   * @see Screen::render
   */
  void render(sf::RenderTarget &target);
};
