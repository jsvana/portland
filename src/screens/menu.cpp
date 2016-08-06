#include "menu.h"

#include "../util.h"

#include <SFML/Graphics.hpp>

#include <iostream>

MenuScreen::MenuScreen() { font.loadFromFile("assets/fonts/arcade.ttf"); }

void MenuScreen::load() {
  sf::Color titleColor(255, 255, 255);

  titleText.setString(title);
  titleText.setFont(font);
  titleText.setCharacterSize(30);
  titleText.setColor(titleColor);
  auto titleSize = titleText.getLocalBounds();
  titleText.setOrigin(titleSize.width / 2, titleSize.height / 2);

  for (auto item : items) {
    std::shared_ptr<sf::Text> textItem;
    textItem->setString(item);
    textItem->setFont(font);
    textItem->setCharacterSize(20);
    auto itemSize = textItem->getLocalBounds();
    textItem->setOrigin(itemSize.width / 2, itemSize.height / 2);
    textItem->setColor(NORMAL_COLOR);
    textItems.push_back(textItem);
  }
}

void MenuScreen::handleEvent(sf::Event &event) {
  if (event.type == sf::Event::KeyPressed) {
    textItems[selectedItem]->setColor(NORMAL_COLOR);
    switch (event.key.code) {
      case sf::Keyboard::Up:
      case sf::Keyboard::W:
        selectedItem = (selectedItem + 1) % items.size();
        break;
      case sf::Keyboard::Down:
      case sf::Keyboard::S:
        selectedItem = (selectedItem > 0) ? selectedItem - 1 : items.size() - 1;
        break;
      case sf::Keyboard::Return:
        itemFunctions[selectedItem]();
        break;
      default:
        break;
    }
    textItems[selectedItem]->setColor(SELECTED_COLOR);
  }
}

bool MenuScreen::update(sf::Time &) { return true; }

void MenuScreen::render(sf::RenderTarget &target) {
  auto targetSize = target.getSize();

  target.draw(titleText);

  for (unsigned int i = 0; i < textItems.size(); i++) {
    auto itemSize = textItems[i]->getLocalBounds();
    textItems[i]->setPosition(targetSize.x / 2, 100 + itemSize.height * i);
    target.draw(*textItems[i]);
  }
}
