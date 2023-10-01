-- Create a Lua script that initializes a variable with a number between 1 and 6.
-- The script should also generate a random number between 1 and 6 (simulating the
-- roll of a dice). The program should display a message if both numbers are equal
-- or different, meaning that we guessed the random dice roll correctly or not.

-- Initialize the random number generator seed based on the current time
math.randomseed(os.time())

-- Set an initial value
guess = 4

-- Generate a random number from 1 to 6 (simulating a dice roll)
dice = math.random(1, 6)

-- Check if the guess is equal to the dice result
if guess == dice then
  print("You guessed "..guess.." correctly! Dice value was "..dice)
else
  print("You guessed "..guess.." incorrectly! Dice value was "..dice)
end
