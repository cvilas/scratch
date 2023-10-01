Fighter = {
  name = "",
  attack_name = "",
  country = "",
  difficulty = "",
}

function Fighter:new(t)
  local obj = t or {}
  setmetatable(obj, self)
  self.__index = self;
  return obj
end

function Fighter:light_punch()
  print(self.name .. " performed a light punch")
end

function Fighter:strong_punch()
  print(self.name .. " performed a strong punch")
end

function Fighter:light_kick()
  print(self.name .. " performed a light kick")
end

function Fighter:heavy_kick()
  print(self.name .. " performed a heavy kick")
end

function Fighter:attack_move()
  print(self.name .. " performed an attack move")
end

--------------------------------
-- Create two fighter objects
--------------------------------
blanka = Fighter:new({
  name = "Blanka",
  country = "Brazil",
  attack_name = "Electric shock",
  difficulty = 3
})
print("Object "..blanka.name.." was created.")

chun_li = Fighter:new({
  name = "Chun Li",
  country = "China",
  attack_name = "Lightning kick",
  difficulty = 5
})
print("Object "..chun_li.name.." was created.")

--------------------------------
-- Call object methods
--------------------------------
blanka:light_punch()
blanka:heavy_kick()
blanka:attack_move()

chun_li:light_punch()
chun_li:heavy_kick()
chun_li:attack_move()
