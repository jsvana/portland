#include "opening.h"

#include "../engine.h"
#include "main_screen.h"

#include <SDL.h>

#include <iostream>

extern SDL_Renderer *renderer;

OpeningScreen::OpeningScreen(int width, int height) : Screen(width, height) {
  SDL_Color white;
  white.r = 255;
  white.g = 255;
  white.b = 255;
  title_ = new visual::Text("Welcome to Vania", 15);
  title_->setPositionCenter(width_ / 2, height_ / 4);
  title_->setColor(white);
  pressEnter_ = new visual::Text("Press Enter to continue", 15);
  pressEnter_->setPositionCenter(width_ / 2, 3 * height_ / 4);
  pressEnter_->setColor(white);
}

void OpeningScreen::handleEvent(const SDL_Event &) {}

bool OpeningScreen::update(unsigned long) {
  auto state = SDL_GetKeyboardState(nullptr);

  if (state[SDL_SCANCODE_RETURN]) {
    Engine::replaceScreen(new MainScreen(width_, height_));
  }

  return true;
}

void OpeningScreen::render(float) {
  Uint8 r, g, b, a;
  SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, a);

  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  rect.w = width_;
  rect.h = height_;
  SDL_RenderFillRect(renderer, &rect);

  title_->render();
  pressEnter_->render();

  SDL_SetRenderDrawColor(renderer, r, g, b, a);
}
