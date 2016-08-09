helpers = require("assets.scripts.helpers")

SPRITE_PATH = "assets/sprites/hero.json"
HERO_TILE = 0
UPDATE_INTERVAL = 100
TILE_DIM = 16

function init()
  if gameInitialized() then
    return
  end
  gameLoadMap("assets/maps/main.json")
  gameLoadCharacter(SPRITE_PATH, HERO_TILE, 1, 6)
  gameSetCharacterMoveSpeed(200, 100)
  gameSetCharacterMaxHp(100)

  gameRegisterTileEvent(9, 6, "showSign")

  local npc = helpers.Npc("assets/sprites/undead.json", 0, 3, 1)
  helpers.npcs:append(npc)

  id = gameAddItem("assets/sprites/item.json", 2, 14, 0)
  print(id)

  --d = helpers.Dialog("Here's a really really really long message that will scroll yay messages wooo so this one time at band camp this really cool thing happened and then the story ended")
  --d:addOptions({"No", "Yes"})
end

readSign = false

function resetSign()
  readSign = false
end

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
