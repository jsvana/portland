#include "state.h"

#include "quadtree.h"

#include <glog/logging.h>

#include <limits.h>

namespace GameState {

bool initialized_ = false;

sf::Vector2f camera_;
sf::FloatRect cameraBounds_;

util::Tick ticks_ = 0;

std::unique_ptr<entities::Sprite> hero_;
float moveSpeed_;

// Jumping stuff
bool jumping_ = false;
int velocityY_ = 0;

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

chaiscript::ChaiScript chai_(chaiscript::Std_Lib::library());

#define ADD_METHOD(Class, Name) chai_.add(chaiscript::fun(&Class::Name), #Name)
#define ADD_FUNCTION(Name) ADD_METHOD(GameState, Name)
#define ADD_TYPE(Type, Name) chai_.add(chaiscript::user_type<Type>(), Name);

void initApi() {
  ADD_FUNCTION(mod);

  ADD_FUNCTION(initialized);
  ADD_FUNCTION(ticks);

  ADD_FUNCTION(loadMap);
  ADD_FUNCTION(popMap);
  ADD_FUNCTION(loadCharacter);
  ADD_FUNCTION(addItem);
  ADD_FUNCTION(setCharacterMoveSpeed);

  ADD_FUNCTION(addNpc);
  ADD_FUNCTION(setNpcCallback);

  ADD_FUNCTION(showDialog);
  ADD_FUNCTION(addDialogOption);
  ADD_FUNCTION(setDialogCallback);

  ADD_FUNCTION(clearEvents);
  ADD_FUNCTION(registerTileEvent);

  ADD_TYPE(entities::Sprite, "Sprite");
  ADD_METHOD(entities::Sprite, id);
  ADD_METHOD(entities::Sprite, hp);
  ADD_METHOD(entities::Sprite, damage);
  ADD_METHOD(entities::Sprite, heal);
  ADD_METHOD(entities::Sprite, startJump);
  ADD_METHOD(entities::Sprite, move);
  ADD_METHOD(entities::Sprite, setMaxHp);
  ADD_METHOD(entities::Sprite, width);
  ADD_METHOD(entities::Sprite, height);
  ADD_METHOD(entities::Sprite, jumping);
  ADD_METHOD(entities::Sprite, setCollisionCallback);
  ADD_METHOD(entities::Sprite, addItem);
  ADD_METHOD(entities::Sprite, removeItem);

  ADD_TYPE(entities::Item, "Item");
  ADD_METHOD(entities::Item, held);
  ADD_METHOD(entities::Item, hold);
  ADD_METHOD(entities::Item, drop);

  ADD_FUNCTION(getHero);
  ADD_FUNCTION(getSprite);
  ADD_FUNCTION(getNpc);
  ADD_FUNCTION(getItem);

  ADD_FUNCTION(getFlag);
  ADD_FUNCTION(setFlag);
  ADD_FUNCTION(addFlagChangeCallback);

  ADD_FUNCTION(getValue);
  ADD_FUNCTION(setValue);
  ADD_FUNCTION(addValueChangeCallback);
}

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
                              map()->pixelHeight());
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

int mod(int a, int b) { return a % b; }

void setTicks(util::Tick ticks) { ticks_ = ticks; }

int ticks() { return (int)(ticks_ % INT_MAX); }

void setHero(std::unique_ptr<entities::Sprite> hero) {
  hero_ = std::move(hero);
}

const std::unique_ptr<entities::Sprite>& hero() { return hero_; }

entities::Sprite* getHero() { return hero_.get(); }

void setHeroMoveSpeed(int amount, int total) {
  moveSpeed_ = (float)amount / (float)total;
}

int heroMoveSpeed() { return moveSpeed_; }

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

bool tileHasEvent(int id) { return tileEvents_.find(id) != tileEvents_.end(); }

const std::tuple<TileCallback, bool>& tileCallback(int id) {
  return tileEvents_[id];
}

void clearTileEvent(int id) { tileEvents_.erase(id); }

void clearTileEvents() { tileEvents_.clear(); }

void runTileEvent() {
  int tileNumber = map()->pointToTileNumber(hero_->getPosition());
  if (!tileHasEvent(tileNumber)) {
    return;
  }

  auto p = tileCallback(tileNumber);
  std::get<TileCallback>(p)();
  if (std::get<bool>(p)) {
    clearTileEvent(tileNumber);
  }
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
  if (!mover->collisionFunc) {
    return;
  }
  mover->collisionFunc(other->id);
}

void markInitialized() { initialized_ = true; }

bool initialized() { return initialized_; }

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

bool loadCharacter(std::string path, int tile, int initX, int initY) {
  hero_ = std::make_unique<entities::Sprite>(path);
  hero_->setPosition(initX * map()->tileWidth(), initY * map()->tileHeight());
  hero_->setTile(tile);

  // IDs will start at 1, the hero gets 0
  hero_->id = 0;

  return true;
}

bool setSpritePosition(entities::Id spriteId, int x, int y) {
  auto sprite = findSprite<entities::Sprite>(spriteId);
  if (!sprite) {
    return false;
  }

  sf::Vector2f p = map()->mapToPixel(x, y);

  sprite->setPosition(p);
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

template <typename T>
entities::Id addSprite(const std::string& path, int tile, int x, int y) {
  sprites().push_back(std::make_unique<T>(path));
  auto item = sprites().back().get();
  item->setPosition(x * GameState::map()->tileWidth(),
                    y * GameState::map()->tileHeight());
  item->setTile(tile);

  const auto spriteId = sprites().size() - 1;
  item->id = spriteId;
  return spriteId;
}

entities::Id addItem(const std::string& path, int tile, int x, int y) {
  return addSprite<entities::Item>(path, tile, x, y);
}

entities::Id addNpc(const std::string& path, int tile, int x, int y) {
  return addSprite<entities::Npc>(path, tile, x, y);
}

template <typename T>
T* findSprite(const entities::Id spriteId) {
  if (spriteId >= sprites().size()) {
    LOG(WARNING) << "Bad sprite ID: " << spriteId;
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
    LOG(WARNING) << "ID " << spriteId << " is incorrect type "
                 << static_cast<int>(type) << " (wanted type "
                 << static_cast<int>(sprites()[spriteId]->type()) << ")";
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
  dialog->setPosition(0, SCREEN_HEIGHT - dialog->pixelHeight());
  return visual::DialogManager::queueDialog(dialog);
}

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

}  // namespace GameState
