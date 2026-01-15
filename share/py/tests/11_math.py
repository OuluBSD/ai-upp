import math

def assert_eq(a, b, msg, tolerance=0.0001):
    diff = a - b
    if diff < 0: diff = -diff
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

assert_eq(math.pi, 3.141592653589793, "pi")
assert_eq(math.e, 2.718281828459045, "e")
assert_eq(math.tau, 6.283185307179586, "tau")
if not math.isinf(math.inf): print("Error: inf is not inf"); exit(1)
if not math.isnan(math.nan): print("Error: nan is not nan"); exit(1)

print("Functions:")
assert_eq(math.sqrt(16), 4.0, "sqrt")
assert_eq(math.fabs(-10.5), 10.5, "fabs")
assert_eq(math.sin(math.pi/2), 1.0, "sin")
assert_eq(math.cos(math.pi), -1.0, "cos")
assert_eq(math.tan(math.pi/4), 1.0, "tan")

assert_eq(math.asin(1), math.pi/2, "asin")
assert_eq(math.acos(-1), math.pi, "acos")
assert_eq(math.atan(1), math.pi/4, "atan")
assert_eq(math.atan2(1, 1), math.pi/4, "atan2")

assert_eq(math.asinh(1), 0.8813735, "asinh")
assert_eq(math.acosh(1), 0.0, "acosh")
assert_eq(math.atanh(0.5), 0.549306, "atanh")

assert_eq(math.sinh(1), 1.175201, "sinh")
assert_eq(math.cosh(1), 1.543080, "cosh")
assert_eq(math.tanh(1), 0.761594, "tanh")

assert_eq(math.exp(1), math.e, "exp")
assert_eq(math.expm1(1), math.e - 1, "expm1")
assert_eq(math.log(math.e), 1.0, "log")
assert_eq(math.log10(100), 2.0, "log10")
assert_eq(math.log2(8), 3.0, "log2")
assert_eq(math.log1p(math.e - 1), 1.0, "log1p")

assert_eq(math.ceil(3.2), 4.0, "ceil")
assert_eq(math.floor(3.8), 3.0, "floor")
assert_eq(math.trunc(3.8), 3.0, "trunc")
assert_eq(math.trunc(-3.8), -3.0, "trunc negative")

assert_eq(math.gcd(48, 18), 6, "gcd")
assert_eq(math.factorial(5), 120, "factorial")
assert_eq(math.hypot(3, 4), 5.0, "hypot")
assert_eq(math.fsum([0.1, 0.1, 0.1]), 0.3, "fsum")

assert_eq(math.erf(0), 0.0, "erf")
assert_eq(math.erfc(0), 1.0, "erfc")
assert_eq(math.gamma(5), 24.0, "gamma") # (5-1)!

print("Checking dir(math):")
d = dir(math)
if 'sqrt' not in d: print("Error: sqrt not in dir(math)"); exit(1)
if 'pi' not in d: print("Error: pi not in dir(math)"); exit(1)
if '__name__' not in d: print("Error: __name__ not in dir(math)"); exit(1)

print("Math tests completed successfully")