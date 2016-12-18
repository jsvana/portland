#include "npc.h"

#include <iostream>

void Npc::render(sf::RenderTarget& window, sf::Vector2f cameraPos) {
  Sprite::render(window, cameraPos);
}
