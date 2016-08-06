#pragma once

#include "screens/screen.h"

#include <memory>

namespace Engine {
  bool init();
  void run();
  void cleanup();

  /**
   * Adds a new screen on top of the stack
   *
   * @param screen Screen to push on the stack
   */
  void pushScreen(Screen *screen);

  /**
   * Adds a new screen on top of the stack
   *
   * @param screen shared Screen pointer to push on the stack
   */
  void pushScreen(std::shared_ptr<Screen>);

  /**
   * Removes the topmost screen from the stack
   *
   * @return Removed topmost screen
   */
  std::shared_ptr<Screen> popScreen();

  /**
   * Removes the topmost screen, pushes the given screen, and
   * returns the removed screen
   *
   * @param screen Screen to push on the stack
   * @return Removed topmost screen
   */
  std::shared_ptr<Screen> replaceScreen(Screen *screen);

  /**
   * Removes the topmost screen, pushes the given screen, and
   * returns the removed screen
   *
   * @param screen shared Screen pointer to push on the stack
   * @return Removed topmost screen
   */
  std::shared_ptr<Screen> replaceScreen(std::shared_ptr<Screen> screen);
}
