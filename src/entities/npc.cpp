#include "npc.h"

#include <SDL.h>

#include <iostream>

extern SDL_Renderer *renderer;

void Npc::render(Point cameraPos) const { Sprite::render(cameraPos); }
