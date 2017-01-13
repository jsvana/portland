#include "menu.h"

#include "../constants.h"
#include "../util.h"

#include <SFML/Graphics.hpp>

#include <cassert>
#include <iostream>

MenuScreen::MenuScreen() { font_.loadFromFile("assets/fonts/arcade.ttf"); }

void MenuScreen::load() {
  sf::Color titleColor(255, 255, 255);

  titleText_.setString(title_);
  titleText_.setFont(font_);
  titleText_.setCharacterSize(30);
  titleText_.setFillColor(titleColor);
  auto titleSize = titleText_.getGlobalBounds();
  titleText_.setOrigin(titleSize.width / 2, titleSize.height / 2);
  titleText_.setPosition(SCREEN_WIDTH / 2, 10);

  for (auto item : items_) {
    sf::Text textItem;
    textItem.setString(item);
    textItem.setFont(font_);
    textItem.setCharacterSize(20);
    auto itemSize = textItem.getGlobalBounds();
    textItem.setOrigin(itemSize.width / 2, itemSize.height / 2);
    textItem.setFillColor(NORMAL_COLOR);
    textItems_.push_back(textItem);
  }

  textItems_[selectedItem_].setFillColor(SELECTED_COLOR);

  assert(textItems_.size() == items_.size());
}

void MenuScreen::handleEvent(sf::Event& event) {
  if (event.type == sf::Event::KeyPressed) {
    textItems_[selectedItem_].setFillColor(NORMAL_COLOR);
    switch (event.key.code) {
      case sf::Keyboard::Up:
      case sf::Keyboard::W:
        selectedItem_ =
            (selectedItem_ > 0) ? selectedItem_ - 1 : items_.size() - 1;
        break;
      case sf::Keyboard::Down:
      case sf::Keyboard::S:
        selectedItem_ = (selectedItem_ + 1) % items_.size();
        break;
      case sf::Keyboard::Return:
        itemFunctions_[selectedItem_]();
        break;
      default:
        break;
    }
    textItems_[selectedItem_].setFillColor(SELECTED_COLOR);
  }
}

bool MenuScreen::update(sf::Time&) { return running_; }

void MenuScreen::render(sf::RenderTarget& target) {
  const auto targetSize = target.getSize();

  target.draw(titleText_);

  for (std::size_t i = 0; i < textItems_.size(); i++) {
    const auto itemSize = textItems_[i].getLocalBounds();
    textItems_[i].setPosition(
        (float)targetSize.x / 2,
        100 + ((float)itemSize.height + PADDING) * (float)i);
    target.draw(textItems_[i]);
  }
}
