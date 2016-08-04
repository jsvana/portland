#pragma once

#include "screens/screen.h"

namespace ScreenManager {

/**
 * Gets the topmost screen on the stack
 *
 * @return Topmost screen
 */
Screen *top();

/**
 * Adds a new screen on top of the stack
 *
 * @param screen Screen to push on the stack
 */
void push(Screen *screen);

/**
 * Removes the topmost screen from the stack
 *
 * @return Removed topmost screen
 */
Screen *pop();

/**
 * Removes the topmost screen, pushes the given screen, and
 * returns the removed screen
 *
 * @param screen Screen to push on the stack
 * @return Removed topmost screen
 */
Screen *replace(Screen *screen);
}
