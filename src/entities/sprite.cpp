#include "sprite.h"

#include <glog/logging.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace entities {

Sprite::Sprite(const std::string& path, SpriteType type) : type_(type) {
  load(path);

  direction_ = util::Direction::RIGHT;
  visualDirection_ = util::Direction::RIGHT;
}

bool Sprite::load(const std::string& path) {
  std::ifstream spritefile(path);

  if (!spritefile.is_open()) {
    LOG(ERROR) << "Unable to load spritefile: " << path;
    return false;
  }

  std::stringstream fileData;
  fileData << spritefile.rdbuf();
  spritefile.close();

  auto spriteData = nlohmann::json::parse(fileData.str());

  dimensions_.width = spriteData["width"].get<int>();
  dimensions_.height = spriteData["height"].get<int>();
  tile_ = spriteData["tile"].get<int>();
  multiFile_ = spriteData["multi_file"].get<bool>();
  scale_ = spriteData["scale"].get<float>();
  frameSpacing_ = spriteData["frame_spacing"].get<int>();
  columns_ = spriteData["columns"].get<int>();

  std::vector<std::string> texturePaths;
  if (multiFile_) {
    texturePaths = spriteData["frames"].get<std::vector<std::string>>();
    totalFrames_ = (int)texturePaths.size();
  } else {
    texturePaths.push_back(spriteData["texture"].get<std::string>());
    totalFrames_ = spriteData["total_frames"].get<int>();
  }

  auto basePath = path.substr(0, path.find_last_of("/"));
  for (auto& path : texturePaths) {
    std::string fullPath = basePath + "/" + path;
    sf::Texture texture;
    texture.loadFromFile(fullPath);

    textures_.push_back(texture);
  }

  return true;
}

void Sprite::updateVelocity() {
  velocityY_ += GRAVITY;

  // Necessary to prevent jumping after initiating a fall
  jumping_ = true;
}

void Sprite::setVelocity(float velocity) { velocityY_ = velocity; }

void Sprite::startJump(float magnitudePercent) {
  if (!jumping_ && canJump_) {
    velocityY_ = STARTING_JUMP_VELOCITY * magnitudePercent;
    jumping_ = true;
  }
}

void Sprite::zeroVelocity(bool stopJump) {
  velocityY_ = 0;
  jumping_ = !stopJump;
}

void Sprite::update(sf::Time& time) {
  time_ += time;
  if (time_ >= sf::milliseconds(500)) {
    int limit = textures_.size();
    if (limit == 1 || totalFrames_ == 1) {
      return;
    }
    int frameIncrease;
    if (multiFile_) {
      limit = totalFrames_;
      frameIncrease = 1;
    } else {
      limit = totalFrames_ * frameSpacing_;
      frameIncrease = frameSpacing_;
    }
    frame_ = (frame_ + frameIncrease) % limit;
    time_ = sf::seconds(0);
  }
}

void Sprite::render(sf::RenderTarget& window, sf::Vector2f cameraPos) {
  int tile = tile_;
  if (!multiFile_) {
    tile += frame_;
  }
  sf::IntRect source((tile % columns_) * dimensions_.width,
                     (tile / columns_) * dimensions_.height, dimensions_.width,
                     dimensions_.height);

  sf::Texture tex;
  if (multiFile_) {
    tex = textures_[frame_];
  } else {
    tex = textures_[0];
  }
  sprite_.setTexture(tex);
  if (visualDirection_ == util::Direction::RIGHT) {
    source.left += dimensions_.width;
    source.width = -dimensions_.width;
  }
  sprite_.setTextureRect(source);
  sprite_.setPosition(dimensions_.left - cameraPos.x,
                      dimensions_.top - cameraPos.y);
  window.draw(sprite_);
}

}  // namespace entities
