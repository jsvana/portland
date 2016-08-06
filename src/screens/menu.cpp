#include "menu.h"

#include "../util.h"

#include <SDL.h>

#include <iostream>

extern SDL_Renderer *renderer;

MenuScreen::MenuScreen(int width, int height) : Screen(width, height) {
  selectedItem_ = 0;
  selectFrames_ = 0;

  SELECTED_COLOR.r = 220;
  SELECTED_COLOR.g = 220;
  SELECTED_COLOR.b = 220;

  NORMAL_COLOR.r = 144;
  NORMAL_COLOR.g = 144;
  NORMAL_COLOR.b = 144;
}

void MenuScreen::load() {
  int heightSection = height_ / (items_.size() + 1);

  SDL_Color titleColor;
  titleColor.r = 255;
  titleColor.g = 255;
  titleColor.b = 255;

  titleTexture_ = new visual::Text(title_, 30);
  titleTexture_->setPositionCenter(width_ / 2, 20);
  titleTexture_->setColor(titleColor);

  for (unsigned int i = 0; i < items_.size(); i++) {
    auto itemTexture = new visual::Text(items_[i], 10);
    if (itemTexture == nullptr) {
      err()->error("Unable to load font for \"{}\"", items_[i]);
      return;
    }
    itemTexture->setPositionCenter(width_ / 2,
                                   heightSection + heightSection * i);
    itemTexture->setColor(NORMAL_COLOR);
    itemTextures_.push_back(itemTexture);
  }
}

void MenuScreen::handleEvent(const SDL_Event &) {}

bool MenuScreen::update(unsigned long) {
  auto state = SDL_GetKeyboardState(nullptr);
  if (selectFrames_ > 0) {
    selectFrames_ -= 1;
  }

  if (state[SDL_SCANCODE_RETURN] && selectFrames_ == 0) {
    bool res = itemFunctions_[selectedItem_]();
    if (!res) {
      return false;
    }
  }

  itemTextures_[selectedItem_]->setColor(NORMAL_COLOR);
  if ((state[SDL_SCANCODE_W] || state[SDL_SCANCODE_UP]) && selectFrames_ == 0) {
    selectedItem_ -= 1;
    if (selectedItem_ < 0) {
      selectedItem_ = items_.size() - 1;
    }
    selectFrames_ = FRAME_DEBOUNCE_DELAY;
  }

  if ((state[SDL_SCANCODE_S] || state[SDL_SCANCODE_DOWN]) &&
      selectFrames_ == 0) {
    selectedItem_ += 1;
    if ((unsigned int)selectedItem_ >= items_.size()) {
      selectedItem_ = 0;
    }
    selectFrames_ = FRAME_DEBOUNCE_DELAY;
  }
  itemTextures_[selectedItem_]->setColor(SELECTED_COLOR);

  return true;
}

void MenuScreen::render(float) {
  titleTexture_->render();

  for (auto &itemTex : itemTextures_) {
    itemTex->render();
  }
}
