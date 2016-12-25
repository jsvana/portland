#include "controls.h"

#include <unordered_map>

namespace controls {

std::unordered_map<util::Direction, std::vector<sf::Keyboard::Key>>
    directionKeys;

std::vector<sf::Keyboard::Key> jumpKeys;

void init() {
  directionKeys[util::Direction::UP].push_back(sf::Keyboard::Up);
  directionKeys[util::Direction::UP].push_back(sf::Keyboard::W);
  directionKeys[util::Direction::DOWN].push_back(sf::Keyboard::Down);
  directionKeys[util::Direction::DOWN].push_back(sf::Keyboard::S);
  directionKeys[util::Direction::LEFT].push_back(sf::Keyboard::Left);
  directionKeys[util::Direction::LEFT].push_back(sf::Keyboard::A);
  directionKeys[util::Direction::RIGHT].push_back(sf::Keyboard::Right);
  directionKeys[util::Direction::RIGHT].push_back(sf::Keyboard::D);

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
