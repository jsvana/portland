#pragma once

#include "../util.h"
#include "sprite.h"

#include <SFML/Graphics.hpp>

#include <string>

namespace entities {

/**
 * Class representing non-player characters in game
 */
class Npc : public Sprite {
 public:
  Npc(const std::string& path) : Sprite(path, entities::SpriteType::NPC) {}

  bool phased() { return false; }

  void update(const sf::Time& time);

  void render(sf::RenderTarget& window, sf::Vector2f cameraPos);
};

}  // namespace entities
