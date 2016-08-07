#pragma once

#include "../util.h"
#include "menu.h"
#include "screen.h"

#include <SDL.h>

/**
 * Game pause screen, supports resuming and quitting
 */
class PauseMenuScreen : public MenuScreen {
 private:
  /**
   * Closes menu. Runs on selection of "Resume"
   *
   * @return Whether screen should still be open
   */
  bool resume();

  /**
   * Quits game. Runs on selection of "Quit"
   *
   * @return Whether screen should still be open
   */
  bool quit();

 public:
  PauseMenuScreen();

  /**
   * @see Screen::render
   */
  void render(sf::RenderTarget &window);
};
