local helpers = require("assets.scripts.helpers")

SPRITE_PATH = "assets/sprites/oryx_sprite.json"
HERO_TILE = 21
UPDATE_INTERVAL = 100
TILE_DIM = 16

function init()
  gameLoadMap("assets/maps/metroidvania.json")
  gameLoadCharacter(SPRITE_PATH, HERO_TILE, 1, 6)
  gameSetCharacterMoveSpeed(4)
  gameSetCharacterMaxHp(100)

  gameRegisterTileEvent(9, 6, "showSign")
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
      npc:update(ticks)
    end)
  end
end
