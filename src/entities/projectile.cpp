#include "projectile.h"

#include <cmath>

namespace entities {

void Projectile::update(const sf::Time& time) {
  Sprite::update(time);

  move(speed_, 0);
  moved_ += std::fabs(speed_);
  if (moved_ > maxDistance_) {
    markNeedsCleanup();
  }
}

}  // namespace entities
