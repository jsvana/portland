#include "state.h"

namespace GameState {

  BoundedPoint camera_;

  unsigned int ticks_ = 0;

  Sprite *hero_;
  int moveSpeed_;

  // Jumping stuff
  bool jumping_ = false;
  int velocityY_ = 0;

  // Stack is used to mimick maps_
  unsigned int nextSpriteId_ = 1;
  std::stack<std::unordered_map<unsigned int, Sprite *>> sprites_;

  std::stack<Map *> maps_;

  std::unordered_map<int, std::string> tileEvents_;

  // Contains all scripting state
  sel::State lua_{true};

  void initLuaApi() {
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

  void updateCamera() {
    camera_.xMin = 0;
    camera_.xMax = map()->pixelWidth() - SCREEN_WIDTH;
    camera_.yMin = 0;
    camera_.yMax = map()->pixelHeight() - SCREEN_HEIGHT;
  }

  BoundedPoint &camera() { return camera_; }

  void setTicks(unsigned int ticks) { ticks_ = ticks; }

  int ticks() { return (int)(ticks_ % INT_MAX); }

  void setHero(Sprite *hero) { hero_ = hero; }

  Sprite *hero() { return hero_; }

  void setHeroMoveSpeed(int speed) { moveSpeed_ = speed; }

  int heroMoveSpeed() { return moveSpeed_; }

  unsigned int allocateSpriteId() {
    unsigned int spriteId = nextSpriteId_;
    nextSpriteId_ += 1;
    return spriteId;
  }

  void pushSprites(std::unordered_map<unsigned int, Sprite *> sprites) {
    sprites_.push(sprites);
  }

  std::unordered_map<unsigned int, Sprite *> &sprites() {
    return sprites_.top();
  }

  void popSprites() { sprites_.pop(); }

  void pushMap(Map *map) { maps_.push(map); }

  Map *map() { return maps_.top(); }

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

  bool positionWalkable(Sprite *sprite, Rect dim) {
    if (!map()->positionWalkable(dim)) {
      return false;
    }

    // Check sprite walkability
    for (auto &s : sprites()) {
      if (s.first == sprite->id) {
        continue;
      }
      Rect otherDim = s.second->getDimensions();
      if (!(dim.x > otherDim.x + otherDim.w || dim.x + dim.w < otherDim.x ||
            dim.y > otherDim.y + otherDim.h || dim.y + dim.h < otherDim.y)) {
        return false;
      }
    }

    if (sprite != hero_) {
      Rect otherDim = hero_->getDimensions();
      if (!(dim.x > otherDim.x + otherDim.w || dim.x + dim.w < otherDim.x ||
            dim.y > otherDim.y + otherDim.h || dim.y + dim.h < otherDim.y)) {
        return false;
      }
    }

    return true;
  }

  bool loadMap(std::string path) {
    Map *map = new Map(path);
    maps_.push(map);
    std::unordered_map<unsigned int, Sprite *> sprites;
    sprites_.push(sprites);

    updateCamera();

    return true;
  }

  bool popMap() {
    maps_.pop();
    sprites_.pop();

    updateCamera();

    return true;
  }

  bool loadCharacter(std::string path, int tile, int initX, int initY) {
    hero_ = new Sprite(path);
    hero_->setPosition(initX * map()->tileWidth(), initY * map()->tileHeight());
    hero_->setTile(tile);
    return true;
  }

  bool setCharacterPosition(int x, int y) {
    Point p = map()->mapToPixel(x, y);

    hero_->setPosition(p);
    camera_.setPosition(p.x - SCREEN_WIDTH / 2, p.y - SCREEN_HEIGHT / 2);
    return true;
  }

  bool setCharacterMoveSpeed(int speed) {
    moveSpeed_ = speed;
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
    Npc *npc = new Npc(path);
    npc->setPosition(x * GameState::map()->tileWidth(),
                     y * GameState::map()->tileHeight());
    npc->setTile(tile);

    unsigned int spriteId = GameState::allocateSpriteId();
    npc->id = spriteId;
    sprites()[spriteId] = npc;
    return spriteId;
  }

  // TODO(jsvana): figure out a way to not repeat the find logic all the time
  bool setNpcCallback(unsigned int npcId, std::string callback) {
    auto iter = sprites().find(npcId);
    if (iter == sprites().end()) {
      err()->warn("Bad NPC ID: {}", npcId);
      return false;
    }
    iter->second->callbackFunc = callback;
    return true;
  }

  bool moveNpc(unsigned int npcId, int dx, int dy) {
    auto iter = sprites().find(npcId);
    if (iter == sprites().end()) {
      err()->warn("Bad NPC ID: {}", npcId);
      return false;
    }
    auto npc = iter->second;
    auto pos = npc->getDimensions();
    pos.x += dx;
    pos.y += dy;
    if (positionWalkable(npc, pos)) {
      npc->setDimensions(pos);
      return true;
    } else {
      return false;
    }
  }

  bool setNpcMaxHp(unsigned int npcId, int hp) {
    auto iter = sprites().find(npcId);
    if (iter == sprites().end()) {
      err()->warn("Bad NPC ID: {}", npcId);
      return false;
    }
    iter->second->setMaxHp(hp);
    return true;
  }

  bool damageNpc(unsigned int npcId, int amount) {
    auto iter = sprites().find(npcId);
    if (iter == sprites().end()) {
      err()->warn("Bad NPC ID: {}", npcId);
      return false;
    }
    iter->second->damage(amount);
    return true;
  }

  bool healNpc(unsigned int npcId, int amount) {
    auto iter = sprites().find(npcId);
    if (iter == sprites().end()) {
      err()->warn("Bad NPC ID: {}", npcId);
      return false;
    }
    iter->second->heal(amount);
    return true;
  }

  bool jumpNpc(unsigned int npcId, int magnitude) {
    if (magnitude < 0 || magnitude > 100) {
      err()->warn("Jump magnitude must be between 0 and 100, inclusive");
      return false;
    }
    auto iter = sprites().find(npcId);
    if (iter == sprites().end()) {
      err()->warn("Bad NPC ID: {}", npcId);
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
