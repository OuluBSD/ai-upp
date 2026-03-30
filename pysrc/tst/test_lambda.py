# Minimal lambda test for PyVM

def test_fn():
    # Simple lambda in dict
    d = {
        "a": lambda x: x + 1,
        "b": lambda: print("hello")
    }
    return d

# Lambda at top level
f = lambda x: x * 2

# Lambda with function call
g = lambda me: test_fn()

print("Lambda test loaded")
