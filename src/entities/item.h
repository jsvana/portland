#pragma once

#include "../util.h"
#include "sprite.h"

#include <json.hpp>

#include <SFML/Graphics.hpp>

#include <string>

namespace entities {

/**
 * Class to load, render, and update items onscreen
 */
class Item : public Sprite {
 private:
  bool held_ = false;

  /**
   * Loads sprite from given path
   *
   * @param path Path to load sprite from
   * @param Whether operation was successful
   */
  bool load(const std::string& path);

 public:
  Item(const std::string& path) : Sprite(path, SpriteType::ITEM) {}

  bool phased() { return true; }

  void drop() {
    held_ = false;
    activate();
  }
  void hold() {
    held_ = true;
    deactivate();
  }
  bool held() { return held_; }

  /**
   * Animates item
   *
   * @param time Amount of time since last update
   */
  void update(sf::Time& time);
};

}  // namespace entities
