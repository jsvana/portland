#include "pause_menu.h"

#include "../screen_manager.h"

#include <SDL.h>

#include <functional>
#include <iostream>

extern SDL_Renderer *renderer;

PauseMenuScreen::PauseMenuScreen(int width, int height)
    : MenuScreen(width, height) {
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
  ScreenManager::pop();
  return true;
}

bool PauseMenuScreen::quit() { return false; }

void PauseMenuScreen::render(float interpolation) {
  Uint8 r, g, b, a;
  SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, a);

  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  rect.w = width_;
  rect.h = height_;
  SDL_RenderFillRect(renderer, &rect);

  SDL_SetRenderDrawColor(renderer, r, g, b, a);

  MenuScreen::render(interpolation);
}
