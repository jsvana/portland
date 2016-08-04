#pragma once

#include "../util.h"

#include "../lib/json.hpp"

#include <SDL.h>

#include <string>

enum SpriteDirection {
  SPRITE_LEFT,
  SPRITE_RIGHT,
  SPRITE_UP,
  SPRITE_DOWN,
};

/**
 * Class to load, render, and update sprites onscreen
 */
class Sprite {
 private:
  Rect dimensions_;
  int totalFrames_ = 0;
  int tile_;
  float scale_;
  bool multiFile_;
  int frameSpacing_;

  int frame_ = 0;

  int hp_;
  int maxHp_;

  // Ticks at last frame transition
  unsigned long lastTicks_ = 0;

  const unsigned long FRAME_TICKS_INTERVAL = 24;

  Rect textureDimensions_;
  std::vector<SDL_Texture *> textures_;

  SpriteDirection direction_;
  SpriteDirection visualDirection_;

  /**
   * Loads sprite from given path
   *
   * @param path Path to load sprite from
   * @param Whether operation was successful
   */
  bool load(const std::string &path);

 public:
  // Name of Lua function to call when sprite is interacted with
  std::string callbackFunc;

  Sprite(const std::string &path);

  /**
   * Gets dimensions of the sprite after scaling
   *
   * @return Scaled sprite dimensions
   */
  Rect getDimensions() {
    Rect dim = dimensions_;
    dim.w = (int)((float)dim.w * scale_);
    dim.h = (int)((float)dim.h * scale_);
    return dim;
  }

  /**
   * Sets dimensions of sprite after scaling
   *
   * @param dimensions New dimensions of sprite
   */
  void setDimensions(Rect dimensions) {
    dimensions.w = (int)((float)dimensions.w / scale_);
    dimensions.h = (int)((float)dimensions.h / scale_);
    dimensions_ = dimensions;
  }

  /**
   * Gets position of sprite
   *
   * @return Position of sprite
   */
  Point getPosition() {
    Point position;
    position.x = dimensions_.x;
    position.y = dimensions_.y;
    return position;
  }

  /**
   * Sets position of sprite
   *
   * @param x New x coordinate of sprite
   * @param y New y coordinate of sprite
   */
  void setPosition(int x, int y) {
    dimensions_.x = x;
    dimensions_.y = y;
  }

  /**
   * Sets position of sprite
   *
   * @param position New position of sprite
   */
  void setPosition(Point position) {
    dimensions_.x = position.x;
    dimensions_.y = position.y;
  }

  /**
   * Moves sprite by given (dx, dy)
   *
   * @param dx Distance to move x coordinate
   * @param dy Distance to move y coordinate
   */
  void move(int dx, int dy) {
    dimensions_.x += dx;
    dimensions_.y += dy;
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
    if (dir == SPRITE_LEFT || dir == SPRITE_RIGHT) {
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
  void fullHeal() {
    hp_ = maxHp_;
  }

  /**
   * Increases HP by the given amount and clamp to max HP
   *
   * @param amount Amount to increase HP by
   */
  void heal(int amount) {
    hp_ += amount;
    clamp<int>(hp_, 0, maxHp_);
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
    clamp<int>(hp_, 0, maxHp_);
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
  int width() { return (int)((float)dimensions_.w * scale_); }

  /**
   * Gets scaled height of sprite
   *
   * @return Scaled height of sprite
   */
  int height() { return (int)((float)dimensions_.h * scale_); }

  /**
   * Sets sprite's tile in spritesheet
   *
   * @param tile New spritesheet tile for sprite
   */
  void setTile(int tile) { tile_ = tile; }

  /**
   * Animates sprite
   *
   * @param ticks Number of ticks from start
   */
  void update(unsigned long ticks);

  /**
   * Renders sprite relative to cameraPos
   *
   * @param cameraPos Position of camera to render sprite
   * relative to
   */
  void render(Point cameraPos) const;
};
