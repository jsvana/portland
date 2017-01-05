#include "item.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace entities {

void Item::update(const sf::Time& time) { Sprite::update(time); }

}  // namespace entities
