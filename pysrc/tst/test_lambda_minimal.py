# Minimal lambda test for PyVM debugging

# Test 1: Simple lambda at top level
f = lambda x: x + 1
print("Test 1 passed: simple lambda")

# Test 2: Lambda in dictionary
d = {
    "a": lambda x: x + 1,
    "b": 42
}
print("Test 2 passed: lambda in dict")

# Test 3: Lambda calling function
def test_fn():
    return "hello"

g = lambda: test_fn()
print("Test 3 passed: lambda calling function")

# Test 4: Lambda in dict calling function
h = {
    "fn": lambda me: test_fn()
}
print("Test 4 passed: lambda in dict calling function")

print("All lambda tests passed!")
