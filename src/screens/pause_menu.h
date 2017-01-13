#pragma once

#include "../util.h"
#include "menu.h"
#include "screen.h"

/**
 * Game pause screen, supports resuming and quitting
 */
class PauseMenuScreen : public MenuScreen {
 private:
  // TODO(jsvana): make this configurable
  const std::string SAVE_FILE = "portland.save";

  /**
   * Closes menu. Runs on selection of "Resume"
   *
   * @return Whether screen should still be open
   */
  bool resume();

  /**
   * Saves the game. Runs on selection of "Save"
   *
   * @return Whether screen should still be open
   */
  bool saveGame();

  /**
   * Loads the game. Runs on selection of "Load"
   *
   * @return Whether screen should still be open
   */
  bool loadGame();

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
  void render(sf::RenderTarget& window);
};
