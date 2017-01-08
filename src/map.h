#pragma once

#include "tileset.h"
#include "util.h"

#include <json.hpp>

#include <SFML/Graphics.hpp>

#include <set>
#include <string>

namespace map {

/**
 * Class to load and contain a map layer
 */
class MapLayer {
 public:
  std::vector<std::vector<TileId>> tiles;

  MapLayer(const nlohmann::json& layerData);

  /**
   * Gets the tile at a given (x, y) coordinate
   *
   * @param x X coordinate of tile
   * @param y Y coordinate of tile
   * @return Tile number of tile at point
   */
  TileId tileAt(const int x, const int y) { return tiles[y][x]; }

  /**
   * Gets the tile at a given sf::Vector2f
   *
   * @param p Location of tile
   * @return Tile number of tile at point
   */
  TileId tileAt(const sf::Vector2f p) { return tileAt((int)p.x, (int)p.y); }
};

/**
 * Class to load, update, and render a tile map
 */
class Map {
 private:
  std::string path_;

  sf::Vector2f position_;

  // Map dimensions in tiles
  int mapWidth_, mapHeight_;

  // Map dimentions in pixels
  int mapPixelWidth_, mapPixelHeight_;

  // Tile dimensions in pixels
  int tileWidth_, tileHeight_;

  // Vector of layers of tile maps
  std::vector<MapLayer> layers_;

  // Vector of tilesets used in the map
  std::vector<std::unique_ptr<Tileset>> tilesets_;

  /**
   * Load a map from the given path
   *
   * @param path Path to map file
   * @return Whether operation was successful
   */
  bool load(const std::string& path);

  /**
   * Clamps a point to map dimensions
   *
   * @param p sf::Vector2f to clamp
   */
  void ensurePointInMap(sf::Vector2f& p);

  /**
   * Takes a rectangle in screen space and returns a list of tiles in
   * map space that the rectangle is touching
   *
   * @param x X coordinate of rectangle
   * @param y Y coordinate of rectangle
   * @param w Width of rectangle
   * @param h Height of rectangle
   * @return Set of hit tiles
   */
  std::set<TileId> hitTiles(const float x, const float y, const float w,
                            const float h);

  /**
   * Takes a rectangle in screen space and returns a list of tiles in
   * map space that the rectangle is touching
   *
   * @param rect Rectangle to check
   * @return Set of hit tiles
   */
  std::set<TileId> hitTiles(const sf::FloatRect rect) {
    return hitTiles(rect.left, rect.top, rect.width, rect.height);
  }

  /**
   * Gets the proper tileset for the given tile ID. Necessary because
   * tile IDs index into tilesets based on a global tile ID offset.
   *
   * @param tile Tile to get tileset for
   * @return Tileset for tile or nullptr if not found
   */
  Tileset* tilesetForTile(const TileId tile);

  /**
   * Checks if tile is walkable
   *
   * @param tile Tile to check
   * @return Whether tile is walkable
   */
  bool walkable(const TileId tile);

 public:
  Map(const std::string& path);

  /**
   * Sets map position to the given point
   *
   * @param x New x coordinate of map
   * @param y New y coordinate of map
   */
  void setPosition(const float x, const float y) {
    position_.x = x;
    position_.y = y;
  }

  /**
   * Sets map position to the given point
   *
   * @param pos New map position
   */
  void setPosition(const sf::Vector2f pos) { setPosition(pos.x, pos.y); }

  /**
   * Finds position of the next not walkable tile above the rect
   *
   * @param dim Rectangle to test
   * @return Position of next not walkable tile
   */
  float positionOfTileAbove(const sf::FloatRect dim);

  /**
   * Finds position of the next not walkable tile below the rect
   *
   * @param dim Rectangle to test
   * @return Position of next not walkable tile
   */
  float positionOfTileBelow(const sf::FloatRect dim);

  /**
   * Gets map position
   *
   * @return Map position
   */
  sf::Vector2f getPosition() { return position_; }

  /**
   * Converts a point in pixel space to tile space
   *
   * @param x X coordinate to map
   * @param y Y coordinate to map
   * @return Mapped point
   */
  sf::Vector2f pixelToMap(const float x, const float y);

  /**
   * Converts a point in pixel space to tile space
   *
   * @param p Point to map
   * @return Mapped point
   */
  sf::Vector2f pixelToMap(const sf::Vector2f p) { return pixelToMap(p.x, p.y); }

  /**
   * Converts a point in tile space to pixel space
   *
   * @param x X coordinate to map
   * @param y Y coordinate to map
   * @return Mapped point
   */
  sf::Vector2f mapToPixel(const float x, const float y);

  /**
   * Converts a point in tile space to pixel space
   *
   * @param p Point to map
   * @return Mapped point
   */
  sf::Vector2f mapToPixel(const sf::Vector2f p) { return mapToPixel(p.x, p.y); }

  /**
   * Calculates unique number for each (x, y) coordinate
   *
   * @param x X coordinate of point
   * @param y Y coordinate of point
   * @return Unique number for point
   */
  int mapPointToTileNumber(const int x, const int y) {
    return y * mapWidth_ + x;
  }

  /**
   * Calculates unique number for each (x, y) coordinate in
   * pixel space
   *
   * @param p Point to calculate number for
   * @return Unique number for point
   */
  int pointToTileNumber(const sf::Vector2f p) {
    auto newPoint = p;
    newPoint = pixelToMap(p.x, p.y);
    if ((int)p.x % tileWidth_ >= tileWidth_ / 2) {
      ++newPoint.x;
    }
    if ((int)p.y % tileHeight_ >= tileHeight_ / 2) {
      ++newPoint.y;
    }
    return mapPointToTileNumber((int)newPoint.x, (int)newPoint.y);
  }

  /**
   * Inverse of pointToTileNumber, given the unique tile number
   * calculate the pixel space point
   *
   * @param t Unique tile number to calculate point for
   */
  sf::Vector2f tileNumberToPoint(const TileId t) {
    sf::Vector2f p((float)(t % mapWidth_), (float)t / mapWidth_);
    return mapToPixel(p);
  }

  /**
   * Determines whether or not a position as specified by a point and
   * dimensions is walkable
   *
   * @param x X coordinate of rectangle to check
   * @param y Y coordinate of rectangle to check
   * @param w Width of rectangle to check
   * @param h Height of rectangle to check
   * @return Whether the rect is walkable
   */
  bool positionWalkable(const float x, const float y, const float w,
                        const float h);

  /**
   * Determines whether or not a position as specified by a rectangle
   * is walkable
   *
   * @param rect Rectangle to check
   * @return Whether the rect is walkable
   */
  bool positionWalkable(const sf::FloatRect rect) {
    return positionWalkable(rect.left, rect.top, rect.width, rect.height);
  }

  /**
   * Gets width of map in pixels
   *
   * @return Width of map in pixels
   */
  int pixelWidth() { return mapPixelWidth_; }

  /**
   * Gets height of map in pixels
   *
   * @return Height of map in pixels
   */
  int pixelHeight() { return mapPixelHeight_; }

  /**
   * Gets width of individual tiles in pixels
   *
   * @return Width of a tile in pixels
   */
  int tileWidth() { return tileWidth_; }

  /**
   * Gets height of individual tiles in pixels
   *
   * @return Height of a tile in pixels
   */
  int tileHeight() { return tileHeight_; }

  /**
   * Updates each tileset
   *
   * @param time Time since last update
   */
  bool update(const sf::Time& time);

  /**
   * Renders map relative to the given camera position
   *
   * @param window Window to render to
   * @param cameraPos Position of camera to render map relative to
   */
  void render(sf::RenderTarget& window, const sf::Vector2f cameraPos);
};

}  // namespace map
