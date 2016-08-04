local helpers = require("assets.scripts.helpers")

SPRITE_PATH = "assets/sprites/oryx_sprite.json"
HERO_TILE = 21
UPDATE_INTERVAL = 100
TILE_DIM = 24

function init()
  gameLoadMap("assets/maps/oryx16_overworld.json")
  gameLoadCharacter(SPRITE_PATH, HERO_TILE, 4, 3)
  gameSetCharacterMoveSpeed(4)
  gameSetCharacterMaxHp(100)

  enterTown()
end

function update()
  ticks = gameTicks()

  if math.fmod(ticks, UPDATE_INTERVAL) then
    helpers.npcs:forEach(function(npc)
      npc:update(ticks)
    end)
  end
end

-- Town

function registerTown()
  gameRegisterTileEvent(7, 8, "enterHouse")
  gameRegisterTileEvent(8, 8, "enterHouse")

  gameRegisterTileEvent(7, 15, "leaveTown")
  gameRegisterTileEvent(8, 15, "leaveTown")
end

function enterHouse()
  gameClearEvents()
  gameLoadMap("assets/maps/oryx16_house.json")
  gameSetCharacterPosition(3, 3)
  gameRegisterTileEvent(3, 4, "leaveHouse")
  gameRegisterTileEvent(4, 4, "leaveHouse")
end

function leaveHouse()
  gameClearEvents()
  helpers.popMap()
  registerTown()
  gameSetCharacterPosition(7, 9)
end

enteredOverworld = false

function leaveTown()
  gameClearEvents()
  helpers.popMap()
  registerOverworld()
  gameSetCharacterPosition(4, 3)

  if not enteredOverworld then
    enteredOverworld = true
    helpers.Dialog("Welcome to the overworld!")
    helpers.Dialog("second dialog")
  end
end

-- Overworld

function registerOverworld()
  gameRegisterTileEvent(4, 2, "enterTown")
end

function enterTown()
  gameLoadMap("assets/maps/oryx16_town.json")
  gameSetCharacterPosition(7, 14)

	local npc = helpers.Npc(SPRITE_PATH, 64, 1, 13)
  npc:setCallback("npcCallback")

  npc:setUpdate(helpers.randomMovement(TILE_DIM))

  helpers.npcs:append(npc)

  registerTown()
end

function npcCallback(npc_id)
  local dialog = helpers.Dialog("Are you new here?")
  dialog:addOptions({"No", "Yes"})
  dialog:setCallback("npcDialogCallback")
end

function npcDialogCallback(choice)
  if choice == 0 then
    helpers.Dialog("Welcome back!")
  else
    helpers.Dialog("Nice to meet you!")
  end
end
