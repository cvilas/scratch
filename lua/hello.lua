print("Hello, World!")

-- Data types:
-- nil
-- number
-- string
-- function
-- CFunction
-- userdata
-- table

name = "Miles Davies"
salary = 500000.00
new_salary = salary * 1.15 
print("The new salary is " .. new_salary .. " dollars")

A = 1000
p = 5
n = 3
result = A * ( 1 + p/100)^n
print("Money " .. result)

if A >= 1000 then
  print("Good result")
else
  print("Try again")
end

-- comparison operators: ==, ~=, >, <
-- logical operators: and, or, not

-- string manipulation
color = "#cd10e3"
color = string.upper(string.gsub(color, "#", ""))
print("Color " .. color)
print("Length of the string Color: " .. #color)

email = "vilas@gmail.com"
if string.find(email, "@") then
  print("Email is valid")
else
  print("Email is invalid")
end

-- multiline string
sea_levels = [[
2000, 5.1
2001, 5.2
2002, 5.3
]]

-- Lua indices start at 1, not 0

local file = io.open("sea_level.csv", "r") -- open file to read only
local text = file:read("all") -- read everything in the file
file:close()
print(text)

-- command line args passed as arg[n]

-- multiple variable assignment
x, y = 1, 2
x, y = y, x -- swaps values

print("Enter a number")
user_option = io.read("*n");
print("You entered number " .. user_option)

quiz_scores = {
mark = 88.7,
john = 99.4,
mary = 67.7
}
for k, v in pairs(quiz_scores) do
  print(k.. " got " ..v.. " score")
end

arr = {1, 4, 6, 9, -1}
for i, v in ipairs(arr) do
  print("index " ..i.. " value " ..v)
end
