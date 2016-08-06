#pragma once

#include "../visual/text.h"
#include "screen.h"

#include <SFML/Graphics.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

/**
 * Parent class to display interactive menus
 */
class MenuScreen : public Screen {
 private:
  sf::Color SELECTED_COLOR = sf::Color(220, 220, 220);
  sf::Color NORMAL_COLOR = sf::Color(144, 144, 144);

  sf::Font font;

  int selectedItem;

 protected:
  std::string title;

  std::vector<std::string> items;
  std::vector<std::function<void(void)>> itemFunctions;

  std::vector<std::shared_ptr<sf::Text>> textItems;
  sf::Text titleText;

  /**
   * Loads menu contents and creates all texts
   */
  void load();

 public:
  MenuScreen();

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
  void render(sf::RenderTarget &target);
};
