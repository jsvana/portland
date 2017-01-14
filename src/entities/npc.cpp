#include "npc.h"

#include <iostream>

namespace entities {

void Npc::update(const sf::Time&) {
  if (hp_ <= 0) {
    markNeedsCleanup();
  }
}

void Npc::render(sf::RenderTarget& window, sf::Vector2f cameraPos) {
  Sprite::render(window, cameraPos);
}

}  // namespace entities
