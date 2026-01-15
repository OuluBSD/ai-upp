print("Iterators")
l = [1, 2, 3]
it = iter(l)
print(next(it))
print(next(it))
print(next(it))
# print(next(it)) # Should return StopIteration which is not printable nicely yet

print("Iterating with range")
r = range(2)
it2 = iter(r)
print(next(it2))
print(next(it2))
