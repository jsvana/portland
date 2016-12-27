#include "controls.h"

#include <unordered_map>

namespace controls {

std::unordered_map<util::Direction, std::vector<sf::Keyboard::Key>>
    directionKeys;

std::vector<sf::Keyboard::Key> jumpKeys;

void init() {
  directionKeys[util::Direction::UP] = {sf::Keyboard::Up, sf::Keyboard::W};
  directionKeys[util::Direction::DOWN] = {sf::Keyboard::Down, sf::Keyboard::S};
  directionKeys[util::Direction::LEFT] = {sf::Keyboard::Left, sf::Keyboard::A};
  directionKeys[util::Direction::RIGHT] = {sf::Keyboard::Right,
                                           sf::Keyboard::D};

  jumpKeys.push_back(sf::Keyboard::Up);
  jumpKeys.push_back(sf::Keyboard::W);
  jumpKeys.push_back(sf::Keyboard::Space);
}

void setDirectionKeys(const util::Direction direction,
                      const std::vector<sf::Keyboard::Key>& keys) {
  directionKeys[direction].clear();
  directionKeys[direction] = keys;
}

bool anyKeyPressed(const std::vector<sf::Keyboard::Key>& keys) {
  for (const auto& key : keys) {
    if (sf::Keyboard::isKeyPressed(key)) {
      return true;
    }
  }
  return false;
}

bool directionPressed(util::Direction direction) {
  return anyKeyPressed(directionKeys[direction]);
}

bool jumpPressed() { return anyKeyPressed(jumpKeys); }

}  // namespace controls
