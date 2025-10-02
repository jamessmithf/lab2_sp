puts "Hello!"
a = gets.to_f
b = gets.to_f
puts "Choose operation + - * /"
op = gets.chomp
result = case op
when "+"
  a + b
when "-"
  a - b
when "*"
  a * b
when "/"
  a / b
else
  "Unknown op"
end
puts "Result: #{result}"
