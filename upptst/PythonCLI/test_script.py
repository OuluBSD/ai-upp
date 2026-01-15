# Test script for Uppy interpreter
print("Hello from Uppy!")
x = 5
y = 10
result = x + y
print("Result:", result)

# Test a loop
for i in range(3):
    print("Loop iteration:", i)

# Test a function
def greet(name):
    return "Hello, " + name + "!"

message = greet("Uppy User")
print(message)

# Test edge cases that might cause issues
print("Testing edge cases...")
empty_list = []
print("Empty list:", empty_list)
nested_list = [1, [2, 3], 4]
print("Nested list:", nested_list)