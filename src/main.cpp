#include "constants.h"
#include "engine.h"
#include "util.h"

#include "screens/opening.h"

int main(int, char **) {
  if (!Engine::init()) {
    util::err()->error("Error loading engine");
    Engine::cleanup();
    return 1;
  }

  Engine::pushScreen(new OpeningScreen());

  Engine::run();

  util::out()->info("Thanks for playing!");

  Engine::cleanup();

  return 0;
}
