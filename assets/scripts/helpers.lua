module = {}

local function round(n)
  return math.floor(n + 0.5)
end
module.round = round

math.randomseed(os.time())

local function displayDialogChoice(message, options)
  uid = gameShowDialog(message)
  for i=1, #options do
    gameAddDialogOption(uid, options[i])
  end
  return uid
end
module.displayDialogChoice = displayDialogChoice


Npc = {}
Npc.__index = Npc

setmetatable(Npc, {
  __call = function (cls, ...)
    return cls.create(...)
  end,
})

function Npc.create(sprite_path, tile, x, y)
	local self = setmetatable({}, Npc)
	self.sprite_path = sprite_path
	self.tile = tile
	self.x = x
	self.y = y
	self.id = gameAddNpc(sprite_path, tile, x, y)
  self.update = nil
  self.moving = false
  self.tweenX = 0
  self.tweenY = 0
	return self
end

function Npc:setCallback(callback)
	gameSetNpcCallback(self.id, callback)
end

function Npc:move(x, y)
  gameMoveNpc(self.id, x, y)
end

function Npc:setMaxHp(hp)
  gameSetNpcMaxHp(self.id, hp)
end

function Npc:damage(amount)
  gameDamageNpc(self.id, amount)
end

function Npc:heal(amount)
  gameHealNpc(self.id, amount)
end

function Npc:setUpdate(update)
  self.updateMethod = update
end

function Npc:update(ticks)
  self.updateMethod(self, ticks)
end
module.Npc = Npc


-- Movement functions
local function randomMovement(tileSize)
  return function(npc, ticks)
    if npc.moving then
      if npc.tweenX > 0 then
        npc.tweenX = npc.tweenX - 1
        npc:move(1, 0)
      elseif npc.tweenX < 0 then
        npc.tweenX = npc.tweenX + 1
        npc:move(-1, 0)
      elseif npc.tweenY > 0 then
        npc.tweenY = npc.tweenY - 1
        npc:move(0, 1)
      elseif npc.tweenY < 0 then
        npc.tweenY = npc.tweenY + 1
        npc:move(0, -1)
      end

      if npc.tweenX == 0 and npc.tweenY == 0 then
        npc.moving = false
      end
    else
      if math.random() > .99 then
        npc.moving = true
        move = tileSize
        if math.random() > .5 then
          move = move * -1
        end
        if math.random() > .5 then
          npc.tweenX = move
        else
          npc.tweenY = move
        end
      end
    end
  end
end
module.randomMovement = randomMovement


Dialog = {}
Dialog.__index = Dialog

setmetatable(Dialog, {
  __call = function (cls, ...)
    return cls.create(...)
  end,
})

function Dialog.create(message)
  local self = setmetatable({}, Dialog)
  self.message = message
  self.id = gameShowDialog(message)
  return self
end

function Dialog:addOptions(options)
  for i=1, #options do
    gameAddDialogOption(self.id, options[i])
  end
end

function Dialog:setCallback(callback)
  gameSetDialogCallback(self.id, callback)
end
module.Dialog = Dialog


List = {}
List.__index = List

setmetatable(List, {
  __call = function (cls, ...)
    return cls.create(...)
  end,
})

function List.create()
  local self = setmetatable({}, List)
  self:clear()
  return self
end

function List:append(item)
  self.lst[self.len + 1] = item
  self.len = self.len + 1
end

function List:at(i)
  return self.lst[i]
end

function List:forEach(func)
  for k, v in pairs(self.lst) do
    func(v)
  end
end

function List:length()
  return self.len
end

function List:clear()
  self.lst = {}
  self.len = 0
end
module.List = List


module.npcs = List()

local function popMap()
  gamePopMap()

  module.npcs:clear()
end
module.popMap = popMap


return module
