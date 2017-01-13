#include "pause_menu.h"

#include "../engine.h"
#include "../state.h"

#include <functional>
#include <iostream>

PauseMenuScreen::PauseMenuScreen() : MenuScreen() {
  title_ = "PAUSED";
  items_ = {"Resume", "Save", "Load", "Quit"};
  itemFunctions_ = {
      std::bind(&PauseMenuScreen::resume, this),
      std::bind(&PauseMenuScreen::saveGame, this),
      std::bind(&PauseMenuScreen::loadGame, this),
      std::bind(&PauseMenuScreen::quit, this),
  };

  load();
}

bool PauseMenuScreen::resume() {
  Engine::popScreen();
  return true;
}

bool PauseMenuScreen::saveGame() {
  GameState::save("portland.save");
  return true;
}

bool PauseMenuScreen::loadGame() {
  GameState::load("portland.save");
  return true;
}

bool PauseMenuScreen::quit() {
  running_ = false;
  return running_;
}

void PauseMenuScreen::render(sf::RenderTarget& target) {
  MenuScreen::render(target);
}
