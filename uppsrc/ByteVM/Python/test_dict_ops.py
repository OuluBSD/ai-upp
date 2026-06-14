d = {
    "a": 1,
    "b": 2,
}

assert d.get("a") == 1
assert d.get("missing") is None
assert d.get("missing", 7) == 7

keys = set()
for k in d:
    keys.add(k)
assert len(keys) == 2

items = d.items()
assert len(items) == 2
seen = 0
for k, v in items:
    if d.get(k) == v:
        seen += 1
assert seen == 2

assert len(d.keys()) == 2
assert len(d.values()) == 2
