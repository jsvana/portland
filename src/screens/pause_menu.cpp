#include "pause_menu.h"

#include "../engine.h"

#include <SDL.h>

#include <functional>
#include <iostream>

PauseMenuScreen::PauseMenuScreen() : MenuScreen() {
  title_ = "PAUSED";
  items_ = {
      "Resume", "Quit",
  };
  itemFunctions_ = {
      std::bind(&PauseMenuScreen::resume, this),
      std::bind(&PauseMenuScreen::quit, this),
  };

  load();
}

bool PauseMenuScreen::resume() {
  Engine::popScreen();
  return true;
}

bool PauseMenuScreen::quit() {
  running_ = false;
  return running_;
}

void PauseMenuScreen::render(sf::RenderTarget &target) {
  MenuScreen::render(target);
}
