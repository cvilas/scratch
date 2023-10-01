-- Create a script that initializes a string value with the email of a user.
-- The program should display if that variable contains a valid email.
-- The requirements for the value to be a valid email are:
-- 1. The email should contain an "@" character
-- 2. The email should contain a "." character
-- 3. The "@" should occur before the "." character
-- 4. The email should not contain any spaces.


-- Initialize a variable with an email address
email = "gustavo@pikuma.com"

-- Performs some checks to see if the email is valid
at_char = string.find(email, "%@")
dot_char = string.find(email, "%.")
space_char = string.find(email, " ")

-- We add a % in front of the character to avoid Lua replacing those characters
-- with special values.
-- For example "." is a pattern that represents *all characters*

-- Test if we have no spaces but found both @ and .
if (not space_char) and (at_char and dot_char) then
  -- Test if . appears before the @, since .find() return the position
  if at_char < dot_char then
    print("Valid email!")
  else
    print("Invalid. There is a . before the @ sign.")
  end
else
  print("Invalid email! No @ sign, or no . sign, or spaces found.")
end
