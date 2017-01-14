#pragma once

#include "../log.h"
#include "../map.h"
#include "../util.h"

#include <SFML/Graphics.hpp>
#include <json.hpp>

#include <set>
#include <string>
#include <unordered_map>

namespace entities {

typedef unsigned int Id;

typedef std::function<void()> SpriteCallback;
typedef std::function<void(Id, Id)> CollisionCallback;
typedef std::function<void(Id)> CleanupCallback;

enum class SpriteType : int {
  HERO = 0,
  ITEM = 1,
  NPC = 2,
  PROJECTILE = 3,
};

/**
 * Class to load, render, and update sprites onscreen
 */
class Sprite {
 protected:
  const float GRAVITY = .5;
  const float STARTING_JUMP_VELOCITY = -7;

  const std::string path_;
  SpriteType type_;

  sf::FloatRect dimensions_;
  int totalFrames_ = 0;
  map::TileId tile_;
  float scale_;
  bool multiFile_;
  int frameSpacing_;
  int columns_;

  sf::Time updateMs_;

  int frame_ = 0;

  int hp_ = 0;
  int maxHp_ = 0;

  bool canJump_ = true;
  bool jumping_ = false;
  float velocityY_ = 0;

  // Whether or not this sprite is updated, rendered, etc
  bool active_ = true;
  bool needsCleanup_ = false;

  sf::Time time_;

  std::set<Id> heldItems_;

  // Ticks at last frame transition
  util::Tick lastTicks_ = 0;

  const util::Tick FRAME_TICKS_INTERVAL = 24;

  sf::FloatRect textureDimensions_;
  std::vector<sf::Texture> textures_;
  sf::Sprite sprite_;

  util::Direction direction_;
  util::Direction visualDirection_;

  std::unordered_map<std::string, bool> flags_;
  std::unordered_map<std::string, int> values_;

  /**
   * Loads sprite from given path
   *
   * @param path Path to load sprite from
   * @param Whether operation was successful
   */
  bool load(const std::string& path);

  /**
   * Serializes a given FloatRect into a JSON blob
   *
   * @param rect FloatRect to serialize
   * @return Serialized JSON blob
   */
  const nlohmann::json serializeFloatRect(const sf::FloatRect& rect) {
    nlohmann::json out;
    out["left"] = rect.left;
    out["top"] = rect.top;
    out["width"] = rect.width;
    out["height"] = rect.height;
    return out;
  }

  /**
   * Deserializes a given FloatRect from a JSON blob
   *
   * @param data JSON blob to deserialize
   * @return Deserialized FloatRect
   */
  const sf::FloatRect deserializeFloatRect(
      const std::unordered_map<std::string, float> data) {
    sf::FloatRect rect;
    rect.left = data.find("left")->second;
    rect.top = data.find("top")->second;
    rect.width = data.find("width")->second;
    rect.height = data.find("height")->second;
    return rect;
  }

 public:
  // API function to call when sprite is interacted with
  SpriteCallback callbackFunc;

  // API function to call when sprite collides with another sprite
  CollisionCallback collisionFunc;

  // API function to call when sprite is marked for cleanup
  CleanupCallback cleanupFunc;

  Id id = 0;

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
   * Gets the sprite's held items
   *
   * @return Held items
   */
  const std::set<Id>& heldItems() { return heldItems_; }

  /**
   * Adds an item to the user's held item set
   *
   * @param itemId ID of item to add
   * @return Whether or not operation was successful
   */
  bool addItem(Id itemId) {
    if (heldItems_.find(itemId) != heldItems_.end()) {
      logger::error("Sprite " + std::to_string(id) + " is already holding " +
                    std::to_string(itemId));
      return false;
    }
    heldItems_.insert(itemId);
    return true;
  }

  /**
   * Checks if sprite is holding the given item
   *
   * @param itemId Item ID to check
   * @return Whether or not sprite is holding the item
   */
  bool holdingItem(const Id itemId) {
    return heldItems_.find(itemId) != heldItems_.end();
  }

  /**
   * Removes an item from the user's held item set
   *
   * @param itemId ID of item to remove
   * @return Whether or not operation was successful
   */
  bool removeItem(Id itemId) {
    auto iter = heldItems_.find(itemId);
    if (iter == heldItems_.end()) {
      logger::error("Sprite " + std::to_string(id) + " is not holding " +
                    std::to_string(itemId));
      return false;
    }
    heldItems_.erase(iter);
    return true;
  }

  /**
   * Sets the collision callback
   *
   * @param New collision callback
   */
  void setCollisionCallback(const CollisionCallback& func) {
    collisionFunc = func;
  }

  /**
   * Sets the cleanup callback
   *
   * @param New cleanup callback
   */
  void setCleanupCallback(const CleanupCallback& func) { cleanupFunc = func; }

  /**
   * Returns whether or not sprite is active
   * @return Whether or not sprite is active
   */
  bool active() { return active_; }

  /**
   * Activates sprite (enables updates, renders, etc)
   */
  void activate() { active_ = true; }

  /**
   * Deactivates sprite (disables updates, renders, etc)
   */
  void deactivate() { active_ = false; }

  /**
   * Returns whether or not sprite needs to be cleaned up
   * @return Whether or not sprite needs to be cleaned up
   */
  bool needsCleanup() { return needsCleanup_; }

  /**
   * Let the engine know that this sprite needs to be destroyed
   */
  void markNeedsCleanup() {
    deactivate();
    needsCleanup_ = true;
    if (cleanupFunc) {
      cleanupFunc(id);
    }
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
  void setPosition(const float x, const float y) {
    dimensions_.left = x;
    dimensions_.top = y;
  }

  /**
   * Moves sprite by given (dx, dy)
   *
   * @param dx Distance to move x coordinate
   * @param dy Distance to move y coordinate
   */
  void move(const float dx, const float dy) {
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
  void setDirection(util::Direction dir) {
    direction_ = dir;
    if (dir == util::Direction::LEFT || dir == util::Direction::RIGHT) {
      visualDirection_ = dir;
    }
  }

  /**
   * Sets sprite's visual direction
   *
   * @param dir New direction
   */
  void setVisualDirection(int dir) {
    auto utilDir = static_cast<util::Direction>(dir);
    visualDirection_ = utilDir;
    direction_ = utilDir;
  }

  /**
   * Gets current sprite direction
   *
   * @return Current sprite direction
   */
  // TODO(jsvana): figure out how to use == on enums in Chai so we don't
  // have to do this dumb casting
  int getDirection() { return static_cast<int>(direction_); }

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
  float width() { return dimensions_.width * scale_; }

  /**
   * Gets scaled height of sprite
   *
   * @return Scaled height of sprite
   */
  float height() { return dimensions_.height * scale_; }

  /**
   * Sets sprite's tile in spritesheet
   *
   * @param tile New spritesheet tile for sprite
   */
  void setTile(const map::TileId tile) { tile_ = tile; }

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
   * Whether or not this sprite can occupy the same space as another sprite
   *
   * @return Whether or not sprite can occupy the same space as another sprite
   */
  virtual bool phased() { return false; }

  /**
  * Set a sprite boolean value
  *
  * @param key Flag to set
  * @param value New value
  */
  void setFlag(const std::string& key, const bool value);

  /**
  * Get a sprite boolean value
  *
  * @param key Flag to get
  * @return Value or false if not set
  */
  bool getFlag(const std::string& key);

  /**
  * Set a sprite integer value
  *
  * @param key Value key to set
  * @param value New value
  */
  void setValue(const std::string& key, const int value);

  /**
  * Get a sprite integer value
  *
  * @param key Key to get
  * @return Value or 0 if not set
  */
  int getValue(const std::string& key);

  /**
   * Serializes the object into a JSON object
   *
   * @return Self as a serialized JSON object
   */
  const nlohmann::json serialize();

  /**
   * Deserializes the object from a JSON object
   *
   * @param data JSON to deserialize from
   */
  void deserialize(const nlohmann::json& data);

  /**
   * Animates sprite
   *
   * @param time Amount of time since last update
   */
  virtual void update(const sf::Time& time);

  /**
   * Renders sprite relative to cameraPos
   *
   * @param window Window to render to
   * @param cameraPos Position of camera to render sprite
   * relative to
   */
  virtual void render(sf::RenderTarget& window, sf::Vector2f cameraPos);
};

}  // namespace entities
