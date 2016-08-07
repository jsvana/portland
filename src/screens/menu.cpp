#include "menu.h"

#include "../constants.h"
#include "../util.h"

#include <SFML/Graphics.hpp>

#include <iostream>

MenuScreen::MenuScreen() { font_.loadFromFile("assets/fonts/arcade.ttf"); }

void MenuScreen::load() {
  sf::Color titleColor(255, 255, 255);

  titleText_.setString(title_);
  titleText_.setFont(font_);
  titleText_.setCharacterSize(30);
  titleText_.setColor(titleColor);
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
    textItem.setColor(NORMAL_COLOR);
    textItems_.push_back(textItem);
  }

  textItems_[selectedItem_].setColor(SELECTED_COLOR);
}

void MenuScreen::handleEvent(sf::Event &event) {
  if (event.type == sf::Event::KeyPressed) {
    textItems_[selectedItem_].setColor(NORMAL_COLOR);
    switch (event.key.code) {
      case sf::Keyboard::Up:
      case sf::Keyboard::W:
        selectedItem_ = (selectedItem_ + 1) % items_.size();
        break;
      case sf::Keyboard::Down:
      case sf::Keyboard::S:
        selectedItem_ =
            (selectedItem_ > 0) ? selectedItem_ - 1 : items_.size() - 1;
        break;
      case sf::Keyboard::Return:
        itemFunctions_[selectedItem_]();
        break;
      default:
        break;
    }
    textItems_[selectedItem_].setColor(SELECTED_COLOR);
  }
}

bool MenuScreen::update(sf::Time &) { return running_; }

void MenuScreen::render(sf::RenderTarget &target) {
  auto targetSize = target.getSize();

  target.draw(titleText_);

  for (unsigned int i = 0; i < textItems_.size(); i++) {
    auto itemSize = textItems_[i].getLocalBounds();
    textItems_[i].setPosition(targetSize.x / 2,
                              100 + (itemSize.height + PADDING) * i);
    target.draw(textItems_[i]);
  }
}
