#include "asset_manager.h"
#include "constants.h"
#include "screen_manager.h"
#include "screens/opening.h"
#include "state.h"
#include "util.h"

#include "engine.h"

#include <iostream>

int main(int, char **) {
  if (!Engine::init()) {
    err()->error("Error loading engine");
    Engine::cleanup();
    return 1;
  }

  ScreenManager::push(new OpeningScreen(SCREEN_WIDTH, SCREEN_HEIGHT));

  Engine::run();

  out()->info("Thanks for playing!");

  Engine::cleanup();

  return 0;
}
