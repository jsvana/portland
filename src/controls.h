#pragma once

#include "util.h"

#include <SFML/Graphics.hpp>

namespace controls {

void init();

void setDirectionKeys(const util::Direction direction,
                      const std::vector<sf::Keyboard::Key>& keys);

bool anyKeyPressed(const std::vector<sf::Keyboard::Key>& keys);
bool anyKeyEvent(const std::vector<sf::Keyboard::Key>& keys,
                 const sf::Keyboard::Key code);

bool directionPressed(util::Direction direction);

bool jumpPressed();

bool actionPressed();

bool attackPressed();

bool jumpEvent(const sf::Keyboard::Key code);

bool actionEvent(const sf::Keyboard::Key code);

bool attackEvent(const sf::Keyboard::Key code);

}  // namespace controls
