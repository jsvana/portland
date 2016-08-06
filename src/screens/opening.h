#pragma once

#include "../util.h"
#include "../visual/text.h"
#include "screen.h"

#include <SDL.h>

#include <string>

/**
 * Opening game screen. Displays a message and exits.
 */
class OpeningScreen : public Screen {
 private:
  visual::Text *title_;
  visual::Text *pressEnter_;

 public:
  OpeningScreen(int width, int height);

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
