#include "asset_manager.h"
#include "constants.h"
#include "screen_manager.h"
#include "screens/opening.h"
#include "state.h"

#include "spdlog/spdlog.h"

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

  auto err = spdlog::stderr_logger_mt("error", true);
  auto out = spdlog::stdout_logger_mt("out", true);

  // Initialize
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    err->error("SDL_Init err->r: {}", SDL_GetError());
    return 1;
  }

  SDL_ShowCursor(SDL_DISABLE);

  // Set the scaling quality to nearest pixel
  if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0") < 0) {
    err->error("Failed to set render scale quality");
    SDL_Quit();
    return 1;
  }

  // load support for the JPG and PNG image formats
  int image_flags = IMG_INIT_JPG | IMG_INIT_PNG;
  if ((IMG_Init(image_flags) & image_flags) != image_flags) {
    err->error("Failed to init image support: {}", IMG_GetError());
    SDL_Quit();
    return 1;
  }

  if (TTF_Init() < 0) {
    err->error("Failed to init TTF support: {}", TTF_GetError());
    IMG_Quit();
    SDL_Quit();
  }

  // Create window
  SDL_Window *window = SDL_CreateWindow(
      "Hello World!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      newWindowSize.w, newWindowSize.h,
      SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if (window == nullptr) {
    err->error("SDL_CreateWindow err->r: {}", SDL_GetError());
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
    err->error("SDL_CreateRenderer err->r: {}", SDL_GetError());
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

  // IDs will start at 1, the hero gets 0
  GameState::hero()->id = 0;

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

        if (!running) {
          goto exit;
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

      // The smaller of the two, based on the aspect ratio should be the base
      // size. Then we can resize the other, based on the aspect ratio.
      if (w * 9 > h * 16) {
        newWindowSize.w = h * 16 / 9;
        newWindowSize.h = h;
      } else {
        newWindowSize.w = w;
        newWindowSize.h = w * 9 / 16;
      }

      // Half of the difference between the native size and the window size will
      // give us the size the black bars should be.
      newWindowSize.x = (w - newWindowSize.w) / 2;
      newWindowSize.y = (h - newWindowSize.h) / 2;

      SDL_DestroyTexture(backBuffer);
      backBuffer = SDL_CreateTexture(renderer, SDL_GetWindowPixelFormat(window),
                                     SDL_TEXTUREACCESS_TARGET, nativeSize.w,
                                     nativeSize.h);

      resize = false;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_RenderCopy(renderer, backBuffer, &nativeSize, &newWindowSize);
    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);

    SDL_SetRenderTarget(renderer, backBuffer);
    SDL_RenderClear(renderer);

    frames += 1;
  }

exit:
  out->info("Thanks for playing!");

  AssetManager::destroy();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  TTF_Quit();
  IMG_Quit();
  SDL_Quit();

  return 0;
}
