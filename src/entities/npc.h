#pragma once

#include "../util.h"
#include "sprite.h"

#include <string>

/**
 * Class representing non-player characters in game
 */
class Npc : public Sprite {
 public:
  Npc(const std::string &path) : Sprite(path) {}

  void render(Point cameraPos) const;
};
