#include "engine.h"

#include "constants.h"
#include "state.h"
#include "util.h"

#include "spdlog/spdlog.h"

#include <SFML/Graphics.hpp>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

namespace Engine {
  std::stack<std::shared_ptr<Screen>> screens;

  sf::RenderWindow window;

  bool running_ = true;

  bool init() {
    window.create(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Portland");

    window.setFramerateLimit(60);

    GameState::initLuaApi();

    out()->info("Game initialized");

    return true;
  }

  void run() {
    sf::Clock clock;

    sf::RenderTexture target;
    target.create(SCREEN_WIDTH, SCREEN_HEIGHT);

    while (window.isOpen() && running_) {
      sf::Time elapsed = clock.restart();

      auto screen = screens.top();

      sf::Event event;
      while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
          window.close();
          return;
        } else if (event.type == sf::Event::Resized) {
          auto windowSize = window.getSize();
          window.setView(sf::View(sf::FloatRect(0.f, 0.f, windowSize.x, windowSize.y)));
        }

        screen->handleEvent(event);
      }

      running_ = screen->update(elapsed);

      auto windowSize = window.getSize();

      target.clear(sf::Color::Black);
      screen->render(target);
      target.display();

      sf::Sprite rendered(target.getTexture());
      rendered.setOrigin(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
      rendered.setPosition(windowSize.x / 2, windowSize.y / 2);

      float scale;
      if (windowSize.x * 9 > windowSize.y * 16) {
        scale = 1.0f * windowSize.y / SCREEN_HEIGHT;
      } else {
        scale = 1.0f * windowSize.x / SCREEN_WIDTH;
      }
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

  void pushScreen(Screen *screen) {
    pushScreen(std::shared_ptr<Screen>(screen));
  }

  void pushScreen(std::shared_ptr<Screen> screen) { screens.push(screen); }

  std::shared_ptr<Screen> popScreen() {
    auto top = screens.top();
    screens.pop();
    return top;
  }

  std::shared_ptr<Screen> replaceScreen(Screen *screen) {
    auto replaced = popScreen();
    pushScreen(screen);
    return replaced;
  }
}
