#pragma once

#include "../util.h"

#include <json.hpp>

#include <SFML/Graphics.hpp>

#include <string>

namespace entities {

typedef std::function<void()> SpriteCallback;

enum class SpriteDirection : int {
  LEFT = 0,
  RIGHT = 1,
  UP = 2,
  DOWN = 3,
};

enum class SpriteType : int {
  HERO = 0,
  ITEM = 1,
  NPC = 2,
};

/**
 * Class to load, render, and update sprites onscreen
 */
class Sprite {
 private:
  const float GRAVITY = .5;
  const float STARTING_JUMP_VELOCITY = -10;

  SpriteType type_;

  sf::FloatRect dimensions_;
  int totalFrames_ = 0;
  int tile_;
  float scale_;
  bool multiFile_;
  int frameSpacing_;
  int columns_;

  int frame_ = 0;

  int hp_;
  int maxHp_;

  bool canJump_ = true;
  bool jumping_ = false;
  float velocityY_ = 0;

  sf::Time time_;

  // Ticks at last frame transition
  unsigned long lastTicks_ = 0;

  const unsigned long FRAME_TICKS_INTERVAL = 24;

  sf::FloatRect textureDimensions_;
  std::vector<sf::Texture> textures_;
  sf::Sprite sprite_;

  SpriteDirection direction_;
  SpriteDirection visualDirection_;

  /**
   * Loads sprite from given path
   *
   * @param path Path to load sprite from
   * @param Whether operation was successful
   */
  bool load(const std::string& path);

 public:
  // Name of Lua function to call when sprite is interacted with
  SpriteCallback callbackFunc;

  unsigned int id;

  Sprite(const std::string& path) : Sprite(path, SpriteType::HERO) {}

  Sprite(const std::string& path, SpriteType type);

  SpriteType type() { return type_; }

  /**
   * Gets dimensions of the sprite after scaling
   *
   * @return Scaled sprite dimensions
   */
  sf::FloatRect getDimensions() {
    sf::FloatRect dim = dimensions_;
    dim.width = dim.width * scale_;
    dim.height = dim.height * scale_;
    return dim;
  }

  /**
   * Sets dimensions of sprite after scaling
   *
   * @param dimensions New dimensions of sprite
   */
  void setDimensions(sf::FloatRect dimensions) {
    dimensions.width = dimensions.width / scale_;
    dimensions.height = dimensions.height / scale_;
    dimensions_ = dimensions;
    dimensions_.top = (int)dimensions_.top;
  }

  /**
   * Gets position of sprite
   *
   * @return Position of sprite
   */
  sf::Vector2f getPosition() {
    sf::Vector2f position;
    position.x = dimensions_.left;
    position.y = dimensions_.top;
    return position;
  }

  /**
   * Sets position of sprite
   *
   * @param x New x coordinate of sprite
   * @param y New y coordinate of sprite
   */
  void setPosition(int x, int y) {
    dimensions_.left = x;
    dimensions_.top = y;
  }

  /**
   * Sets position of sprite
   *
   * @param position New position of sprite
   */
  void setPosition(sf::Vector2f position) {
    dimensions_.left = position.x;
    dimensions_.top = position.y;
  }

  /**
   * Moves sprite by given (dx, dy)
   *
   * @param dx Distance to move x coordinate
   * @param dy Distance to move y coordinate
   */
  void move(int dx, int dy) {
    dimensions_.left += dx;
    dimensions_.top += dy;
  }

  /**
   * Sets sprite direction
   *
   * Sprite only renders left and right and will only update
   * render direction in those cases
   *
   * @param dir New sprite direction
   */
  void setDirection(SpriteDirection dir) {
    direction_ = dir;
    if (dir == SpriteDirection::LEFT || dir == SpriteDirection::RIGHT) {
      visualDirection_ = dir;
    }
  }

  /**
   * Gets current sprite direction
   *
   * @return Current sprite direction
   */
  SpriteDirection getDirection() { return direction_; }

  /**
   * Sets HP to max HP
   */
  void fullHeal() { hp_ = maxHp_; }

  /**
   * Increases HP by the given amount and clamp to max HP
   *
   * @param amount Amount to increase HP by
   */
  void heal(int amount) {
    hp_ += amount;
    util::clamp<int>(hp_, 0, maxHp_);
  }

  /**
   * Sets maximum HP and heals sprite
   *
   * @param maxHp value to set max HP to
   */
  void setMaxHp(int maxHp) {
    maxHp_ += maxHp;
    fullHeal();
  }

  /**
   * Decreases HP by the given amount and clamp to 0
   *
   * @param amount Amount to decrease HP by
   */
  void damage(int amount) {
    hp_ -= amount;
    util::clamp<int>(hp_, 0, maxHp_);
  }

  /**
   * Gets current HP
   *
   * @return Current HP
   */
  int hp() { return hp_; }

  /**
   * Gets scaled width of sprite
   *
   * @return Scaled width of sprite
   */
  int width() { return dimensions_.width * scale_; }

  /**
   * Gets scaled height of sprite
   *
   * @return Scaled height of sprite
   */
  int height() { return dimensions_.height * scale_; }

  /**
   * Sets sprite's tile in spritesheet
   *
   * @param tile New spritesheet tile for sprite
   */
  void setTile(int tile) { tile_ = tile; }

  /**
   * Gets the sprite's vertical velocity
   *
   * @return Sprite's vertical velocity
   */
  float velocity() { return velocityY_; }

  /**
   * Updates the vertical velocity with the gravity constant
   */
  void updateVelocity();

  /**
   * Updates the vertical velocity with the gravity constant
   */
  void setVelocity(float velocity);

  /**
   * Initiates jump by setting vertical velocity
   *
   * @param magnitudePercent Percentage of total jump to start
   */
  void startJump(float magnitudePercent);

  /**
   * Clears out falling velocity
   *
   * @param stopJump Whether or not jump should stop
   */
  void zeroVelocity(bool stopJump);

  /**
   * Gets whether or not sprite is jumping
   *
   * @return Whether or not sprite is jumping
   */
  bool jumping() { return jumping_; }

  /**
   * Gets whether or not sprite can jump
   *
   * @return Whether or not sprite can jump
   */
  bool canJump() { return canJump_; }

  /**
   * Allows sprite to jump
   */
  void allowJump() { canJump_ = true; }

  /**
   * Forbids player from jumping
   */
  void forbidJump() { canJump_ = false; }

  /**
   * Animates sprite
   *
   * @param time Amount of time since last update
   */
  void update(sf::Time& time);

  /**
   * Renders sprite relative to cameraPos
   *
   * @param window Window to render to
   * @param cameraPos Position of camera to render sprite
   * relative to
   */
  void render(sf::RenderTarget& window, sf::Vector2f cameraPos);
};

}  // namespace entities
