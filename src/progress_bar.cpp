#include "progress_bar.h"

extern SDL_Renderer *renderer;

void ProgressBar::render() {
  SDL_Rect rect;

  Uint8 r, g, b, a;
  SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);

  // Draw background
  rect = dimensions_.toSdlRect();
  rect.x -= PADDING;
  rect.y -= PADDING;
  rect.w += 2 * PADDING;
  rect.h += 2 * PADDING;
  SDL_SetRenderDrawColor(renderer, 200, 200, 200, a);
  SDL_RenderFillRect(renderer, &rect);

  // Draw outer rect
  rect = dimensions_.toSdlRect();
  SDL_SetRenderDrawColor(renderer, 50, 50, 50, a);
  SDL_RenderFillRect(renderer, &rect);

  // Draw inner rect
  rect = fillDimensions_.toSdlRect();
  SDL_SetRenderDrawColor(renderer, 200, 50, 50, a);
  SDL_RenderFillRect(renderer, &rect);

  SDL_SetRenderDrawColor(renderer, r, g, b, a);
}
