#include "engine.h"

#include "asset_manager.h"
#include "constants.h"
#include "state.h"
#include "util.h"

#include "spdlog/spdlog.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

SDL_Renderer *renderer = nullptr;

namespace Engine {
  std::stack<std::shared_ptr<Screen>> screens;

  SDL_Window *window = nullptr;
  SDL_Texture *backBuffer = nullptr;
  SDL_Rect nativeSize;
  SDL_Rect newWindowSize;

  bool init() {
    nativeSize.x = 0;
    nativeSize.y = 0;
    nativeSize.w = SCREEN_WIDTH;
    nativeSize.h = SCREEN_HEIGHT;

    newWindowSize.x = 0;
    newWindowSize.y = 0;
    newWindowSize.w = 2 * nativeSize.w;
    newWindowSize.h = 2 * nativeSize.h;

    // Initialize
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
      err()->error("SDL_Init err->r: {}", SDL_GetError());
      return false;
    }

    SDL_ShowCursor(SDL_DISABLE);

    // Set the scaling quality to nearest pixel
    if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0") < 0) {
      err()->error("Failed to set render scale quality");
      return false;
    }

    // load support for the JPG and PNG image formats
    int image_flags = IMG_INIT_JPG | IMG_INIT_PNG;
    if ((IMG_Init(image_flags) & image_flags) != image_flags) {
      err()->error("Failed to init image support: {}", IMG_GetError());
      return false;
    }

    if (TTF_Init() < 0) {
      err()->error("Failed to init TTF support: {}", TTF_GetError());
      return false;
    }

    // Create window
    window = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, newWindowSize.w,
                              newWindowSize.h,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
      err()->error("SDL_CreateWindow err->r: {}", SDL_GetError());
      return false;
    }

    // Create renderer for window
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED |
                                                  SDL_RENDERER_PRESENTVSYNC |
                                                  SDL_RENDERER_TARGETTEXTURE);
    if (renderer == nullptr) {
      err()->error("SDL_CreateRenderer err->r: {}", SDL_GetError());
      return false;
    }

    backBuffer =
        SDL_CreateTexture(renderer, SDL_GetWindowPixelFormat(window),
                          SDL_TEXTUREACCESS_TARGET, nativeSize.w, nativeSize.h);
    if (backBuffer == nullptr) {
      err()->error("SDL_CreateTexture err->r: {}", SDL_GetError());
      return false;
    }

    SDL_SetRenderTarget(renderer, backBuffer);

    GameState::initLuaApi();

    return true;
  }

  void run() {
    unsigned long frames = 0;
    bool resizeFlag = false;
    SDL_Event event;
    unsigned int nextTick = SDL_GetTicks();
    int loops;

    while (true) {
      auto screen = screens.top();

      loops = 0;

      while (SDL_GetTicks() > nextTick && loops < MAX_FRAMESKIP) {
        // Handle events
        while (SDL_PollEvent(&event)) {
          if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_q) {
              return;
            }
          } else if (event.type == SDL_QUIT) {
            return;
          } else if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
              resizeFlag = true;
            } else if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
              return;
            }
          }

          screen->handleEvent(event);
        }

        GameState::setTicks(frames);

        if (!screen->update(frames)) {
          return;
        }

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

      if (resizeFlag) {
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

        // Half of the difference between the native size and the window size
        // will
        // give us the size the black bars should be.
        newWindowSize.x = (w - newWindowSize.w) / 2;
        newWindowSize.y = (h - newWindowSize.h) / 2;

        SDL_DestroyTexture(backBuffer);
        backBuffer = SDL_CreateTexture(
            renderer, SDL_GetWindowPixelFormat(window),
            SDL_TEXTUREACCESS_TARGET, nativeSize.w, nativeSize.h);

        resizeFlag = false;
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
  }

  void cleanup() {
    while (!screens.empty()) {
      popScreen();
    }

    AssetManager::destroy();
    if (backBuffer != nullptr) {
      SDL_DestroyTexture(backBuffer);
    }
    if (renderer != nullptr) {
      SDL_DestroyRenderer(renderer);
    }
    if (window != nullptr) {
      SDL_DestroyWindow(window);
    }
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
  }

  void pushScreen(Screen *screen) { pushScreen(std::shared_ptr<Screen>(screen)); }

  void pushScreen(std::shared_ptr<Screen> screen) { screens.push(screen); }

  std::shared_ptr<Screen> popScreen() {
    auto top = screens.top();
    screens.pop();
    return top;
  }

  std::shared_ptr<Screen> replaceScreen(Screen *screen) {
    auto replaced = popScreen();
    pushScreen(screen);
    return replaced;
  }
}
