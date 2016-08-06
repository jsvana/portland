#pragma once

#include "../util.h"
#include "sprite.h"

#include <SFML/Graphics.hpp>

#include <string>

/**
 * Class representing non-player characters in game
 */
class Npc : public Sprite {
 public:
  Npc(const std::string &path) : Sprite(path) {}

  void render(sf::RenderTarget &window, sf::Vector2f cameraPos);
};
