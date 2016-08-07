#include "map.h"

#include "util.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

Map::Map(const std::string &path) : path_(path) { load(path); }

bool Map::load(const std::string &path) {
  std::ifstream mapfile(path, std::ios::in);
  if (!mapfile.is_open()) {
    err()->error("Unable to open mapfile \"{}\"", path);
    return false;
  }

  std::stringstream fileData;
  fileData << mapfile.rdbuf();
  mapfile.close();

  auto mapBasePath = path.substr(0, path.find_last_of("/"));
  auto mapData = nlohmann::json::parse(fileData.str());

  auto tilesets = mapData["tilesets"].get<std::vector<nlohmann::json>>();
  for (auto &tileset : tilesets) {
    tilesets_.push_back(std::make_shared<Tileset>(mapBasePath, tileset));
  }

  mapWidth_ = mapData["width"].get<int>();
  mapHeight_ = mapData["height"].get<int>();

  tileWidth_ = mapData["tilewidth"].get<int>();
  tileHeight_ = mapData["tileheight"].get<int>();
  mapPixelWidth_ = mapWidth_ * tileWidth_;
  mapPixelHeight_ = mapHeight_ * tileHeight_;

  auto layers = mapData["layers"].get<std::vector<nlohmann::json>>();
  for (auto &layer : layers) {
    layers_.push_back(MapLayer(layer));
  }

  return true;
}

void Map::ensurePointInMap(sf::Vector2f &p) {
  if (p.x < 0) {
    p.x = 0;
  } else if (p.x >= mapWidth_) {
    p.x = mapWidth_ - 1;
  }
  if (p.y < 0) {
    p.y = 0;
  } else if (p.y >= mapHeight_) {
    p.y = mapHeight_ - 1;
  }
}

sf::Vector2f Map::mapToPixel(int x, int y) {
  sf::Vector2f pixelPosition(x, y);
  ensurePointInMap(pixelPosition);
  pixelPosition.x *= tileWidth_;
  pixelPosition.y *= tileHeight_;
  return pixelPosition;
}

sf::Vector2f Map::pixelToMap(int x, int y) {
  sf::Vector2f mapPosition(x, y);
  mapPosition.x = (int)(mapPosition.x / tileWidth_);
  mapPosition.y = (int)(mapPosition.y / tileHeight_);
  ensurePointInMap(mapPosition);
  return mapPosition;
}

std::set<unsigned int> Map::hitTiles(int x, int y, int w, int h) {
  auto topLeft = pixelToMap(x, y);
  auto bottomRight = pixelToMap(x + w - 1, y + h - 1);

  // Only add the top nonzero tile
  std::set<unsigned int> tiles;
  for (int i = topLeft.y; i <= bottomRight.y; i++) {
    for (int j = topLeft.x; j <= bottomRight.x; j++) {
      for (int k = (int)layers_.size() - 1; k >= 0; k--) {
        unsigned int tile = layers_[k].tileAt(j, i);
        if (tile != 0) {
          tiles.insert(tile);
          break;
        }
      }
    }
  }
  return tiles;
}

float Map::positionOfTileAbove(sf::FloatRect dim) {
  auto topLeft = pixelToMap(dim.left, dim.top);
  auto topRight = pixelToMap(dim.left + dim.width - 1, dim.top);

  for (int i = topLeft.y; i >= 0; i--) {
    for (int j = topLeft.x; j <= topRight.x; j++) {
      for (int k = (int)layers_.size() - 1; k >= 0; k--) {
        unsigned int tile = layers_[k].tileAt(j, i);
        if (tile == 0 || walkable(tile)) {
          continue;
        }
        auto newPos = mapToPixel(j, i);
        return newPos.y + tileHeight_;
      }
    }
  }

  return 0;
}

float Map::positionOfTileBelow(sf::FloatRect dim) {
  auto bottomLeft = pixelToMap(dim.left, dim.top + dim.height - 1);
  auto bottomRight =
      pixelToMap(dim.left + dim.width - 1, dim.top + dim.height - 1);

  for (int i = bottomLeft.y; i < mapHeight_; i++) {
    for (int j = bottomLeft.x; j <= bottomRight.x; j++) {
      for (int k = (int)layers_.size() - 1; k >= 0; k--) {
        unsigned int tile = layers_[k].tileAt(j, i);
        if (tile == 0 || walkable(tile)) {
          continue;
        }
        auto newPos = mapToPixel(j, i);
        return newPos.y - dim.height;
      }
    }
  }

  return std::numeric_limits<float>::max();
}

bool Map::isLadder(sf::FloatRect dim) {
  auto pos = pixelToMap(dim.left, dim.top);
  for (int i = (int)layers_.size() - 1; i >= 0; i--) {
    unsigned int tile = layers_[i].tileAt(pos.x, pos.y);
    if (tile != 0 && ladder(tile)) {
      return true;
    }
  }
  return false;
}

bool Map::positionWalkable(int x, int y, int w, int h) {
  // Default bounds detection
  if (x < 0 || y < 0 || x + w > mapPixelWidth_ || y + h > mapPixelHeight_) {
    return false;
  }

  std::set<unsigned int> tiles = hitTiles(x, y, w, h);
  for (unsigned int tile : tiles) {
    if (tile == 0) {
      continue;
    }
    if (!walkable(tile)) {
      return false;
    }
  }
  return true;
}

std::shared_ptr<Tileset> Map::tilesetForTile(unsigned int tile) {
  for (auto &tileset : tilesets_) {
    if (tileset->contains(tile)) {
      return tileset;
    }
  }
  err()->warn("Could not find tileset for {}", tile);
  return nullptr;
}

bool Map::ladder(unsigned int tile) {
  auto tileset = tilesetForTile(tile);
  if (!tileset) {
    return false;
  }
  return tileset->ladder(tile);
}

bool Map::walkable(unsigned int tile) {
  auto tileset = tilesetForTile(tile);
  if (!tileset) {
    return false;
  }
  return tileset->walkable(tile);
}

bool Map::update(sf::Time &time) {
  for (auto &tileset : tilesets_) {
    tileset->update(time);
  }

  return true;
}

void Map::render(sf::RenderTarget &window, sf::Vector2f cameraPos) {
  int xStart, xEnd;
  int yStart, yEnd;
  xStart = 0;
  xEnd = mapWidth_;

  yStart = 0;
  yEnd = mapHeight_;

  int x = position_.x - cameraPos.x;
  int y = position_.y - cameraPos.y;

  for (auto &layer : layers_) {
    for (int i = yStart; i < yEnd; i++) {
      for (int j = xStart; j < xEnd; j++) {
        unsigned int tile = layer.tiles[i][j];
        if (tile == 0) {
          continue;
        }
        auto tileset = tilesetForTile(tile);
        if (!tileset) {
          continue;
        }
        tileset->renderTile(window, tile, x + j * tileset->width(),
                            y + i * tileset->height());
      }
    }
  }
}

MapLayer::MapLayer(const nlohmann::json &layerData) {
  auto width = layerData["width"].get<int>();
  auto height = layerData["height"].get<int>();
  auto data = layerData["data"].get<std::vector<int>>();
  auto data_iter = data.begin();
  for (int i = 0; i < height; i++) {
    std::vector<unsigned int> row;
    for (int j = 0; j < width; j++) {
      row.push_back(*data_iter);
      data_iter++;
    }
    tiles.push_back(row);
  }
}
