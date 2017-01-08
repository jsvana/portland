#include "controls.h"

#include <unordered_map>

namespace controls {

std::unordered_map<util::Direction, std::vector<sf::Keyboard::Key>>
    directionKeys;

std::vector<sf::Keyboard::Key> jumpKeys;
std::vector<sf::Keyboard::Key> actionKeys;
std::vector<sf::Keyboard::Key> attackKeys;

void init() {
  directionKeys[util::Direction::UP] = {sf::Keyboard::Up, sf::Keyboard::W};
  directionKeys[util::Direction::DOWN] = {sf::Keyboard::Down, sf::Keyboard::S};
  directionKeys[util::Direction::LEFT] = {sf::Keyboard::Left, sf::Keyboard::A};
  directionKeys[util::Direction::RIGHT] = {sf::Keyboard::Right,
                                           sf::Keyboard::D};

  jumpKeys = {sf::Keyboard::Up, sf::Keyboard::W, sf::Keyboard::Space};
  actionKeys = {sf::Keyboard::Return};
  attackKeys = {sf::Keyboard::C};
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

bool anyKeyEvent(const std::vector<sf::Keyboard::Key>& keys,
                 const sf::Keyboard::Key code) {
  for (const auto& key : keys) {
    if (key == code) {
      return true;
    }
  }
  return false;
}

bool directionPressed(util::Direction direction) {
  return anyKeyPressed(directionKeys[direction]);
}

bool jumpPressed() { return anyKeyPressed(jumpKeys); }

bool actionPressed() { return anyKeyPressed(actionKeys); }

bool attackPressed() { return anyKeyPressed(attackKeys); }

bool jumpEvent(const sf::Keyboard::Key code) {
  return anyKeyEvent(jumpKeys, code);
}

bool actionEvent(const sf::Keyboard::Key code) {
  return anyKeyEvent(actionKeys, code);
}

bool attackEvent(const sf::Keyboard::Key code) {
  return anyKeyEvent(attackKeys, code);
}

}  // namespace controls
