#include "pause_menu.h"

#include "../engine.h"

#include <SDL.h>

#include <functional>
#include <iostream>

PauseMenuScreen::PauseMenuScreen() : MenuScreen() {
  title = "PAUSED";
  items = {
      "Resume", "Quit",
  };
  itemFunctions = {
      std::bind(&PauseMenuScreen::resume, this),
      std::bind(&PauseMenuScreen::quit, this),
  };

  load();
}

bool PauseMenuScreen::resume() {
  Engine::popScreen();
  return true;
}

bool PauseMenuScreen::quit() { return false; }

void PauseMenuScreen::render(sf::RenderTarget &target) {
  MenuScreen::render(target);
}
