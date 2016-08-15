#pragma once

#include "../util.h"
#include "sprite.h"

#include <json.hpp>

#include <SFML/Graphics.hpp>

#include <string>

/**
 * Class to load, render, and update items onscreen
 */
class Item : public Sprite {
 private:
  /**
   * Loads sprite from given path
   *
   * @param path Path to load sprite from
   * @param Whether operation was successful
   */
  bool load(const std::string &path);

 public:
  unsigned int id;

  Item(const std::string &path) : Sprite(path, SPRITE_ITEM) {}

  /**
   * Animates item
   *
   * @param time Amount of time since last update
   */
  void update(sf::Time &time);
};
