#include "asset_manager.h"
#include "constants.h"
#include "screen_manager.h"
#include "screens/opening.h"
#include "state.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <iostream>

SDL_Renderer *renderer;

int main(int, char **) {
  SDL_Rect nativeSize;
  nativeSize.x = 0;
  nativeSize.y = 0;
  nativeSize.w = SCREEN_WIDTH;
  nativeSize.h = SCREEN_HEIGHT;

  SDL_Rect newWindowSize;
  newWindowSize.x = 0;
  newWindowSize.y = 0;
  newWindowSize.w = 2 * nativeSize.w;
  newWindowSize.h = 2 * nativeSize.h;

  // Initialize
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  SDL_ShowCursor(SDL_DISABLE);

  // Set the scaling quality to nearest pixel
  if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0") < 0) {
    std::cout << "Failed to set render scale quality" << std::endl;
    SDL_Quit();
    return 1;
  }

  // load support for the JPG and PNG image formats
  int image_flags = IMG_INIT_JPG | IMG_INIT_PNG;
  if ((IMG_Init(image_flags) & image_flags) != image_flags) {
    std::cout << "Failed to init JPG and PNG support" << std::endl;
    std::cout << IMG_GetError() << std::endl;
    SDL_Quit();
    return 1;
  }

  if (TTF_Init() < 0) {
    std::cout << "Failed to init TTF support" << std::endl;
    std::cout << TTF_GetError() << std::endl;
    IMG_Quit();
    SDL_Quit();
  }

  // Create window
  SDL_Window *window = SDL_CreateWindow(
      "Hello World!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      newWindowSize.w, newWindowSize.h,
      SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if (window == nullptr) {
    std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 1;
  }

  // Create renderer for window
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED |
                                                SDL_RENDERER_PRESENTVSYNC |
                                                SDL_RENDERER_TARGETTEXTURE);
  if (renderer == nullptr) {
    std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 1;
  }

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

  SDL_Texture *backBuffer =
      SDL_CreateTexture(renderer, SDL_GetWindowPixelFormat(window),
                        SDL_TEXTUREACCESS_TARGET, nativeSize.w, nativeSize.h);

  SDL_SetRenderTarget(renderer, backBuffer);

  GameState::initLuaApi();

  ScreenManager::push(new OpeningScreen(nativeSize.w, nativeSize.h));

  bool running = true;
  unsigned long frames = 0;
  SDL_Event event;
  bool resize = false;
  int loops;

  unsigned int nextTick = SDL_GetTicks();
  while (running) {
    auto screen = ScreenManager::top();

    loops = 0;

    while (SDL_GetTicks() > nextTick && loops < MAX_FRAMESKIP) {
      // Handle events
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYDOWN) {
          if (event.key.keysym.sym == SDLK_q) {
            running = false;
          }
        } else if (event.type == SDL_QUIT) {
          running = false;
        } else if (event.type == SDL_WINDOWEVENT) {
          if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            resize = true;
          } else if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
            running = false;
          }
        }

        screen->handleEvent(event);
      }

      GameState::setTicks(frames);

      running = screen->update(frames);

      nextTick += MILLISECONDS_PER_FRAME;
      loops += 1;
    }

    // Calculate interpolation
    float interpolation =
        (float)(SDL_GetTicks() + MILLISECONDS_PER_FRAME - nextTick) /
        (float)MILLISECONDS_PER_FRAME;

    // Render to backbuffer and copy to screen
    screen->render(interpolation);
    SDL_SetRenderTarget(renderer, NULL);

    if (resize) {
      int w, h;
      SDL_GetWindowSize(window, &w, &h);

      newWindowSize.w = w;
      newWindowSize.h = h;

      SDL_DestroyTexture(backBuffer);
      backBuffer = SDL_CreateTexture(renderer, SDL_GetWindowPixelFormat(window),
                                     SDL_TEXTUREACCESS_TARGET, nativeSize.w,
                                     nativeSize.h);

      SDL_Rect viewPort;
      SDL_RenderGetViewport(renderer, &viewPort);

      if (viewPort.w != newWindowSize.w || viewPort.h != newWindowSize.h) {
        SDL_RenderSetViewport(renderer, &newWindowSize);
      }

      resize = false;
    }

    SDL_RenderCopy(renderer, backBuffer, &nativeSize, &newWindowSize);
    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);

    SDL_SetRenderTarget(renderer, backBuffer);
    SDL_RenderClear(renderer);

    frames += 1;
  }

  std::cout << "Thanks for playing!" << std::endl;

  AssetManager::destroy();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();

  return 0;
}
