#pragma once

#include "constants.h"
#include "entities/item.h"
#include "entities/npc.h"
#include "entities/sprite.h"
#include "map.h"
#include "visual/dialog.h"

#include <chaiscript/chaiscript.hpp>
#include <chaiscript/chaiscript_stdlib.hpp>

#include <iterator>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

namespace GameState {

typedef std::function<void()> TileCallback;
typedef std::function<void(bool)> FlagChangeCallback;

const int GRAVITY = 2;

const int STARTING_JUMP_VELOCITY = -15;

/**
 * Initializes API
 */
void initApi();

/**
 * Updates the camera bounds on map change
 */
void updateCamera();

/**
 * Gets game camera
 */
sf::Vector2f& camera();

/**
 * Finds a sprite that collides with the given rectangle
 *
 * @param collisionRect Rectangle to check
 * @return Sprite colliding or nullptr
 */
entities::Sprite* spriteCollision(const sf::FloatRect& collisionRect);

/**
 * Finds position of the next sprite above the sprite
 *
 * @param sprite Sprite to check
 * @return Position of next sprite
 */
float positionOfSpriteAbove(const std::unique_ptr<entities::Sprite>& sprite);

/**
 * Finds position of the next dense object above the sprite
 *
 * @param sprite Sprite to check
 * @return Position of next dense object
 */
float densePositionAbove(const std::unique_ptr<entities::Sprite>& sprite);

/**
 * Finds position of the next sprite below the sprite
 *
 * @param sprite Sprite to check
 * @return Position of next sprite
 */
float positionOfSpriteBelow(const std::unique_ptr<entities::Sprite>& sprite);

/**
 * Finds position of the next dense object below the sprite
 *
 * @param sprite Sprite to check
 * @return Position of next dense object
 */
float densePositionBelow(const std::unique_ptr<entities::Sprite>& sprite);

/**
 * API wrapper from mod
 *
 * @param a Number to mod
 * @param b Number to mod by
 * @return mod(a, b)
 */
int mod(int a, int b);

/**
 * Sets the current game ticks
 *
 * @param ticks Current game ticks
 */
void setTicks(util::Tick ticks);

/**
 * Gets the current game ticks
 *
 * @return Current game ticks
 */
int ticks();

/**
 * Gets the main character
 *
 * @return Main character
 */
const std::unique_ptr<entities::Sprite>& hero();

/**
 * API wrapper for hero()
 *
 * @return Pointer to hero
 */
entities::Sprite* getHero();

/**
 * Gets the hero's move speed
 *
 * @return Hero's move speed
 */
int heroMoveSpeed();

/**
 * Increments the internal sprite ID counter and returns the previous
 * value
 *
 * @return Next sprite ID
 */
entities::Id allocateSpriteId();

/**
 * Gets a reference to the topmost sprite map
 *
 * @return Reference to topmost sprite map
 */
std::vector<std::unique_ptr<entities::Sprite>>& sprites();

/**
 * Gets the topmost map
 *
 * @return Topmost map
 */
const std::unique_ptr<map::Map>& map();

/**
 * Gets a reference to the ChaiScript state
 *
 * @return Reference to ChaiScript state
 */
chaiscript::ChaiScript& chai();

/**
 * Adds a callback to a specific tile
 *
 * @param id Tile ID to add callback to
 * @param callback Name of ChaiScript callback function
 */
void addTileEvent(int id, TileCallback callback);

/**
 * Checks if given tile ID has an event
 *
 * @return Whether tile has event
 */
bool tileHasEvent(int id);

/**
 * Gets ChaiScript callback function name for tile ID
 *
 * @param id Tile ID to get callback function for
 * @return ChaiScript callback function name
 */
const TileCallback& tileCallback(int id);

/**
 * Clears an event for a specific tile
 *
 * @param id Tile ID to clear event on
 */
void clearTileEvent(int id);

/**
 * Clears all tile events
 */
void clearTileEvents();

/**
 * Checks if a position is walkable by a given entity ID
 *
 * @param sprite Moving entity
 * @param dim Rectangle to check
 * @return Whether position is walkable
 */
bool positionWalkable(const std::unique_ptr<entities::Sprite>& sprite,
                      sf::FloatRect dim);

/**
 * Checks if a position is walkable by a given entity ID
 *
 * @param sprite Moving entity
 * @param dim Rectangle to check
 * @return Whether position is walkable
 */
bool positionWalkable(entities::Sprite* sprite, sf::FloatRect dim);

/**
 * Attempts to run collision callback on `mover` against `other`
 *
 * @param mover Sprite to run callback on
 * @param other Sprite to run callback against
 */
void dispatchCollision(entities::Sprite* mover, entities::Sprite* other);

/**
 * Checks for an event on the character's current tile, runs the event, and
 * clears it
 */
void runTileEvent();

/**
 * Marks the game as initialized
 */
void markInitialized();

/**
 * Gets whether or not game has been initialized
 *
 * @return Whether or not game has been initialized
 */
bool initialized();

/**
 * Loads a new map and pushes it onto the stack
 *
 * @param path Path of new map to load
 * @return Whether the operation is successful
 */
bool loadMap(std::string path);

/**
 * Pops the current map from the stack
 *
 * @return Whether the operation is successful
 */
bool popMap();

/**
 * Loads a character sprite and sets its tile index and position
 *
 * @param path Path to sprite definition
 * @param tile Tile index of sprite
 * @param initX Initial x position
 * @param initY Initial y position
 * @return Whether the operation is successful
 */
bool loadCharacter(std::string path, int tile, int initX, int initY);

/**
 * Sets the character move speed
 *
 * @param speed New move speed
 * @return Whether the operation is successful
 */
bool setCharacterMoveSpeed(int amount, int total);

/**
 * Adds a new item and sets its tile index and position
 *
 * @param path Path to item definition
 * @param tile Tile index of item
 * @param initX Initial x position
 * @param initY Initial y position
 * @return Item ID
 */
entities::Id addItem(const std::string& path, int tile, int x, int y);

/**
 * Adds a new NPC and sets its tile index and position
 *
 * @param path Path to NPC definition
 * @param tile Tile index of NPC
 * @param initX Initial x position
 * @param initY Initial y position
 * @return NPC ID
 */
entities::Id addNpc(const std::string& path, int tile, int x, int y);

/**
 * Finds a sprite with the given ID or returns nullptr
 *
 * @template T Type of sprite to cast to
 * @param spriteId ID of sprite to find
 * @return Found sprite or nullptr
 */
template <typename T>
T* findSprite(entities::Id spriteId);

/**
 * API wrapper to get a sprite
 *
 * @param spriteId ID of sprite to get
 * @return Found sprite or nullptr
 */
entities::Sprite* getSprite(const entities::Id spriteId);

/**
 * API wrapper to get an NPC
 *
 * @param npcId ID of NPC to get
 * @return Found NPC or nullptr
 */
entities::Npc* getNpc(const entities::Id npcId);

/**
 * API wrapper to get an item
 *
 * @param npcId ID of item to get
 * @return Found item or nullptr
 */
entities::Item* getItem(const entities::Id itemId);

/**
 * Finds a sprite with the given ID or returns nullptr
 *
 * @template T Type of sprite to cast to
 * @param spriteId ID of sprite to find
 * @param type Type of sprite to find
 * @return Found sprite or nullptr
 */
template <typename T>
T* findSprite(entities::Id spriteId, const entities::SpriteType type);

/**
 * Sets an NPC callback function
 *
 * @param npcId ID of NPC to set callback for
 * @param callback ChaiScript callback function
 * @return Whether the operation is successful
 */
bool setNpcCallback(entities::Id npcId, entities::SpriteCallback callback);

/**
 * Queues a dialog to be shown
 *
 * @param message Message to show in dialog
 * @return Dialog ID
 */
visual::DialogManager::Id showDialog(std::string message);

/**
 * Adds an option to the given dialog
 *
 * @param option Option to add to dialog
 * @return Whether the operation is successful
 */
bool addDialogOption(visual::DialogManager::Id uid, std::string option);

/**
 * Sets the ChaiScript callback for the given dialog
 *
 * @param uid ID of dialog to set callback for
 * @param callback ChaiScript callback function
 * @return Whether the operation is successful
 */
bool setDialogCallback(visual::DialogManager::Id uid,
                       visual::DialogCallback callback);

/**
 * Adds a tile event to a tile to run when the character hits the
 * tile
 *
 * @param x X coordinate of tile
 * @param y Y coordinate of tile
 * @param callback Name of ChaiScript callback function to run on contact
 * @return Whether the operation is successful
 */
bool registerTileEvent(int x, int y, TileCallback callback);

/**
 * Clears all registered tile events
 *
 * @return Whether the operation is successful
 */
bool clearEvents();

/**
 * Set a global boolean game flag
 *
 * @param flag Flag to set
 * @param value Value of flag
 */
void setFlag(const std::string& flag, bool value);

/**
 * Get a global boolean game flag
 *
 * @param flag Flag to get
 * @return Value of flag or false if not set
 */
bool getFlag(const std::string& flag);

const std::list<FlagChangeCallback> flagChangeCallbacks(
    const std::string& flag);

void addFlagChangeCallback(const std::string& flag,
                           const FlagChangeCallback& func);

}  // namespace GameState
