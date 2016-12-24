#include "npc.h"

#include <iostream>

namespace entities {

void Npc::render(sf::RenderTarget& window, sf::Vector2f cameraPos) {
  Sprite::render(window, cameraPos);
}

}  // namespace entities
