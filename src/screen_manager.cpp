#include "screen_manager.h"

#include <stack>

namespace ScreenManager {
std::stack<std::shared_ptr<Screen>> screens;

std::shared_ptr<Screen> top() { return screens.top(); }

void push(Screen *screen) { push(std::shared_ptr<Screen>(screen)); }

void push(std::shared_ptr<Screen> screen) { screens.push(screen); }

std::shared_ptr<Screen> pop() {
  auto top = screens.top();
  screens.pop();
  return top;
}

std::shared_ptr<Screen> replace(Screen *screen) {
  auto replaced = pop();
  push(screen);
  return replaced;
}
}
