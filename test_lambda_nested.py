# Test lambda in nested dictionary
d = {
    "a": 1,
    "b": {
        "c": lambda x: x + 1
    },
    "d": lambda y: y * 2
}
print(d)
