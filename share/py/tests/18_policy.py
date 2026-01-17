import os

print("Testing safe operation in sandbox...")
# os.path functions that don't check PolicyKit (like split, join)
print("Join: %s" % os.path.join("a", "b"))
print("Done.")
