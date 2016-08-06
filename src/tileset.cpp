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

  dimensions_.w = tilesetData["imagewidth"].get<int>();
  dimensions_.h = tilesetData["imageheight"].get<int>();

  texture_ = AssetManager::getTexture(basePath + "/" + texturePath);
  if (texture_ == nullptr) {
    err()->error("\"{}\" does not exist, skipping render", texturePath);
    return false;
  }

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

void Tileset::renderTile(unsigned int tile, int x, int y) const {
  if (tile == 0) {
    return;
  }

  bool flipHorizontal = tile & FLIPPED_HORIZONTALLY;
  bool flipVertical = tile & FLIPPED_VERTICALLY;
  bool flipDiagonal = tile & FLIPPED_DIAGONALLY;

  unsigned int flags = 0;
  unsigned int angle = 0;
  if (flipDiagonal) {
    angle = 90;
    flipVertical = !(tile & FLIPPED_HORIZONTALLY);
    flipHorizontal = tile & FLIPPED_VERTICALLY;
  }
  if (flipHorizontal) {
    flags |= SDL_FLIP_HORIZONTAL;
  }
  if (flipVertical) {
    flags |= SDL_FLIP_VERTICAL;
  }

  tile &= ~(FLIPPED_HORIZONTALLY | FLIPPED_VERTICALLY | FLIPPED_DIAGONALLY);

  tile -= firstGid_;
  tile = tileFor(tile);

  SDL_Rect source;
  source.x = tileWidth_ * (tile % columns_);
  source.y = tileHeight_ * (tile / columns_);
  source.w = tileWidth_;
  source.h = tileHeight_;

  SDL_Rect dest;
  dest.x = x;
  dest.y = y;
  dest.w = tileWidth_;
  dest.h = tileHeight_;

  SDL_RenderCopyEx(renderer, texture_, &source, &dest, angle, NULL,
                   (SDL_RendererFlip)flags);
}
