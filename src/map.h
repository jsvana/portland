#pragma once

#include "tileset.h"
#include "util.h"

#include <json.hpp>

#include <SFML/Graphics.hpp>

#include <set>
#include <string>

/**
 * Class to load and contain a map layer
 */
class MapLayer {
 public:
  std::vector<std::vector<unsigned int>> tiles;

  MapLayer(const nlohmann::json &layerData);

  /**
   * Gets the tile at a given (x, y) coordinate
   *
   * @param x X coordinate of tile
   * @param y Y coordinate of tile
   * @return Tile number of tile at point
   */
  unsigned int tileAt(int x, int y) { return tiles[y][x]; }

  /**
   * Gets the tile at a given Point
   *
   * @param p Location of tile
   * @return Tile number of tile at point
   */
  unsigned int tileAt(Point p) { return tileAt(p.x, p.y); }
};

/**
 * Class to load, update, and render a tile map
 */
class Map {
 private:
  std::string path_;

  Point position_;

  // Map dimensions in tiles
  int mapWidth_, mapHeight_;

  // Map dimentions in pixels
  int mapPixelWidth_, mapPixelHeight_;

  // Tile dimensions in pixels
  int tileWidth_, tileHeight_;

  // Vector of layers of tile maps
  std::vector<MapLayer> layers_;

  // Vector of tilesets used in the map
  std::vector<std::shared_ptr<Tileset>> tilesets_;

  /**
   * Load a map from the given path
   *
   * @param path Path to map file
   * @return Whether operation was successful
   */
  bool load(const std::string &path);

  /**
   * Clamps a point to map dimensions
   *
   * @param p Point to clamp
   */
  void ensurePointInMap(Point &p);

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
  std::set<unsigned int> hitTiles(int x, int y, int w, int h);

  /**
   * Takes a rectangle in screen space and returns a list of tiles in
   * map space that the rectangle is touching
   *
   * @param rect Rectangle to check
   * @return Set of hit tiles
   */
  std::set<unsigned int> hitTiles(Rect rect) {
    return hitTiles(rect.x, rect.y, rect.w, rect.h);
  }

  /**
   * Gets the proper tileset for the given tile ID. Necessary because
   * tile IDs index into tilesets based on a global tile ID offset.
   *
   * @param tile Tile to get tileset for
   * @return Tileset for tile or nullptr if not found
   */
  std::shared_ptr<Tileset> tilesetForTile(unsigned int tile);

  /**
   * Checks if tile is walkable
   *
   * @param tile Tile to check
   * @return Whether tile is walkable
   */
  bool walkable(unsigned int tile);

  /**
   * Checks if tile is a ladder
   *
   * @param tile Tile to check
   * @return Whether tile is a ladder
   */
  bool ladder(unsigned int tile);

 public:
  Map(const std::string &path);

  /**
   * Sets map position to the given point
   *
   * @param x New x coordinate of map
   * @param y New y coordinate of map
   */
  void setPosition(int x, int y) {
    position_.x = x;
    position_.y = y;
  }

  /**
   * Sets map position to the given point
   *
   * @param pos New map position
   */
  void setPosition(Point pos) { setPosition(pos.x, pos.y); }

  /**
   * Finds next position below character that isn't walkable
   * and return the adjusted position accordingly
   *
   * @param dim Rectangle to test
   * @return Adjusted position
   */
  Rect snapRectToTileBelow(Rect dim);

  /**
   * Gets map position
   *
   * @return Map position
   */
  Point getPosition() { return position_; }

  /**
   * Converts a point in pixel space to tile space
   *
   * @param x X coordinate to map
   * @param y Y coordinate to map
   * @return Mapped point
   */
  Point pixelToMap(int x, int y);

  /**
   * Converts a point in pixel space to tile space
   *
   * @param p Point to map
   * @return Mapped point
   */
  Point pixelToMap(Point p) { return pixelToMap(p.x, p.y); }

  /**
   * Converts a point in tile space to pixel space
   *
   * @param x X coordinate to map
   * @param y Y coordinate to map
   * @return Mapped point
   */
  Point mapToPixel(int x, int y);

  /**
   * Converts a point in tile space to pixel space
   *
   * @param p Point to map
   * @return Mapped point
   */
  Point mapToPixel(Point p) { return mapToPixel(p.x, p.y); }

  /**
   * Calculates unique number for each (x, y) coordinate
   *
   * @param x X coordinate of point
   * @param y Y coordinate of point
   * @return Unique number for point
   */
  int mapPointToTileNumber(int x, int y) { return y * mapWidth_ + x; }

  /**
   * Calculates unique number for each (x, y) coordinate in
   * pixel space
   *
   * @param p Point to calculate number for
   * @return Unique number for point
   */
  int pointToTileNumber(Point p) {
    p = pixelToMap(p.x, p.y);
    return mapPointToTileNumber(p.x, p.y);
  }

  /**
   * Inverse of pointToTileNumber, given the unique tile number
   * calculate the pixel space point
   *
   * @param t Unique tile number to calculate point for
   */
  Point tileNumberToPoint(unsigned int t) {
    Point p(t % mapWidth_, t / mapWidth_);
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
  bool positionWalkable(int x, int y, int w, int h);

  /**
   * Determines whether or not a position as specified by a rectangle
   * is walkable
   *
   * @param rect Rectangle to check
   * @return Whether the rect is walkable
   */
  bool positionWalkable(Rect rect) {
    return positionWalkable(rect.x, rect.y, rect.w, rect.h);
  }

  /**
   * Determines whether or not a position as specified by a rectangle
   * is a ladder
   *
   * @param rect Rectangle to check
   * @return Whether the rect is a ladder
   */
  bool isLadder(Rect rect);

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
   * @param ticks Number of ticks since start
   */
  bool update(unsigned int ticks);

  /**
   * Renders map relative to the given camera position
   *
   * @param window Window to render to
   * @param cameraPos Position of camera to render map relative to
   */
  void render(sf::RenderTarget &window, Point cameraPos);
};
