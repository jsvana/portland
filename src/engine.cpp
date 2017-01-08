#include "engine.h"

#include "constants.h"
#include "controls.h"
#include "log.h"
#include "state.h"
#include "util.h"

#include <SFML/Graphics.hpp>

namespace Engine {
std::stack<std::unique_ptr<Screen>> screens;

sf::RenderWindow window;

bool running_ = true;

bool init() {
  sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
  int scale =
      std::min(desktop.width / SCREEN_WIDTH, desktop.height / SCREEN_HEIGHT);

  logger::info("Desktop width: " + std::to_string(desktop.width));
  logger::info("Desktop height: " + std::to_string(desktop.height));

  logger::info("Screen scale ratio: " + std::to_string(scale));

  window.create(sf::VideoMode(SCREEN_WIDTH * scale, SCREEN_HEIGHT * scale),
                "Portland");

  window.setFramerateLimit(60);
  window.setVerticalSyncEnabled(true);

  GameState::initApi();

  controls::init();

  logger::info("Game initialized");

  return true;
}

void run() {
  sf::Clock clock;

  sf::RenderTexture target;
  target.create(SCREEN_WIDTH, SCREEN_HEIGHT);

#ifdef __APPLE__
  // See SFML bug #1132
  window.display();
#endif

  while (window.isOpen() && running_) {
    sf::Time elapsed = clock.restart();

    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
        return;
      } else if (event.type == sf::Event::Resized) {
        const auto windowSize = window.getSize();
        window.setView(
            sf::View(sf::FloatRect(0.f, 0.f, (float)windowSize.x, (float)windowSize.y)));
      }

      screens.top()->handleEvent(event);
    }

    running_ = screens.top()->update(elapsed);

    auto windowSize = window.getSize();

    target.clear(sf::Color::Black);
    screens.top()->render(target);
    target.display();

    sf::Sprite rendered(target.getTexture());
    rendered.setOrigin(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    rendered.setPosition((float)windowSize.x / 2, (float)windowSize.y / 2);

    float scale = std::min(1.0f * windowSize.y / SCREEN_HEIGHT,
                           1.0f * windowSize.x / SCREEN_WIDTH);

    rendered.setScale(scale, scale);

    window.clear(sf::Color::Black);
    window.draw(rendered);
    window.display();
  }
}

void cleanup() {
  while (!screens.empty()) {
    popScreen();
  }
}

void pushScreen(Screen* screen) { pushScreen(std::unique_ptr<Screen>(screen)); }

void pushScreen(std::unique_ptr<Screen> screen) {
  screens.push(std::move(screen));
}

std::unique_ptr<Screen> popScreen() {
  auto top = std::move(screens.top());
  screens.pop();
  return top;
}

std::unique_ptr<Screen> replaceScreen(Screen* screen) {
  auto replaced = popScreen();
  pushScreen(screen);
  return replaced;
}
}
