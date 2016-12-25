#pragma once

#include "util.h"

#include <SFML/Graphics.hpp>

namespace controls {

void init();

void setDirectionKeys(const util::Direction direction,
                      const std::vector<sf::Keyboard::Key>& keys);

bool anyKeyPressed(const std::vector<sf::Keyboard::Key>& keys);

bool directionPressed(util::Direction direction);

bool jumpPressed();

}  // namespace controls
