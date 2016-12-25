#include "state.h"

#include "quadtree.h"

#include <glog/logging.h>

#include <limits.h>

namespace GameState {

bool initialized_ = false;

sf::Vector2f camera_;
sf::FloatRect cameraBounds_;

unsigned int ticks_ = 0;

std::unique_ptr<entities::Sprite> hero_;
float moveSpeed_;

// Jumping stuff
bool jumping_ = false;
int velocityY_ = 0;

// Stack is used to mimick maps_
std::stack<std::vector<std::unique_ptr<entities::Sprite>>> sprites_;

std::stack<std::unique_ptr<Map>> maps_;

std::unordered_map<int, TileCallback> tileEvents_;

chaiscript::ChaiScript chai_(chaiscript::Std_Lib::library());

std::string helloWorld(const std::string& t_name) {
  return "Hello " + t_name + "!";
}

void initApi() {
  chai_.add(chaiscript::fun(&mod), "mod");

  chai_.add(chaiscript::fun(&helloWorld), "helloWorld");

  chai_.add(chaiscript::fun(&GameState::initialized), "gameInitialized");
  chai_.add(chaiscript::fun(&GameState::ticks), "gameTicks");

  chai_.add(chaiscript::fun(&GameState::loadMap), "gameLoadMap");
  chai_.add(chaiscript::fun(&GameState::popMap), "gamePopMap");
  chai_.add(chaiscript::fun(&GameState::loadCharacter), "gameLoadCharacter");
  chai_.add(chaiscript::fun(&GameState::setCharacterMoveSpeed),
            "gameSetCharacterMoveSpeed");
  chai_.add(chaiscript::fun(&GameState::setCharacterMaxHp),
            "gameSetCharacterMaxHp");
  chai_.add(chaiscript::fun(&GameState::damageCharacter),
            "gameDamageCharacter");
  chai_.add(chaiscript::fun(&GameState::healCharacter), "gameHealCharacter");

  chai_.add(chaiscript::fun(&GameState::addItem), "gameAddItem");

  chai_.add(chaiscript::fun(&GameState::addNpc), "gameAddNpc");
  chai_.add(chaiscript::fun(&GameState::setNpcCallback), "gameSetNpcCallback");
  chai_.add(chaiscript::fun(&GameState::moveNpc), "gameMoveNpc");
  chai_.add(chaiscript::fun(&GameState::setNpcMaxHp), "gameSetNpcMaxHp");
  chai_.add(chaiscript::fun(&GameState::damageNpc), "gameDamageNpc");
  chai_.add(chaiscript::fun(&GameState::healNpc), "gameHealNpc");
  chai_.add(chaiscript::fun(&GameState::jumpNpc), "gameJumpNpc");

  chai_.add(chaiscript::fun(&GameState::showDialog), "gameShowDialog");
  chai_.add(chaiscript::fun(&GameState::addDialogOption),
            "gameAddDialogOption");
  chai_.add(chaiscript::fun(&GameState::setDialogCallback),
            "gameSetDialogCallback");

  chai_.add(chaiscript::fun(&GameState::setCharacterPosition),
            "gameSetCharacterPosition");
  chai_.add(chaiscript::fun(&GameState::clearEvents), "gameClearEvents");
  chai_.add(chaiscript::fun(&GameState::registerTileEvent),
            "gameRegisterTileEvent");
}

sf::Vector2f& camera() { return camera_; }

float positionOfSpriteAbove(const std::unique_ptr<entities::Sprite>& sprite) {
  auto dim = sprite->getDimensions();
  sf::FloatRect collisionRect(dim.left, 0, dim.width, dim.top);
  for (const auto& s : sprites()) {
    if (s->id == sprite->id) {
      continue;
    }

    auto sDim = s->getDimensions();
    if (collisionRect.intersects(sDim)) {
      return sDim.top + sDim.height;
    }
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
  for (const auto& s : sprites()) {
    if (s->id == sprite->id) {
      continue;
    }

    auto sDim = s->getDimensions();
    if (collisionRect.intersects(sDim)) {
      return sDim.top - dim.height;
    }
  }
  return std::numeric_limits<float>::max();
}

float densePositionBelow(const std::unique_ptr<entities::Sprite>& sprite) {
  float spriteBelow = positionOfSpriteBelow(sprite);
  float tileBelow = map()->positionOfTileBelow(sprite->getDimensions());
  return std::min<float>(spriteBelow, tileBelow);
}

int mod(int a, int b) { return a % b; }

void setTicks(unsigned int ticks) { ticks_ = ticks; }

int ticks() { return (int)(ticks_ % INT_MAX); }

void setHero(std::unique_ptr<entities::Sprite> hero) {
  hero_ = std::move(hero);
}

const std::unique_ptr<entities::Sprite>& hero() { return hero_; }

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

const std::unique_ptr<Map>& map() { return maps_.top(); }

chaiscript::ChaiScript& chai() { return chai_; }

void addTileEvent(int id, TileCallback callback) { tileEvents_[id] = callback; }

bool tileHasEvent(int id) {
  auto event = tileEvents_.find(id);
  if (event == tileEvents_.end()) {
    return false;
  }
  return true;
}

const TileCallback& tileCallback(int id) { return tileEvents_[id]; }

void clearTileEvent(int id) { tileEvents_.erase(id); }

void clearTileEvents() { tileEvents_.clear(); }

void runTileEvent() {
  int tileNumber = map()->pointToTileNumber(hero_->getPosition());
  if (!tileHasEvent(tileNumber)) {
    return;
  }

  TileCallback callback = tileCallback(tileNumber);
  callback();
  clearTileEvent(tileNumber);
}

bool positionWalkable(const std::unique_ptr<entities::Sprite>& sprite,
                      sf::FloatRect dim) {
  return positionWalkable(sprite.get(), dim);
}

static void printRect(const sf::FloatRect& rect) {
  LOG(INFO) << "(" << rect.left << ", " << rect.top << "), (" << rect.width
            << ", " << rect.height << ")";
}

bool positionWalkable(entities::Sprite* sprite, sf::FloatRect dim) {
  if (sprite->phased()) {
    return true;
  }

  if (!map()->positionWalkable(dim)) {
    return false;
  }

  // Check sprite walkability
  for (const auto& s : sprites()) {
    if (s->phased()) {
      continue;
    }
    if (s->id == sprite->id) {
      continue;
    }
    sf::FloatRect intersection;
    if (sprite->getDimensions().intersects(s->getDimensions(), intersection)) {
      LOG(INFO) << "sprite " << sprite->id;
      printRect(sprite->getDimensions());
      LOG(INFO) << "s " << s->id;
      printRect(s->getDimensions());
      LOG(INFO) << "intersect ";
      printRect(intersection);
      return false;
    }
  }

  if (sprite != hero_.get() &&
      sprite->getDimensions().intersects(hero_->getDimensions())) {
    return false;
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
  hero_ = util::make_unique<entities::Sprite>(path);
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

unsigned int addItem(std::string path, int tile, int x, int y) {
  auto item = util::make_unique<entities::Item>(path);
  item->setPosition(x * GameState::map()->tileWidth(),
                    y * GameState::map()->tileHeight());
  item->setTile(tile);

  unsigned int spriteId = sprites().size() + 1;
  item->id = spriteId;
  sprites().push_back(std::move(item));
  return spriteId;
}

unsigned int addNpc(std::string path, int tile, int x, int y) {
  auto npc = util::make_unique<entities::Npc>(path);
  npc->setPosition(x * GameState::map()->tileWidth(),
                   y * GameState::map()->tileHeight());
  npc->setTile(tile);

  unsigned int spriteId = sprites().size() + 1;
  npc->id = spriteId;
  sprites().push_back(std::move(npc));
  return spriteId;
}

template <typename T>
T* findSprite(unsigned int spriteId, const entities::SpriteType type) {
  if (spriteId >= sprites().size()) {
    LOG(WARNING) << "Bad sprite ID: " << spriteId;
    return nullptr;
  }
  if (sprites()[spriteId]->type() != type) {
    LOG(WARNING) << "ID " << spriteId << " is incorrect type "
                 << static_cast<int>(type) << " (wanted type "
                 << static_cast<int>(sprites()[spriteId]->type()) << ")";
    return nullptr;
  }
  return static_cast<T*>(sprites()[spriteId].get());
}

bool setNpcCallback(unsigned int npcId, entities::SpriteCallback callback) {
  auto npc = findSprite<entities::Npc>(npcId, entities::SpriteType::NPC);
  if (npc == nullptr) {
    return false;
  }
  npc->callbackFunc = callback;
  return true;
}

bool moveNpc(unsigned int npcId, int dx, int dy) {
  auto npc = findSprite<entities::Npc>(npcId, entities::SpriteType::NPC);
  if (npc == nullptr) {
    return false;
  }
  auto pos = npc->getDimensions();
  pos.left += dx;
  pos.top += dy;
  if (positionWalkable(npc, pos)) {
    npc->setDimensions(pos);
    return true;
  }
  return false;
}

bool setNpcMaxHp(unsigned int npcId, int hp) {
  auto npc = findSprite<entities::Npc>(npcId, entities::SpriteType::NPC);
  if (npc == nullptr) {
    return false;
  }
  npc->setMaxHp(hp);
  return true;
}

bool damageNpc(unsigned int npcId, int amount) {
  auto npc = findSprite<entities::Npc>(npcId, entities::SpriteType::NPC);
  if (npc == nullptr) {
    return false;
  }
  npc->damage(amount);
  return true;
}

bool healNpc(unsigned int npcId, int amount) {
  auto npc = findSprite<entities::Npc>(npcId, entities::SpriteType::NPC);
  if (npc == nullptr) {
    return false;
  }
  npc->heal(amount);
  return true;
}

bool jumpNpc(unsigned int npcId, int magnitude) {
  if (magnitude < 0 || magnitude > 100) {
    LOG(WARNING) << "Jump magnitude must be between 0 and 100, inclusive";
    return false;
  }
  auto npc = findSprite<entities::Npc>(npcId, entities::SpriteType::NPC);
  if (npc == nullptr) {
    return false;
  }
  npc->startJump((float)magnitude / 100.0);
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

bool setDialogCallback(unsigned int uid, DialogCallback callback) {
  return visual::DialogManager::setDialogCallback(uid, callback);
}

bool registerTileEvent(int x, int y, TileCallback callback) {
  addTileEvent(map()->mapPointToTileNumber(x, y), callback);
  return true;
}

bool clearEvents() {
  clearTileEvents();
  return true;
}

///////////////////
// API Callbacks //
///////////////////

void collision(unsigned int, unsigned int) {
  // lua_["collision"](spriteId1, spriteId2);
}

}  // namespace GameState
