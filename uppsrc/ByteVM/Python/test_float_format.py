delta = 12.345

assert "%.1f BB" % delta == "12.3 BB"
assert "%.2f BB" % delta == "12.35 BB"
assert "%g BB" % delta == "12.345 BB"
