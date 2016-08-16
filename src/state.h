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

  const int GRAVITY = 2;

  const int STARTING_JUMP_VELOCITY = -15;

  /**
   * Initializes Lua API
   */
  void initLuaApi();

  /**
   * Updates the camera bounds on map change
   */
  void updateCamera();

  /**
   * Gets game camera
   */
  sf::Vector2f &camera();

  /**
   * Finds position of the next sprite above the sprite
   *
   * @param sprite Sprite to check
   * @return Position of next sprite
   */
  float positionOfSpriteAbove(const std::unique_ptr<Sprite> &sprite);

  /**
   * Finds position of the next dense object above the sprite
   *
   * @param sprite Sprite to check
   * @return Position of next dense object
   */
  float densePositionAbove(const std::unique_ptr<Sprite> &sprite);

  /**
   * Finds position of the next sprite below the sprite
   *
   * @param sprite Sprite to check
   * @return Position of next sprite
   */
  float positionOfSpriteBelow(const std::unique_ptr<Sprite> &sprite);

  /**
   * Finds position of the next dense object below the sprite
   *
   * @param sprite Sprite to check
   * @return Position of next dense object
   */
  float densePositionBelow(const std::unique_ptr<Sprite> &sprite);

  /**
   * Sets the current game ticks
   *
   * @param ticks Current game ticks
   */
  void setTicks(unsigned int ticks);

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
  const std::unique_ptr<Sprite> &hero();

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
  unsigned int allocateSpriteId();

  /**
   * Gets a reference to the topmost sprite map
   *
   * @return Reference to topmost sprite map
   */
  std::vector<std::unique_ptr<Sprite>> &sprites();

  /**
   * Gets the topmost map
   *
   * @return Topmost map
   */
  const std::unique_ptr<Map> &map();

  /**
   * Gets a reference to the ChaiScript state
   *
   * @return Reference to ChaiScript state
   */
  chaiscript::ChaiScript &chai();

  /**
   * Adds a callback to a specific tile
   *
   * @param id Tile ID to add callback to
   * @param callback Name of Lua callback function
   */
  void addTileEvent(int id, std::string callback);

  /**
   * Checks if given tile ID has an event
   *
   * @return Whether tile has event
   */
  bool tileHasEvent(int id);

  /**
   * Gets Lua callback function name for tile ID
   *
   * @param id Tile ID to get callback function for
   * @return Lua callback function name
   */
  std::string tileCallback(int id);

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
  bool positionWalkable(const std::unique_ptr<Sprite> &sprite,
                        sf::FloatRect dim);

  /**
   * Checks if a position is walkable by a given entity ID
   *
   * @param sprite Moving entity
   * @param dim Rectangle to check
   * @return Whether position is walkable
   */
  bool positionWalkable(Sprite *sprite, sf::FloatRect dim);

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
   * Sets the character position
   *
   * @param x New x position
   * @param y New y position
   * @return Whether the operation is successful
   */
  bool setCharacterPosition(int x, int y);

  /**
   * Sets the character move speed
   *
   * @param speed New move speed
   * @return Whether the operation is successful
   */
  bool setCharacterMoveSpeed(int amount, int total);

  /**
   * Sets the character's max HP
   *
   * @param hp New max HP
   * @return Whether the operation is successful
   */
  bool setCharacterMaxHp(int hp);

  /**
   * Lowers the character's HP
   *
   * @param amount Amount to lower HP by
   * @return Whether the operation is successful
   */
  bool damageCharacter(int amount);

  /**
   * Raises the character's HP
   *
   * @param amount Amount to raise HP by
   * @return Whether the operation is successful
   */
  bool healCharacter(int amount);

  /**
   * Adds a new item and sets its tile index and position
   *
   * @param path Path to item definition
   * @param tile Tile index of item
   * @param initX Initial x position
   * @param initY Initial y position
   * @return Item ID
   */
  unsigned int addItem(std::string path, int tile, int x, int y);

  /**
   * Adds a new NPC and sets its tile index and position
   *
   * @param path Path to NPC definition
   * @param tile Tile index of NPC
   * @param initX Initial x position
   * @param initY Initial y position
   * @return NPC ID
   */
  unsigned int addNpc(std::string path, int tile, int x, int y);

  /**
   * Finds a sprite with the given ID or returns nullptr
   *
   * @template T Type of sprite to cast to
   * @param spriteId ID of sprite to find
   */
  template <typename T>
  T *findSprite(unsigned int spriteId);

  /**
   * Sets an NPC callback function
   *
   * @param npcId ID of NPC to set callback for
   * @param callback Name of Lua callback function
   * @return Whether the operation is successful
   */
  bool setNpcCallback(unsigned int npcId, std::string callback);

  /**
   * Moves an NPC
   *
   * @param npcId ID of NPC to move
   * @param dx Distance to move NPC x coordinate
   * @param dy Distance to move NPC y coordinate
   * @return Whether the operation is successful
   */
  bool moveNpc(unsigned int npcId, int x, int y);

  /**
   * Sets an NPC's max HP
   *
   * @param npcId ID of NPC to set max HP for
   * @param hp New max HP
   * @return Whether the operation is successful
   */
  bool setNpcMaxHp(unsigned int npcId, int hp);

  /**
   * Lowers an NPC's HP
   *
   * @param npcId ID of NPC to damage
   * @param amount Amount to lower HP by
   * @return Whether the operation is successful
   */
  bool damageNpc(unsigned int npcId, int amount);

  /**
   * Raises an NPC's HP
   *
   * @param npcId ID of NPC to heal
   * @param amount Amount to raise HP by
   * @return Whether the operation is successful
   */
  bool healNpc(unsigned int npcId, int amount);

  /**
   * Makes an NPC jump
   *
   * @param npcId ID of NPC to jump
   * @param magnitude Magnitude of the jump, from 0 to 100
   * @return Whether the operation is successful
   */
  bool jumpNpc(unsigned int npcId, int magnitude);

  /**
   * Queues a dialog to be shown
   *
   * @param message Message to show in dialog
   * @return Dialog ID
   */
  unsigned int showDialog(std::string message);

  /**
   * Adds an option to the given dialog
   *
   * @param option Option to add to dialog
   * @return Whether the operation is successful
   */
  bool addDialogOption(unsigned int uid, std::string option);

  /**
   * Sets the Lua callback for the given dialog
   *
   * @param uid ID of dialog to set callback for
   * @param callback Name of Lua callback function
   * @return Whether the operation is successful
   */
  bool setDialogCallback(unsigned int uid, std::string callback);

  /**
   * Adds a tile event to a tile to run when the character hits the
   * tile
   *
   * @param x X coordinate of tile
   * @param y Y coordinate of tile
   * @param callback Name of Lua callback function to run on contact
   * @return Whether the operation is successful
   */
  bool registerTileEvent(int x, int y, std::string callback);

  /**
   * Clears all registered tile events
   *
   * @return Whether the operation is successful
   */
  bool clearEvents();

  ///////////////////////
  // LUA API Callbacks //
  ///////////////////////

  void collision(unsigned int spriteId1, unsigned int spriteId2);
}
