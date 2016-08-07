#include "state.h"

#include <limits.h>

namespace GameState {

  bool initialized_ = false;

  sf::Vector2f camera_;
  sf::FloatRect cameraBounds_;

  unsigned int ticks_ = 0;

  std::unique_ptr<Sprite> hero_;
  float moveSpeed_;

  // Jumping stuff
  bool jumping_ = false;
  int velocityY_ = 0;

  // Stack is used to mimick maps_
  unsigned int nextSpriteId_ = 1;
  std::stack<std::unordered_map<unsigned int, std::unique_ptr<Sprite>>>
      sprites_;

  std::stack<std::unique_ptr<Map>> maps_;

  std::unordered_map<int, std::string> tileEvents_;

  // Contains all scripting state
  sel::State lua_{true};

  void initLuaApi() {
    lua_["gameInitialized"] = &GameState::initialized;
    lua_["gameTicks"] = &GameState::ticks;

    lua_["gameLoadMap"] = &GameState::loadMap;
    lua_["gamePopMap"] = &GameState::popMap;
    lua_["gameLoadCharacter"] = &GameState::loadCharacter;
    lua_["gameSetCharacterMoveSpeed"] = &GameState::setCharacterMoveSpeed;
    lua_["gameSetCharacterMaxHp"] = &GameState::setCharacterMaxHp;
    lua_["gameDamageCharacter"] = &GameState::damageCharacter;
    lua_["gameHealCharacter"] = &GameState::healCharacter;

    lua_["gameAddNpc"] = &GameState::addNpc;
    lua_["gameSetNpcCallback"] = &GameState::setNpcCallback;
    lua_["gameMoveNpc"] = &GameState::moveNpc;
    lua_["gameSetNpcMaxHp"] = &GameState::setNpcMaxHp;
    lua_["gameDamageNpc"] = &GameState::damageNpc;
    lua_["gameHealNpc"] = &GameState::healNpc;
    lua_["gameJumpNpc"] = &GameState::jumpNpc;

    lua_["gameShowDialog"] = &GameState::showDialog;
    lua_["gameAddDialogOption"] = &GameState::addDialogOption;
    lua_["gameSetDialogCallback"] = &GameState::setDialogCallback;

    lua_["gameSetCharacterPosition"] = &GameState::setCharacterPosition;
    lua_["gameClearEvents"] = &GameState::clearEvents;
    lua_["gameRegisterTileEvent"] = &GameState::registerTileEvent;
  }

  sf::Vector2f &camera() { return camera_; }

  float positionOfSpriteAbove(const std::unique_ptr<Sprite> &sprite) {
    auto dim = sprite->getDimensions();
    sf::FloatRect collisionRect(dim.left, 0, dim.width, dim.top);
    for (const auto &s : sprites()) {
      if (s.first == sprite->id) {
        continue;
      }

      auto sDim = s.second->getDimensions();
      if (collisionRect.intersects(sDim)) {
        return sDim.top + sDim.height;
      }
    }
    return 0;
  }

  float positionOfSpriteBelow(const std::unique_ptr<Sprite> &sprite) {
    auto dim = sprite->getDimensions();
    sf::FloatRect collisionRect(dim.left, dim.top + dim.height, dim.width,
                                map()->pixelHeight());
    for (const auto &s : sprites()) {
      if (s.first == sprite->id) {
        continue;
      }

      auto sDim = s.second->getDimensions();
      if (collisionRect.intersects(sDim)) {
        return sDim.top - dim.height;
      }
    }
    return std::numeric_limits<float>::max();
  }

  void setTicks(unsigned int ticks) { ticks_ = ticks; }

  int ticks() { return (int)(ticks_ % INT_MAX); }

  void setHero(std::unique_ptr<Sprite> hero) { hero_ = std::move(hero); }

  const std::unique_ptr<Sprite> &hero() { return hero_; }

  void setHeroMoveSpeed(int amount, int total) {
    moveSpeed_ = (float)amount / (float)total;
  }

  int heroMoveSpeed() { return moveSpeed_; }

  unsigned int allocateSpriteId() {
    unsigned int spriteId = nextSpriteId_;
    nextSpriteId_ += 1;
    return spriteId;
  }

  void pushSprites(
      std::unordered_map<unsigned int, std::unique_ptr<Sprite>> sprites) {
    sprites_.push(std::move(sprites));
  }

  std::unordered_map<unsigned int, std::unique_ptr<Sprite>> &sprites() {
    return sprites_.top();
  }

  void popSprites() { sprites_.pop(); }

  const std::unique_ptr<Map> &map() { return maps_.top(); }

  sel::State &lua() { return lua_; }

  void addTileEvent(int id, std::string callback) {
    tileEvents_[id] = callback;
  }

  bool tileHasEvent(int id) {
    auto event = tileEvents_.find(id);
    if (event == tileEvents_.end()) {
      return false;
    }
    return true;
  }

  std::string tileCallback(int id) { return tileEvents_[id]; }

  void clearTileEvent(int id) { tileEvents_.erase(id); }

  void clearTileEvents() { tileEvents_.clear(); }

  void runTileEvent() {
    int tileNumber = map()->pointToTileNumber(hero_->getPosition());
    if (!tileHasEvent(tileNumber)) {
      return;
    }

    std::string callback = tileCallback(tileNumber);
    lua()[callback.c_str()]();
    clearTileEvent(tileNumber);
  }

  bool positionWalkable(const std::unique_ptr<Sprite> &sprite,
                        sf::FloatRect dim) {
    if (!map()->positionWalkable(dim)) {
      return false;
    }

    // Check sprite walkability
    for (auto &s : sprites()) {
      if (s.first == sprite->id) {
        continue;
      }
      auto otherDim = s.second->getDimensions();
      if (!(dim.left > otherDim.left + otherDim.width ||
            dim.left + dim.width < otherDim.left ||
            dim.top > otherDim.top + otherDim.height ||
            dim.top + dim.height < otherDim.top)) {
        return false;
      }
    }

    if (sprite != hero_) {
      auto otherDim = hero_->getDimensions();
      if (!(dim.left > otherDim.left + otherDim.width ||
            dim.left + dim.width < otherDim.left ||
            dim.top > otherDim.top + otherDim.height ||
            dim.top + dim.height < otherDim.top)) {
        return false;
      }
    }

    return true;
  }

  void markInitialized() { initialized_ = true; }

  bool initialized() { return initialized_; }

  bool loadMap(std::string path) {
    maps_.push(util::make_unique<Map>(path));
    sprites_.emplace();

    return true;
  }

  bool popMap() {
    maps_.pop();
    sprites_.pop();

    return true;
  }

  bool loadCharacter(std::string path, int tile, int initX, int initY) {
    hero_ = util::make_unique<Sprite>(path);
    hero_->setPosition(initX * map()->tileWidth(), initY * map()->tileHeight());
    hero_->setTile(tile);

    // IDs will start at 1, the hero gets 0
    hero_->id = 0;

    return true;
  }

  bool setCharacterPosition(int x, int y) {
    sf::Vector2f p = map()->mapToPixel(x, y);

    hero_->setPosition(p);
    camera_.x = p.x - SCREEN_WIDTH / 2;
    camera_.y = p.y - SCREEN_HEIGHT / 2;
    return true;
  }

  bool setCharacterMoveSpeed(int amount, int total) {
    moveSpeed_ = (float)amount / (float)total;
    return true;
  }

  bool setCharacterMaxHp(int hp) {
    hero_->setMaxHp(hp);
    return true;
  }

  bool damageCharacter(int amount) {
    hero_->damage(amount);
    return true;
  }

  bool healCharacter(int amount) {
    hero_->heal(amount);
    return true;
  }

  unsigned int addNpc(std::string path, int tile, int x, int y) {
    auto npc = util::make_unique<Npc>(path);
    npc->setPosition(x * GameState::map()->tileWidth(),
                     y * GameState::map()->tileHeight());
    npc->setTile(tile);

    unsigned int spriteId = GameState::allocateSpriteId();
    npc->id = spriteId;
    sprites()[spriteId] = std::move(npc);
    return spriteId;
  }

  // TODO(jsvana): figure out a way to not repeat the find logic all the time
  bool setNpcCallback(unsigned int npcId, std::string callback) {
    auto iter = sprites().find(npcId);
    if (iter == sprites().end()) {
      util::err()->warn("Bad NPC ID: {}", npcId);
      return false;
    }
    iter->second->callbackFunc = callback;
    return true;
  }

  bool moveNpc(unsigned int npcId, int dx, int dy) {
    auto iter = sprites().find(npcId);
    if (iter == sprites().end()) {
      util::err()->warn("Bad NPC ID: {}", npcId);
      return false;
    }
    auto pos = iter->second->getDimensions();
    pos.left += dx;
    pos.top += dy;
    if (positionWalkable(iter->second, pos)) {
      iter->second->setDimensions(pos);
      return true;
    } else {
      return false;
    }
  }

  bool setNpcMaxHp(unsigned int npcId, int hp) {
    auto iter = sprites().find(npcId);
    if (iter == sprites().end()) {
      util::err()->warn("Bad NPC ID: {}", npcId);
      return false;
    }
    iter->second->setMaxHp(hp);
    return true;
  }

  bool damageNpc(unsigned int npcId, int amount) {
    auto iter = sprites().find(npcId);
    if (iter == sprites().end()) {
      util::err()->warn("Bad NPC ID: {}", npcId);
      return false;
    }
    iter->second->damage(amount);
    return true;
  }

  bool healNpc(unsigned int npcId, int amount) {
    auto iter = sprites().find(npcId);
    if (iter == sprites().end()) {
      util::err()->warn("Bad NPC ID: {}", npcId);
      return false;
    }
    iter->second->heal(amount);
    return true;
  }

  bool jumpNpc(unsigned int npcId, int magnitude) {
    if (magnitude < 0 || magnitude > 100) {
      util::err()->warn("Jump magnitude must be between 0 and 100, inclusive");
      return false;
    }
    auto iter = sprites().find(npcId);
    if (iter == sprites().end()) {
      util::err()->warn("Bad NPC ID: {}", npcId);
      return false;
    }
    iter->second->startJump((float)magnitude / 100.0);
    return true;
  }

  unsigned int showDialog(std::string message) {
    auto dialog = new visual::Dialog(message);
    dialog->setPosition(0, SCREEN_HEIGHT - dialog->pixelHeight());
    return visual::DialogManager::queueDialog(dialog);
  }

  bool addDialogOption(unsigned int uid, std::string option) {
    return visual::DialogManager::addDialogOption(uid, option);
  }

  bool setDialogCallback(unsigned int uid, std::string callback) {
    return visual::DialogManager::setDialogCallback(uid, callback);
  }

  bool registerTileEvent(int x, int y, std::string callback) {
    addTileEvent(map()->mapPointToTileNumber(x, y), callback);
    return true;
  }

  bool clearEvents() {
    clearTileEvents();
    return true;
  }

  ///////////////////////
  // LUA API Callbacks //
  ///////////////////////

  void collision(unsigned int spriteId1, unsigned int spriteId2) {
    lua_["collision"](spriteId1, spriteId2);
  }

}  // namespace GameState
