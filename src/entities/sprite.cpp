#include "sprite.h"

#include "../asset_manager.h"

#include <SDL.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

extern SDL_Renderer *renderer;

Sprite::Sprite(const std::string &path) {
  load(path);

  direction_ = SPRITE_RIGHT;
  visualDirection_ = SPRITE_RIGHT;
}

bool Sprite::load(const std::string &path) {
  std::ifstream spritefile(path);

  if (!spritefile.is_open()) {
    std::cout << "Unable to open spritefile \"" << path << "\"" << std::endl;
    return false;
  }

  std::stringstream fileData;
  fileData << spritefile.rdbuf();
  spritefile.close();

  auto spriteData = nlohmann::json::parse(fileData.str());

  dimensions_.w = spriteData["width"].get<int>();
  dimensions_.h = spriteData["height"].get<int>();
  tile_ = spriteData["tile"].get<int>();
  multiFile_ = spriteData["multi_file"].get<bool>();
  scale_ = spriteData["scale"].get<float>();
  frameSpacing_ = spriteData["frame_spacing"].get<int>();

  std::vector<std::string> texturePaths;
  if (multiFile_) {
    texturePaths = spriteData["frames"].get<std::vector<std::string>>();
    totalFrames_ = (int) texturePaths.size();
  } else {
    texturePaths.push_back(spriteData["texture"].get<std::string>());
    totalFrames_ = spriteData["total_frames"].get<int>();
  }

  auto basePath = path.substr(0, path.find_last_of("/"));
  for (auto &path : texturePaths) {
    std::string fullPath = basePath + "/" + path;
    auto texture = AssetManager::getTexture(fullPath);
    if (texture == nullptr) {
      std::cout << "\"" << fullPath << "\" does not exist, skipping render"
                << std::endl;
      return false;
    }

    // The two images should be the same size so I'm being lazy
    SDL_QueryTexture(texture, NULL, NULL, &textureDimensions_.w,
                     &textureDimensions_.h);

    textures_.push_back(texture);
  }

  return true;
}

void Sprite::updateVelocity() {
  velocityY_ += GRAVITY;

  // Necessary to prevent jumping after initiating a fall
  jumping_ = true;
}

void Sprite::startJump(float magnitudePercent) {
  velocityY_ = (int)((float)STARTING_JUMP_VELOCITY * magnitudePercent);
  jumping_ = true;
}

void Sprite::zeroVelocity(bool stopJump) {
  velocityY_ = 0;
  jumping_ = !stopJump;
}

void Sprite::update(unsigned long ticks) {
  if (ticks - lastTicks_ >= FRAME_TICKS_INTERVAL) {
    int limit = textures_.size();
    int frameIncrease;
    if (multiFile_) {
      limit = totalFrames_;
      frameIncrease = 1;
    } else {
      limit = totalFrames_ * frameSpacing_;
      frameIncrease = frameSpacing_;
    }
    frame_ = (frame_ + frameIncrease) % limit;
    lastTicks_ = ticks;
  }
}

void Sprite::render(Point cameraPos) const {
  SDL_Rect source;
  int tile = tile_;
  if (!multiFile_) {
    tile += frame_;
  }
  source.x = (tile % (textureDimensions_.w / dimensions_.w)) * dimensions_.w;
  source.y = (tile / (textureDimensions_.w / dimensions_.w)) * dimensions_.h;

  source.w = dimensions_.w;
  source.h = dimensions_.h;

  SDL_Rect dest;
  dest.x = dimensions_.x - cameraPos.x;
  dest.y = dimensions_.y - cameraPos.y;
  dest.w = (int)((float)dimensions_.w * scale_);
  dest.h = (int)((float)dimensions_.h * scale_);

  SDL_Texture *tex;
  if (multiFile_) {
    tex = textures_[frame_];
  } else {
    tex = textures_[0];
  }
  if (visualDirection_ == SPRITE_LEFT) {
    SDL_RenderCopy(renderer, tex, &source, &dest);
  } else {
    SDL_RenderCopyEx(renderer, tex, &source, &dest, 0, NULL,
                     SDL_FLIP_HORIZONTAL);
  }
}
