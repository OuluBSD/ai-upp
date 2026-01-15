import math

def assert_eq(a, b, msg, tolerance):
    diff = a - b
    if diff < 0:
        diff = -diff
    if diff > tolerance:
        print("Assertion failed:", msg, "|", a, "!=", b)
        exit(1)

print("Math module tests:")
print("Constants:")
print("pi:", math.pi)
print("e:", math.e)
print("tau:", math.tau)
print("inf:", math.inf)
print("nan:", math.nan)

assert_eq(math.pi, 3.141592653589793, "pi", 0.0001)
assert_eq(math.e, 2.718281828459045, "e", 0.0001)
assert_eq(math.tau, 6.283185307179586, "tau", 0.0001)
if not math.isinf(math.inf):
    print("Error: inf is not inf")
    exit(1)
if not math.isnan(math.nan):
    print("Error: nan is not nan")
    exit(1)

print("Functions:")
assert_eq(math.sqrt(16), 4.0, "sqrt", 0.0001)
assert_eq(math.fabs(-10.5), 10.5, "fabs", 0.0001)
assert_eq(math.sin(math.pi/2), 1.0, "sin", 0.0001)
assert_eq(math.cos(math.pi), -1.0, "cos", 0.0001)
assert_eq(math.tan(math.pi/4), 1.0, "tan", 0.0001)

assert_eq(math.asin(1), math.pi/2, "asin", 0.0001)
assert_eq(math.acos(-1), math.pi, "acos", 0.0001)
assert_eq(math.atan(1), math.pi/4, "atan", 0.0001)
assert_eq(math.atan2(1, 1), math.pi/4, "atan2", 0.0001)

assert_eq(math.asinh(1), 0.8813735, "asinh", 0.0001)
assert_eq(math.acosh(1), 0.0, "acosh", 0.0001)
assert_eq(math.atanh(0.5), 0.549306, "atanh", 0.0001)

assert_eq(math.sinh(1), 1.175201, "sinh", 0.0001)
assert_eq(math.cosh(1), 1.543080, "cosh", 0.0001)
assert_eq(math.tanh(1), 0.761594, "tanh", 0.0001)

assert_eq(math.exp(1), math.e, "exp", 0.0001)
assert_eq(math.expm1(1), math.e - 1, "expm1", 0.0001)
assert_eq(math.log(math.e), 1.0, "log", 0.0001)
assert_eq(math.log10(100), 2.0, "log10", 0.0001)
assert_eq(math.log2(8), 3.0, "log2", 0.0001)
assert_eq(math.log1p(math.e - 1), 1.0, "log1p", 0.0001)

assert_eq(math.ceil(3.2), 4.0, "ceil", 0.0001)
assert_eq(math.floor(3.8), 3.0, "floor", 0.0001)
assert_eq(math.trunc(3.8), 3.0, "trunc", 0.0001)
assert_eq(math.trunc(-3.8), -3.0, "trunc negative", 0.0001)

assert_eq(math.gcd(48, 18), 6, "gcd", 0.0001)
assert_eq(math.factorial(5), 120, "factorial", 0.0001)
assert_eq(math.hypot(3, 4), 5.0, "hypot", 0.0001)
assert_eq(math.fsum([0.1, 0.1, 0.1]), 0.3, "fsum", 0.0001)

assert_eq(math.erf(0), 0.0, "erf", 0.0001)
assert_eq(math.erfc(0), 1.0, "erfc", 0.0001)
assert_eq(math.gamma(5), 24.0, "gamma", 0.0001)

print("Checking dir(math):")
d = dir(math)
# Temporarily removed 'in' checks until compiler supports it
print("dir(math) count:", len(d))

print("Math tests completed successfully")
