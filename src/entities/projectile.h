#pragma once

#include "../util.h"
#include "sprite.h"

#include <json.hpp>

#include <SFML/Graphics.hpp>

#include <string>

namespace entities {

/**
 * Class to load, render, and update projectiles onscreen
 */
class Projectile : public Sprite {
 private:
  float speed_ = 1;
  float moved_ = 0;

  float maxDistance_ = 0;

  /**
   * Loads sprite from given path
   *
   * @param path Path to load sprite from
   * @param Whether operation was successful
   */
  bool load(const std::string& path);

 public:
  Projectile(const std::string& path) : Sprite(path, SpriteType::PROJECTILE) {}

  void setSpeed(const float speed) { speed_ = speed; }
  void setMaxDistance(const float maxDistance) { maxDistance_ = maxDistance; }

  bool phased() { return true; }

  /**
   * Animates item
   *
   * @param time Amount of time since last update
   */
  void update(const sf::Time& time);
};

}  // namespace entities
