#pragma once

#include <json.hpp>

#include <SFML/Graphics.hpp>

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * Container for all the various tile properties
 */
struct TileProperties {
  bool ladder;
  bool walkable;

  std::vector<unsigned int> animationTiles;
  int frame;
};

/**
 * Class to load, query, and render tiles in a tileset
 */
class Tileset {
 private:
  // Constants set by Tiled to represent reflections and rotations
  const unsigned int FLIPPED_HORIZONTALLY = 0x80000000;
  const unsigned int FLIPPED_VERTICALLY = 0x40000000;
  const unsigned int FLIPPED_DIAGONALLY = 0x20000000;

  // Update interval for the tileset
  const unsigned int FRAME_TICKS_INTERVAL = 24;

  // Updated and returned when a tile is not found
  TileProperties defaultTile_;

  int tileWidth_;
  int tileHeight_;
  int tileCount_;
  int columns_;
  unsigned int lastTicks_;

  // Number representing the first tile number in the tileset
  // (used to find tileset ownership for tiles)
  unsigned int firstGid_;

  std::string name_;

  // Map of tile ID to tile properties
  std::unordered_map<unsigned int, TileProperties> tiles_;

  sf::Texture texture_;
  sf::Sprite tile_;

  /**
   * Loads a tileset from the passed JSON object
   *
   * @param basePath Path the JSON was loaded from. Used to resolve relative
   * paths.
   * @param tilesetData JSON object to load tileset from
   * @return Whether the operation was successful
   */
  bool load(const std::string &basePath, const nlohmann::json &tilesetData);

  /**
   * Removes rotation and reflection flags on a tile
   *
   * @param tile Tile to remove flags from
   * @return Tile without flags
   */
  unsigned int removeFlags(unsigned int tile);

 public:
  Tileset(const std::string &basePath, const nlohmann::json &tilesetData);

  /**
   * Gets TileProperties for the given tile index or the default tile if
   * the tile is not found
   *
   * @param t Tile to find properties for
   * @return Found TileProperties
   */
  const TileProperties &tile(unsigned int t) const {
    auto res = tiles_.find(t);
    if (res == tiles_.end()) {
      return defaultTile_;
    }
    return res->second;
  }

  /**
   * Resolves the current animation tile to render for a given tile index
   *
   * @param tileIdx Tile to find animation for
   * @return Current tile index for animation
   */
  unsigned int tileFor(unsigned int tileIdx) const {
    auto t = tile(tileIdx);
    if (t.animationTiles.empty()) {
      return tileIdx;
    }
    return t.animationTiles[t.frame];
  }

  /**
   * Gets the width of the tileset's tiles
   *
   * @param Width of the tileset's tiles
   */
  int width() { return tileWidth_; }

  /**
   * Gets the height of the tileset's tiles
   *
   * @param Height of the tileset's tiles
   */
  int height() { return tileHeight_; }

  /**
   * Checks if the tileset contains the given tile
   *
   * @param tile Tile to check
   * @return Whether the tileset contains the tile
   */
  bool contains(unsigned int tile);

  /**
   * Checks if the given tile is walkable
   *
   * @param tile Tile to check
   * @return Whether the tile is walkable
   */
  bool walkable(unsigned int tile);

  /**
   * Checks if the given tile is a ladder
   *
   * @param tile Tile to check
   * @return Whether the tile is a ladder
   */
  bool ladder(unsigned int tile);

  /**
   * Animate tiles in the tileset
   *
   * @param ticks Number of ticks from start
   * @return Always returns true
   */
  bool update(unsigned int ticks);

  /**
   * Renders the given tile at the specified point
   *
   * @param window Window to render to
   * @param tile Tile index to render
   * @param x X coordinate of render point
   * @param y Y coordinate of render point
   */
  void renderTile(sf::RenderTarget &window, unsigned int tile, int x, int y);
};
