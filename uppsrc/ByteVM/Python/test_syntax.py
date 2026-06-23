s = {1, 2, 3}
assert len(s) == 3
assert 2 in s

label = f"set={len(s)}"
assert label == "set=3"

value = 1 | 2 & 4 ^ 8
assert value == 9
assert 1 << 3 + 1 == 16
assert 16 >> 2 + 0 == 4

picked = [x for x in [1, 2, 3] if (n := x) > 1]
assert picked == [2, 3]
assert n == 3

filtered = {x * 2 for x in [1, 2, 3] if (m := x) > 1}
assert len(filtered) == 2
assert 4 in filtered
assert 6 in filtered
assert m == 3

mapping = {x: x * 2 for x in [1, 2]}
assert mapping.get(2) == 4
