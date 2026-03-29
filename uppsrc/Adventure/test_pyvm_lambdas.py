# PyVM Lambda Test Cases
# Test file for debugging lambda compilation

# Test 1: Simple lambda at top level (should work)
f1 = lambda x: x + 1

# Test 2: Lambda with no args (should work)
f2 = lambda: 42

# Test 3: Lambda in simple dict (may fail)
d1 = {
    "a": lambda x: x + 1
}

# Test 4: Lambda in dict with trailing comma (may fail)
d2 = {
    "a": lambda x: x + 1,
}

# Test 5: Lambda calling function (may fail)
def test_fn():
    return "hello"

f3 = lambda: test_fn()

# Test 6: Lambda in dict calling function (may fail)
d3 = {
    "fn": lambda: test_fn()
}

# Test 7: Lambda with arg calling function (may fail)
d4 = {
    "fn": lambda me: test_fn()
}

# Test 8: Multiple lambdas in dict (may fail)
d5 = {
    "a": lambda x: x + 1,
    "b": lambda y: y * 2,
}

# Test 9: Lambda after while loop (reported failure pattern)
def fn_with_loop():
    while True:
        x = 1
        break_time(10)

def fn_after_loop():
    pass

# Test 10: Lambda after while with multi-line condition (reported failure)
def fn_with_multiline():
    while True:
        if (x == 1
            and y == 2):
            do_something()
        break_time(10)

def fn_after_multiline():
    pass

# Test 11: Dict with lambda after multi-line function
d6 = {
    "enter": lambda me: test_fn(),
    "exit": lambda: None
}

print("All lambda tests loaded!")
