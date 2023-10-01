--- Demonstrates how an event handler is incorporated into an object

TouchEvent = {
  onEvent = ""
}

function TouchEvent:new()
  local ev = {}
  setmetatable(ev, self)
  self.__index = self
  return ev
end

function TouchEvent:Connect(func)
  self.onEvent = func
end

GameObject = {
  Touch = TouchEvent:new()
}

function GameObject:new()
  local obj = {}
  setmetatable(obj, self)
  self.__index = self;
  return obj
end

function GameObject:touch() 
  self.Touch.onEvent()
end

-- A callback to trigger on touch event
function on_touch()
  print("touched")
end

game_object = GameObject:new()

game_object.Touch:Connect(on_touch) -- pass callback to Touch event
game_object:touch()