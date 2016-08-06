#include "npc.h"

#include <SDL.h>

#include <iostream>

void Npc::render(sf::RenderTarget &window, Point cameraPos) {
  Sprite::render(window, cameraPos);
}
