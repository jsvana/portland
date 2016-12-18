#include "constants.h"
#include "engine.h"
#include "util.h"

#include "screens/opening.h"

#include <glog/logging.h>

int main(int, char* argv[]) {
  // Set up logging
  FLAGS_logtostderr = 1;
  google::InitGoogleLogging(argv[0]);

  if (!Engine::init()) {
    LOG(ERROR) << "Error loading engine";
    Engine::cleanup();
    return 1;
  }

  Engine::pushScreen(new OpeningScreen());

  Engine::run();

  LOG(INFO) << "Thanks for playing!";

  Engine::cleanup();

  return 0;
}
