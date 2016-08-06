#include "tileset.h"

#include "asset_manager.h"
#include "util.h"

#include <iostream>
#include <string>

extern SDL_Renderer *renderer;

Tileset::Tileset(const std::string &basePath,
                 const nlohmann::json &tilesetData) {
  load(basePath, tilesetData);

  defaultTile_.walkable = false;
  defaultTile_.frame = 0;
}

bool Tileset::load(const std::string &basePath,
                   const nlohmann::json &tilesetData) {
  auto texturePath = tilesetData["image"].get<std::string>();
  tileWidth_ = tilesetData["tilewidth"].get<int>();
  tileHeight_ = tilesetData["tileheight"].get<int>();

  tileCount_ = tilesetData["tilecount"].get<int>();
  firstGid_ = tilesetData["firstgid"].get<int>();
  columns_ = tilesetData["columns"].get<int>();

  name_ = tilesetData["name"].get<std::string>();

  texture_.loadFromFile(basePath + "/" + texturePath);

  auto properties = tilesetData.find("tileproperties");
  auto animationData = tilesetData.find("tiles");
  if (properties != tilesetData.end()) {
    for (int i = 0; i < tileCount_; i++) {
      TileProperties t;
      auto res = (*properties).find(std::to_string(i));
      if (res == (*properties).end()) {
        t.ladder = false;
        t.walkable = true;
      } else {
        t.ladder = (*res)["ladder"].get<bool>();
        t.walkable = (*res)["walkable"].get<bool>();
      }

      t.frame = 0;

      if (animationData != tilesetData.end()) {
        auto iter = (*animationData).find(std::to_string(i));
        if (iter != (*animationData).end()) {
          auto animationList = (*iter).find("animation");
          if (animationList != (*iter).end()) {
            auto animation =
                (*animationList).get<std::vector<nlohmann::json>>();
            for (auto &obj : animation) {
              unsigned int tileId = obj["tileid"].get<unsigned int>();
              t.animationTiles.push_back(tileId);
            }
          }
        }
      }

      // If animation isn't found, default to an "animation" of one tile
      if (t.animationTiles.empty()) {
        t.animationTiles.push_back(i);
      }

      tiles_[i] = t;
    }
  }

  lastTicks_ = 0;

  return true;
}

unsigned int Tileset::removeFlags(unsigned int tile) {
  tile &= ~(FLIPPED_HORIZONTALLY | FLIPPED_VERTICALLY | FLIPPED_DIAGONALLY);
  return tile;
}

bool Tileset::contains(unsigned int tile) {
  tile = removeFlags(tile);
  return tile >= firstGid_ && tile < firstGid_ + tileCount_;
}

bool Tileset::walkable(unsigned int tile) {
  tile = removeFlags(tile);
  return tiles_[tile - firstGid_].walkable;
}

bool Tileset::ladder(unsigned int tile) {
  tile = removeFlags(tile);
  return tiles_[tile - firstGid_].ladder;
}

bool Tileset::update(unsigned int ticks) {
  if (ticks - lastTicks_ >= FRAME_TICKS_INTERVAL) {
    for (auto &tile : tiles_) {
      tile.second.frame =
          (tile.second.frame + 1) % tile.second.animationTiles.size();
      lastTicks_ = ticks;
    }
  }
  return true;
}

void Tileset::renderTile(sf::RenderTarget &window, unsigned int tile, int x,
                         int y) {
  if (tile == 0) {
    return;
  }

  bool flipHorizontal = tile & FLIPPED_HORIZONTALLY;
  bool flipVertical = tile & FLIPPED_VERTICALLY;
  bool flipDiagonal = tile & FLIPPED_DIAGONALLY;

  unsigned int angle = 0;
  if (flipDiagonal) {
    angle = 90;
    flipVertical = !(tile & FLIPPED_HORIZONTALLY);
    flipHorizontal = tile & FLIPPED_VERTICALLY;
  }

  tile &= ~(FLIPPED_HORIZONTALLY | FLIPPED_VERTICALLY | FLIPPED_DIAGONALLY);

  tile -= firstGid_;
  tile = tileFor(tile);

  sf::IntRect source(tileWidth_ * (tile % columns_),
                     tileHeight_ * (tile / columns_), tileWidth_, tileHeight_);

  if (flipHorizontal) {
    source.left += tileWidth_;
    source.width = -tileWidth_;
  }
  if (flipVertical) {
    source.top += tileHeight_;
    source.height = -tileHeight_;
  }

  tile_.setTextureRect(source);
  tile_.setRotation(angle);
  tile_.setPosition(x, y);

  window.draw(tile_);
}
