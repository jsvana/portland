#pragma once

#include "../screen_manager.h"
#include "../text.h"
#include "screen.h"

#include <SDL.h>

#include <functional>
#include <string>
#include <vector>

/**
 * Parent class to display interactive menus
 */
class MenuScreen : public Screen {
 private:
  const int FRAME_DEBOUNCE_DELAY = 5;

  SDL_Color SELECTED_COLOR;
  SDL_Color NORMAL_COLOR;

  int selectedItem_;
  unsigned int selectFrames_;

  Text *titleTexture_;
  std::vector<Text *> itemTextures_;

 protected:
  std::string title_;
  std::vector<std::string> items_;
  std::vector<std::function<int(void)>> itemFunctions_;

  /**
   * Loads menu contents and creates all texts
   */
  void load();

 public:
  MenuScreen(int width, int height);

  /**
   * @see Screen::handleEvent
   */
  void handleEvent(const SDL_Event &event);

  /**
   * @see Screen::update
   */
  bool update(unsigned long frames);

  /**
   * @see Screen::render
   */
  void render(float interpolation);
};
