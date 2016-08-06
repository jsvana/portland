#pragma once

#include "screens/screen.h"

#include <memory>

namespace ScreenManager {

/**
 * Gets the topmost screen on the stack
 *
 * @return Topmost screen
 */
std::shared_ptr<Screen> top();

/**
 * Adds a new screen on top of the stack
 *
 * @param screen Screen to push on the stack
 */
void push(Screen *screen);

/**
 * Adds a new screen on top of the stack
 *
 * @param screen shared Screen pointer to push on the stack
 */
void push(std::shared_ptr<Screen>);

/**
 * Removes the topmost screen from the stack
 *
 * @return Removed topmost screen
 */
std::shared_ptr<Screen> pop();

/**
 * Removes the topmost screen, pushes the given screen, and
 * returns the removed screen
 *
 * @param screen Screen to push on the stack
 * @return Removed topmost screen
 */
std::shared_ptr<Screen> replace(Screen *screen);

/**
 * Removes the topmost screen, pushes the given screen, and
 * returns the removed screen
 *
 * @param screen shared Screen pointer to push on the stack
 * @return Removed topmost screen
 */
std::shared_ptr<Screen> replace(std::shared_ptr<Screen> screen);
}
