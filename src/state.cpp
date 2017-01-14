#include "state.h"

#include "controls.h"
#include "log.h"

#include <limits.h>

#include <cmath>
#include <ostream>
#include <random>

namespace GameState {

bool initialized_ = false;

sf::Vector2f camera_;
sf::FloatRect cameraBounds_;

util::Tick ticks_ = 0;

std::unique_ptr<entities::Sprite> hero_;
float moveSpeed_;

std::queue<util::Direction> queuedMoves_;

std::unordered_map<std::string, bool> flags_;
std::unordered_map<std::string, std::list<FlagChangeCallback>>
    flagChangeCallbacks_;

std::unordered_map<std::string, int> values_;
std::unordered_map<std::string, std::list<ValueChangeCallback>>
    valueChangeCallbacks_;

// Stack is used to mimick maps_
std::stack<std::vector<std::unique_ptr<entities::Sprite>>> sprites_;

std::stack<std::unique_ptr<map::Map>> maps_;

std::unordered_map<int, std::tuple<TileCallback, bool>> tileEvents_;
std::unordered_map<int, TileCallback> tileActions_;

std::default_random_engine generator;
std::uniform_int_distribution<int> distribution(0, INT_MAX);

chaiscript::ChaiScript chai_(chaiscript::Std_Lib::library());

#define ADD_METHOD(Class, Name) chai_.add(chaiscript::fun(&Class::Name), #Name)
#define ADD_FUNCTION(Name) ADD_METHOD(GameState, Name)
#define ADD_TYPE(Type, Name) chai_.add(chaiscript::user_type<Type>(), Name);

void initApi() {
  ADD_FUNCTION(mod);
  ADD_FUNCTION(iabs);
  ADD_FUNCTION(randomNumber);

  ADD_FUNCTION(initialized);
  ADD_FUNCTION(ticks);

  ADD_FUNCTION(debug);
  ADD_FUNCTION(info);
  ADD_FUNCTION(warning);
  ADD_FUNCTION(error);

  ADD_FUNCTION(loadMap);
  ADD_FUNCTION(popMap);
  ADD_FUNCTION(loadCharacter);
  ADD_FUNCTION(addItem);
  ADD_FUNCTION(setCharacterMoveSpeed);

  ADD_FUNCTION(save);

  ADD_FUNCTION(addNpc);
  ADD_FUNCTION(setNpcCallback);

  ADD_FUNCTION(addProjectile);

  ADD_FUNCTION(dialogRunning);
  ADD_FUNCTION(showDialog);
  ADD_FUNCTION(addDialogOption);
  ADD_FUNCTION(setDialogCallback);

  ADD_FUNCTION(clearEvents);
  ADD_FUNCTION(registerTileEvent);
  ADD_FUNCTION(registerTileAction);
  ADD_FUNCTION(runTileAction);

  ADD_TYPE(entities::Sprite, "Sprite");
  ADD_METHOD(entities::Sprite, id);
  ADD_METHOD(entities::Sprite, hp);
  ADD_METHOD(entities::Sprite, setTile);
  ADD_METHOD(entities::Sprite, damage);
  ADD_METHOD(entities::Sprite, heal);
  ADD_METHOD(entities::Sprite, fullHeal);
  ADD_METHOD(entities::Sprite, startJump);
  ADD_METHOD(entities::Sprite, move);
  ADD_METHOD(entities::Sprite, getDimensions);
  ADD_METHOD(entities::Sprite, setDimensions);
  ADD_METHOD(entities::Sprite, getPosition);
  ADD_METHOD(entities::Sprite, setPosition);
  ADD_METHOD(entities::Sprite, setMaxHp);
  ADD_METHOD(entities::Sprite, width);
  ADD_METHOD(entities::Sprite, height);
  ADD_METHOD(entities::Sprite, jumping);
  ADD_METHOD(entities::Sprite, setCollisionCallback);
  ADD_METHOD(entities::Sprite, setCleanupCallback);
  ADD_METHOD(entities::Sprite, addItem);
  ADD_METHOD(entities::Sprite, getFlag);
  ADD_METHOD(entities::Sprite, setFlag);
  ADD_METHOD(entities::Sprite, getValue);
  ADD_METHOD(entities::Sprite, setValue);
  ADD_METHOD(entities::Sprite, removeItem);
  ADD_METHOD(entities::Sprite, activate);
  ADD_METHOD(entities::Sprite, deactivate);
  ADD_METHOD(entities::Sprite, markNeedsCleanup);
  ADD_METHOD(entities::Sprite, getDirection);
  ADD_METHOD(entities::Sprite, setVisualDirection);

  ADD_TYPE(entities::Item, "Item");
  ADD_METHOD(entities::Item, held);
  ADD_METHOD(entities::Item, hold);
  ADD_METHOD(entities::Item, drop);

  ADD_TYPE(entities::Projectile, "Projectile");
  ADD_METHOD(entities::Projectile, setSpeed);
  ADD_METHOD(entities::Projectile, setMaxDistance);

  ADD_TYPE(sf::Vector2f, "Vector2f");
  ADD_METHOD(sf::Vector2f, x);
  ADD_METHOD(sf::Vector2f, y);

  ADD_TYPE(sf::FloatRect, "FloatRect");
  ADD_METHOD(sf::FloatRect, left);
  ADD_METHOD(sf::FloatRect, top);
  ADD_METHOD(sf::FloatRect, width);
  ADD_METHOD(sf::FloatRect, height);

  ADD_FUNCTION(getHero);
  ADD_FUNCTION(getSprite);
  ADD_FUNCTION(getNpc);
  ADD_FUNCTION(getItem);
  ADD_FUNCTION(getProjectile);

  ADD_FUNCTION(getFlag);
  ADD_FUNCTION(setFlag);
  ADD_FUNCTION(addFlagChangeCallback);

  ADD_FUNCTION(getValue);
  ADD_FUNCTION(setValue);
  ADD_FUNCTION(addValueChangeCallback);

  chai_.add_global_const(
      chaiscript::const_var(static_cast<int>(util::Direction::LEFT)),
      "DIRECTION_LEFT");
  chai_.add_global_const(
      chaiscript::const_var(static_cast<int>(util::Direction::RIGHT)),
      "DIRECTION_RIGHT");

  ADD_FUNCTION(directionPressed);
  ADD_FUNCTION(queueMove);

  ADD_METHOD(controls, jumpPressed);
  ADD_METHOD(controls, actionPressed);
  ADD_METHOD(controls, attackPressed);
}

void debug(const std::string& msg) { logger::debug("[SCRIPT] " + msg); }
void info(const std::string& msg) { logger::info("[SCRIPT] " + msg); }
void warning(const std::string& msg) { logger::warning("[SCRIPT] " + msg); }
void error(const std::string& msg) { logger::error("[SCRIPT] " + msg); }

sf::Vector2f& camera() { return camera_; }

entities::Sprite* spriteCollision(
    const std::unique_ptr<entities::Sprite>& sprite,
    const sf::FloatRect& collisionRect) {
  for (const auto& s : sprites()) {
    if (!s || !s->active() || s->phased() || s->id == sprite->id) {
      continue;
    }

    if (collisionRect.intersects(s->getDimensions())) {
      return s.get();
    }
  }
  return nullptr;
}

float positionOfSpriteAbove(const std::unique_ptr<entities::Sprite>& sprite) {
  auto dim = sprite->getDimensions();
  sf::FloatRect collisionRect(dim.left, 0, dim.width, dim.top);
  const auto other = spriteCollision(sprite, collisionRect);
  if (other) {
    const auto oDim = other->getDimensions();
    return oDim.top + oDim.height;
  }
  return 0;
}

float densePositionAbove(const std::unique_ptr<entities::Sprite>& sprite) {
  float spriteAbove = positionOfSpriteAbove(sprite);
  float tileAbove = map()->positionOfTileAbove(sprite->getDimensions());
  return std::max<float>(spriteAbove, tileAbove);
}

float positionOfSpriteBelow(const std::unique_ptr<entities::Sprite>& sprite) {
  auto dim = sprite->getDimensions();
  sf::FloatRect collisionRect(dim.left, dim.top + dim.height, dim.width,
                              (float)map()->pixelHeight());
  const auto other = spriteCollision(sprite, collisionRect);
  if (other) {
    const auto oDim = other->getDimensions();
    return oDim.top - dim.height;
  }
  return std::numeric_limits<float>::max();
}

// TODO(jsvana): make this and Above return the sprite if there is one
// for collisions
float densePositionBelow(const std::unique_ptr<entities::Sprite>& sprite) {
  float spriteBelow = positionOfSpriteBelow(sprite);
  float tileBelow = map()->positionOfTileBelow(sprite->getDimensions());
  return std::min<float>(spriteBelow, tileBelow);
}

void dispatchCollisions() {
  for (std::size_t i = 1; i < sprites().size(); i++) {
    const auto sprite1 = sprites()[i].get();
    if (!sprite1->active()) {
      continue;
    }
    const auto dim1 = sprite1->getDimensions();
    for (std::size_t j = i + 1; j < sprites().size(); j++) {
      const auto sprite2 = sprites()[j].get();
      if (!sprite2->active()) {
        continue;
      }
      if (dim1.intersects(sprite2->getDimensions())) {
        dispatchCollision(sprite1, sprite2);
      }
    }
    if (dim1.intersects(hero()->getDimensions())) {
      dispatchCollision(sprite1, hero().get());
    }
  }
}

int mod(int a, int b) { return a % b; }
int iabs(int a) { return abs(a); }

int randomNumber(int min, int max) {
  return (distribution(generator) % (max - min)) + min;
}

void setTicks(util::Tick ticks) { ticks_ = ticks; }

void tick() { ++ticks_; }

int ticks() { return (int)(ticks_ % INT_MAX); }

void setHero(std::unique_ptr<entities::Sprite> hero) {
  hero_ = std::move(hero);
}

const std::unique_ptr<entities::Sprite>& hero() { return hero_; }

entities::Sprite* getHero() { return hero_.get(); }

void setHeroMoveSpeed(int amount, int total) {
  moveSpeed_ = (float)amount / (float)total;
}

float heroMoveSpeed() { return moveSpeed_; }

void pushSprites(std::vector<std::unique_ptr<entities::Sprite>> sprites) {
  sprites_.push(std::move(sprites));
}

std::vector<std::unique_ptr<entities::Sprite>>& sprites() {
  return sprites_.top();
}

void popSprites() { sprites_.pop(); }

const std::unique_ptr<map::Map>& map() { return maps_.top(); }

chaiscript::ChaiScript& chai() { return chai_; }

void addTileEvent(int id, TileCallback callback, bool clearOnFire) {
  tileEvents_[id] = std::make_tuple(callback, clearOnFire);
}

void addTileAction(int id, TileCallback callback) {
  tileActions_[id] = callback;
}

bool tileHasEvent(int id) { return tileEvents_.find(id) != tileEvents_.end(); }

const std::tuple<TileCallback, bool>& tileEvent(int id) {
  return tileEvents_[id];
}

bool tileHasAction(int id) {
  return tileActions_.find(id) != tileActions_.end();
}

const TileCallback& tileAction(int id) { return tileActions_[id]; }

void clearTileEvent(int id) { tileEvents_.erase(id); }

void clearTileEvents() { tileEvents_.clear(); }

void runTileEvent() {
  int tileNumber = map()->pointToTileNumber(hero_->getPosition());
  if (!tileHasEvent(tileNumber)) {
    return;
  }

  auto p = tileEvent(tileNumber);
  std::get<TileCallback>(p)();
  if (std::get<bool>(p)) {
    clearTileEvent(tileNumber);
  }
}

void runTileAction() {
  int tileNumber = map()->pointToTileNumber(hero_->getPosition());
  if (!tileHasAction(tileNumber)) {
    return;
  }

  tileAction(tileNumber)();
}

bool positionWalkable(const std::unique_ptr<entities::Sprite>& sprite,
                      sf::FloatRect dim) {
  return positionWalkable(sprite.get(), dim);
}

bool positionWalkable(entities::Sprite* sprite, sf::FloatRect dim) {
  if (!map()->positionWalkable(dim)) {
    return false;
  }

  // Check sprite walkability
  for (const auto& s : sprites()) {
    if (!s || !s->active() || s->id == sprite->id) {
      continue;
    }
    sf::FloatRect intersection;
    if (dim.intersects(s->getDimensions(), intersection)) {
      dispatchCollision(sprite, s.get());
      if (!s->phased()) {
        return false;
      }
    }
  }

  if (sprite != hero_.get() && dim.intersects(hero_->getDimensions())) {
    dispatchCollision(sprite, hero_.get());
    if (!sprite->phased()) {
      return false;
    }
  }

  return true;
}

void dispatchCollision(entities::Sprite* mover, entities::Sprite* other) {
  if (mover->collisionFunc) {
    mover->collisionFunc(mover->id, other->id);
  }
  if (other->collisionFunc) {
    other->collisionFunc(other->id, mover->id);
  }
}

void markInitialized() { initialized_ = true; }

bool initialized() { return initialized_; }

void queueMove(const int dir) {
  queuedMoves_.push(static_cast<util::Direction>(dir));
}

std::queue<util::Direction>& moves() { return queuedMoves_; }

bool directionPressed(const int dir) {
  return controls::directionPressed(static_cast<util::Direction>(dir));
}

bool loadMap(std::string path) {
  maps_.push(std::make_unique<map::Map>(path));
  sprites_.emplace();
  sprites().emplace_back();

  return true;
}

bool popMap() {
  maps_.pop();
  sprites_.pop();

  return true;
}

bool loadCharacter(std::string path, float initX, float initY) {
  hero_ = std::make_unique<entities::Sprite>(path);
  hero_->setPosition(initX * map()->tileWidth(), initY * map()->tileHeight());

  // IDs will start at 1, the hero gets 0
  hero_->id = 0;

  return true;
}

bool setSpritePosition(entities::Id spriteId, float x, float y) {
  auto sprite = findSprite<entities::Sprite>(spriteId);
  if (!sprite) {
    return false;
  }

  sf::Vector2f p = map()->mapToPixel(x, y);
  sprite->setPosition(p.x, p.y);
  return true;
}

bool setCharacterMoveSpeed(float speed) {
  moveSpeed_ = speed;
  return true;
}

bool setCharacterMaxHp(int hp) {
  hero_->setMaxHp(hp);
  return true;
}

template <typename T>
entities::Id addSprite(const std::string& path, float x, float y) {
  sprites().push_back(std::make_unique<T>(path));
  auto item = sprites().back().get();
  item->setPosition(x * GameState::map()->tileWidth(),
                    y * GameState::map()->tileHeight());

  const auto spriteId = sprites().size() - 1;
  item->id = spriteId;
  return spriteId;
}

entities::Id addItem(const std::string& path, float x, float y) {
  return addSprite<entities::Item>(path, x, y);
}

entities::Id addNpc(const std::string& path, float x, float y) {
  return addSprite<entities::Npc>(path, x, y);
}

entities::Id addProjectile(const std::string& path, float x, float y) {
  return addSprite<entities::Projectile>(path, x, y);
}

template <typename T>
T* findSprite(const entities::Id spriteId) {
  if (spriteId >= sprites().size()) {
    logger::warning("Bad sprite ID: " + spriteId);
    return nullptr;
  }
  return static_cast<T*>(sprites()[spriteId].get());
}

template <typename T>
T* findSprite(const entities::Id spriteId, const entities::SpriteType type) {
  auto sprite = findSprite<T>(spriteId);
  if (!sprite) {
    return nullptr;
  }
  if (sprites()[spriteId]->type() != type) {
    logger::warning(
        "ID " + std::to_string(spriteId) + " is incorrect type " +
        std::to_string(static_cast<int>(type)) + " (wanted type " +
        std::to_string(static_cast<int>(sprites()[spriteId]->type())) + ")");
    return nullptr;
  }
  return sprite;
}

entities::Sprite* getSprite(const entities::Id spriteId) {
  return findSprite<entities::Sprite>(spriteId);
}

entities::Npc* getNpc(const entities::Id npcId) {
  return findSprite<entities::Npc>(npcId, entities::SpriteType::NPC);
}

entities::Projectile* getProjectile(const entities::Id projectileId) {
  return findSprite<entities::Projectile>(projectileId,
                                          entities::SpriteType::PROJECTILE);
}

entities::Item* getItem(const entities::Id itemId) {
  return findSprite<entities::Item>(itemId, entities::SpriteType::ITEM);
}

bool setNpcCallback(const entities::Id npcId,
                    entities::SpriteCallback callback) {
  auto npc = findSprite<entities::Npc>(npcId, entities::SpriteType::NPC);
  if (!npc) {
    return false;
  }
  npc->callbackFunc = callback;
  return true;
}

visual::DialogManager::Id showDialog(std::string message) {
  auto dialog = new visual::Dialog(message);
  dialog->setPosition(0, (float)SCREEN_HEIGHT - dialog->pixelHeight());
  return visual::DialogManager::queueDialog(dialog);
}

bool dialogRunning() { return visual::DialogManager::running(); }

bool addDialogOption(visual::DialogManager::Id uid, std::string option) {
  return visual::DialogManager::addDialogOption(uid, option);
}

bool setDialogCallback(visual::DialogManager::Id uid,
                       visual::DialogCallback callback) {
  return visual::DialogManager::setDialogCallback(uid, callback);
}

bool registerTileEvent(int x, int y, TileCallback callback, bool clearOnFire) {
  addTileEvent(map()->mapPointToTileNumber(x, y), callback, clearOnFire);
  return true;
}

bool registerTileAction(int x, int y, TileCallback callback) {
  addTileAction(map()->mapPointToTileNumber(x, y), callback);
  return true;
}

bool clearEvents() {
  clearTileEvents();
  return true;
}

void setFlag(const std::string& flag, bool value) {
  flags_[flag] = value;
  for (auto& callback : flagChangeCallbacks(flag)) {
    callback(value);
  }
}

bool getFlag(const std::string& flag) {
  const auto iter = flags_.find(flag);
  if (iter == flags_.end()) {
    return false;
  }
  return iter->second;
}

const std::list<FlagChangeCallback> flagChangeCallbacks(
    const std::string& flag) {
  const auto iter = flagChangeCallbacks_.find(flag);
  if (iter == flagChangeCallbacks_.end()) {
    return std::list<FlagChangeCallback>();
  }
  return iter->second;
}

void addFlagChangeCallback(const std::string& flag,
                           const FlagChangeCallback& func) {
  flagChangeCallbacks_[flag].push_back(func);
}

void setValue(const std::string& key, const int value) {
  values_[key] = value;
  for (auto& callback : valueChangeCallbacks(key)) {
    callback(value);
  }
}

int getValue(const std::string& key) {
  const auto iter = values_.find(key);
  if (iter == values_.end()) {
    return 0;
  }
  return iter->second;
}

const std::list<ValueChangeCallback> valueChangeCallbacks(
    const std::string& key) {
  const auto iter = valueChangeCallbacks_.find(key);
  if (iter == valueChangeCallbacks_.end()) {
    return std::list<ValueChangeCallback>();
  }
  return iter->second;
}

void addValueChangeCallback(const std::string& key,
                            const ValueChangeCallback& func) {
  valueChangeCallbacks_[key].push_back(func);
}

bool save(const std::string& path) {
  nlohmann::json gameState;
  gameState["hero"] = hero()->serialize();

  nlohmann::json savedSprites;
  for (const auto& sprite : sprites()) {
    if (!sprite) {
      continue;
    }
    savedSprites[sprite->id] = sprite->serialize();
  }
  gameState["sprites"] = savedSprites;
  gameState["flags"] = flags_;
  gameState["values"] = values_;

  std::ofstream saveFile(path);

  if (!saveFile.is_open()) {
    logger::error("Unable to open save file: " + path);
    return false;
  }

  saveFile << gameState.dump(4);
  saveFile.close();

  logger::info("Game saved to " + path);

  return true;
}

bool load(const std::string& path) {
  std::ifstream saveFile(path, std::ios::in);
  if (!saveFile.is_open()) {
    logger::warning("Unable to open save file: " + path);
    return false;
  }

  std::stringstream fileData;
  fileData << saveFile.rdbuf();
  saveFile.close();

  const auto saveData = nlohmann::json::parse(fileData.str());

  const auto heroData =
      saveData["hero"].get<std::unordered_map<std::string, nlohmann::json>>();
  hero_ = std::make_unique<entities::Sprite>(
      heroData.find("path")->second.get<std::string>());
  hero_->deserialize(heroData);

  sprites().clear();
  sprites().emplace_back();
  for (const auto& spriteData :
       saveData["sprites"].get<std::vector<nlohmann::json>>()) {
    if (spriteData.is_null()) {
      continue;
    }
    const auto spritePath = spriteData["path"].get<std::string>();
    const auto spriteType = static_cast<entities::SpriteType>(
        spriteData["type"].get<entities::Id>());
    switch (spriteType) {
      case entities::SpriteType::NPC:
        sprites().push_back(std::make_unique<entities::Npc>(spritePath));
        break;
      case entities::SpriteType::ITEM:
        sprites().push_back(std::make_unique<entities::Item>(spritePath));
        break;
      case entities::SpriteType::PROJECTILE:
        sprites().push_back(std::make_unique<entities::Projectile>(spritePath));
        break;
      default:
        sprites().push_back(std::make_unique<entities::Sprite>(spritePath));
        break;
    }
    sprites().back()->deserialize(spriteData);
  }
  flags_.clear();
  for (const auto& p :
       saveData["flags"].get<std::unordered_map<std::string, bool>>()) {
    flags_[p.first] = p.second;
  }
  values_.clear();
  for (const auto& p :
       saveData["values"].get<std::unordered_map<std::string, int>>()) {
    values_[p.first] = p.second;
  }

  chai().eval<std::function<void()>>("restoreCallbacks")();

  return true;
}

}  // namespace GameState
