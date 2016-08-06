#include "asset_manager.h"

#include "util.h"

#include <iostream>
#include <map>

extern SDL_Renderer *renderer;

namespace AssetManager {
  static std::map<std::string, SDL_Texture *> textures_;
  static std::map<std::string, TTF_Font *> fonts_;

  TTF_Font *getFont(const std::string &path, int size) {
    auto res = fonts_.find(path);
    if (res != fonts_.end()) {
      return res->second;
    }
    TTF_Font *font = TTF_OpenFont(path.c_str(), size);
    if (font == nullptr) {
      err()->error("Error loading \"{}\": {}", path, TTF_GetError());
      return nullptr;
    }
    fonts_[path] = font;
    return font;
  }

  SDL_Texture *getTexture(const std::string &path) {
    auto res = textures_.find(path);
    if (res != textures_.end()) {
      return res->second;
    }
    SDL_Texture *texture = IMG_LoadTexture(renderer, path.c_str());
    if (texture == nullptr) {
      err()->error("Error loading \"{}\": {}", path, IMG_GetError());
      return nullptr;
    }
    textures_[path] = texture;
    return texture;
  }

  void destroy() {
    for (auto texture : textures_) {
      SDL_DestroyTexture(texture.second);
    }
    for (auto font : fonts_) {
      TTF_CloseFont(font.second);
    }

    textures_.clear();
    fonts_.clear();
  }
}
