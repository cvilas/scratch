-- Initialize a sentence
text = "The quick fox jumps over the lazy dog."

-- Split string and return a list of words
words = string.gmatch(text, "%a+")

result = ""

-- Loop all words adding each word to the beginning of the final result string
for word in words do
  result = word.." "..result
end

-- Display the final inverted result sentence
print("Inverted sentence: "..result)
