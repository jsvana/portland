local helpers = require("assets.scripts.helpers")

SPRITE_PATH = "assets/sprites/hero.json"
HERO_TILE = 0
UPDATE_INTERVAL = 100
TILE_DIM = 16

function init()
  gameLoadMap("assets/maps/main.json")
  gameLoadCharacter(SPRITE_PATH, HERO_TILE, 1, 6)
  gameSetCharacterMoveSpeed(4)
  gameSetCharacterMaxHp(100)

  gameRegisterTileEvent(9, 6, "showSign")

  local npc = helpers.Npc("assets/sprites/undead.json", 0, 3, 1)
  helpers.npcs:append(npc)
end

readSign = false

function showSign()
  if readSign then
    return
  end

  helpers.Dialog("You're reading a sign!")
end

function update()
  ticks = gameTicks()

  if math.fmod(ticks, UPDATE_INTERVAL) then
    helpers.npcs:forEach(function(npc)
      if npc.updateMethod then
        npc:update(ticks)
      end
      npc:jump(100)
    end)
  end
end

function collision(id1, id2)
  print("COLLISION, ID1", id1, "ID2", id2)
end
