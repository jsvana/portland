#include "screen_manager.h"

#include <stack>

namespace ScreenManager {
std::stack<Screen *> screens;

Screen *top() { return screens.top(); }

void push(Screen *screen) { screens.push(screen); }

Screen *pop() {
  Screen *top = screens.top();
  screens.pop();
  return top;
}

Screen *replace(Screen *screen) {
  Screen *replaced = pop();
  push(screen);
  return replaced;
}
}
