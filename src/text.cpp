#include "text.h"

#include "asset_manager.h"

#include <iostream>

extern SDL_Renderer *renderer;

Text::Text(const std::string &text, int size) : text_(text), size_(size) {
  // Load font texture
  font_ = AssetManager::getFont("assets/fonts/arcade.ttf", size_);
  load();
  visible_ = true;
}

void Text::load() {
  SDL_Surface *surface = TTF_RenderText_Solid(font_, text_.c_str(), color_);
  if (surface == nullptr) {
    std::cerr << "Failed to render text" << std::endl;
    std::cerr << TTF_GetError() << std::endl;
    return;
  }

  texture_ = SDL_CreateTextureFromSurface(renderer, surface);
  if (texture_ == nullptr) {
    std::cerr << "Failed to create text texture from surface" << std::endl;
    std::cerr << SDL_GetError() << std::endl;
    return;
  }
  SDL_FreeSurface(surface);

  SDL_QueryTexture(texture_, NULL, NULL, &position_.w, &position_.h);
}

void Text::setPositionCenter(int x, int y) {
  position_.x = x - position_.w / 2;
  position_.y = y - position_.h / 2;
}

void Text::render() {
  if (!visible_) {
    return;
  }
  SDL_RenderCopy(renderer, texture_, nullptr, &position_);
}
