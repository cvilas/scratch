function factorial(n)
  if n == 1 then
   return 1
  else
   return n * factorial(n - 1)
  end
end

print("Factorial " .. factorial(3))
