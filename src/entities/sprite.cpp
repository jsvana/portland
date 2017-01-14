#include "sprite.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace entities {

Sprite::Sprite(const std::string& path, SpriteType type)
    : path_(path), type_(type) {
  load(path);

  direction_ = util::Direction::RIGHT;
  visualDirection_ = util::Direction::RIGHT;
}

bool Sprite::load(const std::string& path) {
  std::ifstream spritefile(path);

  if (!spritefile.is_open()) {
    logger::error("Unable to load spritefile: " + path);
    return false;
  }

  std::stringstream fileData;
  fileData << spritefile.rdbuf();
  spritefile.close();

  auto spriteData = nlohmann::json::parse(fileData.str());

  dimensions_.width = spriteData["width"].get<float>();
  dimensions_.height = spriteData["height"].get<float>();
  tile_ = spriteData["tile"].get<map::TileId>();
  updateMs_ = sf::milliseconds(spriteData["update_ms"].get<int>());
  multiFile_ = spriteData["multi_file"].get<bool>();
  scale_ = spriteData["scale"].get<float>();
  sprite_.setScale(scale_, scale_);
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

void Sprite::setFlag(const std::string& key, const bool flag) {
  flags_[key] = flag;
}

bool Sprite::getFlag(const std::string& key) {
  const auto iter = flags_.find(key);
  if (iter == flags_.end()) {
    return false;
  }
  return iter->second;
}

void Sprite::setValue(const std::string& key, const int value) {
  values_[key] = value;
}

int Sprite::getValue(const std::string& key) {
  const auto iter = values_.find(key);
  if (iter == values_.end()) {
    return 0;
  }
  return iter->second;
}

const nlohmann::json Sprite::serialize() {
  nlohmann::json out;

  out["id"] = id;
  out["path"] = path_;
  out["tile"] = tile_;
  out["type"] = static_cast<int>(type_);
  out["dimensions"] = serializeFloatRect(dimensions_);
  out["hp"] = hp_;
  out["max_hp"] = maxHp_;
  out["active"] = active_;
  out["held_items"] = heldItems_;
  out["flags"] = flags_;
  out["values"] = values_;

  return out;
}

void Sprite::deserialize(const nlohmann::json& data) {
  id = data["id"].get<Id>();
  tile_ = data["tile"].get<map::TileId>();
  type_ = static_cast<SpriteType>(data["type"].get<int>());
  dimensions_ = deserializeFloatRect(
      data["dimensions"].get<std::unordered_map<std::string, float>>());
  hp_ = data["hp"].get<float>();
  maxHp_ = data["max_hp"].get<float>();
  active_ = data["active"].get<bool>();
  heldItems_.clear();
  for (const auto itemId : data["held_items"].get<std::vector<Id>>()) {
    heldItems_.insert(itemId);
  }
  flags_.clear();
  for (const auto& p :
       data["flags"].get<std::unordered_map<std::string, bool>>()) {
    flags_[p.first] = p.second;
  }
  values_.clear();
  for (const auto& p :
       data["values"].get<std::unordered_map<std::string, int>>()) {
    values_[p.first] = p.second;
  }
}

void Sprite::update(const sf::Time& time) {
  if (!active()) {
    return;
  }
  time_ += time;
  if (time_ >= updateMs_) {
    int limit = textures_.size();
    if (limit == 1 && totalFrames_ == 1) {
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
  if (!active()) {
    return;
  }
  map::TileId tile = tile_;
  if (!multiFile_) {
    tile += frame_;
  }
  sf::IntRect source((tile % columns_) * (int)dimensions_.width,
                     (tile / columns_) * (int)dimensions_.height,
                     (int)dimensions_.width, (int)dimensions_.height);

  sf::Texture tex;
  if (multiFile_) {
    tex = textures_[frame_];
  } else {
    tex = textures_[0];
  }
  sprite_.setTexture(tex);
  if (visualDirection_ == util::Direction::RIGHT) {
    source.left += (int)dimensions_.width;
    source.width = (int)-dimensions_.width;
  }
  sprite_.setTextureRect(source);
  sprite_.setPosition(dimensions_.left - cameraPos.x,
                      dimensions_.top - cameraPos.y);
  window.draw(sprite_);
}

}  // namespace entities
