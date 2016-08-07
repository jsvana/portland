#include "opening.h"

#include "../engine.h"
#include "main_screen.h"

#include <SDL.h>

#include <iostream>

OpeningScreen::OpeningScreen() {
  font.loadFromFile("assets/fonts/arcade.ttf");

  titleText.setFont(font);
  titleText.setString("Welcome to Portland!");
  auto titleSize = titleText.getLocalBounds();
  titleText.setCharacterSize(30);
  titleText.setOrigin(titleSize.width / 2, titleSize.height / 2);

  enterText.setFont(font);
  enterText.setString("Press Enter to continue");
  enterText.setCharacterSize(15);
  auto enterSize = enterText.getLocalBounds();
  enterText.setOrigin(enterSize.width / 2, enterSize.height / 2);
}

void OpeningScreen::handleEvent(sf::Event &event) {
  if (event.type == sf::Event::KeyPressed) {
    Engine::replaceScreen(new MainScreen());
  }
}

bool OpeningScreen::update(sf::Time &) { return true; }

void OpeningScreen::render(sf::RenderTarget &target) {
  auto windowSize = target.getSize();

  titleText.setPosition(windowSize.x / 2, windowSize.y / 4);
  target.draw(titleText);

  enterText.setPosition(windowSize.x / 2, windowSize.y / 5 * 3);
  target.draw(enterText);
}
