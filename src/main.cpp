#include "constants.h"
#include "engine.h"
#include "log.h"
#include "util.h"

#include "screens/opening.h"

int main(int, char**) {
  logger::init("portland.log");

  if (!Engine::init()) {
    logger::error("Error loading engine");
    Engine::cleanup();
    return 1;
  }

  Engine::pushScreen(new OpeningScreen());

  Engine::run();

  logger::info("Thanks for playing!");

  Engine::cleanup();

  logger::cleanup();

  return 0;
}
