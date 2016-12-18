#pragma once

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

  float PADDING = 10;

  sf::Font font_;

  int selectedItem_ = 0;

 protected:
  std::string title_;

  std::vector<std::string> items_;
  std::vector<std::function<void(void)>> itemFunctions_;

  std::vector<sf::Text> textItems_;
  sf::Text titleText_;

  // Can be used by child menus to close the game
  bool running_ = true;

  /**
   * Loads menu contents and creates all texts
   */
  void load();

 public:
  MenuScreen();

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
  void render(sf::RenderTarget& target);
};
