import math

def assert_val(name, actual, expected):
    epsilon = 1e-9
    if abs(actual - expected) > epsilon:
        print("FAIL: " + name)
        return False
    print("PASS: " + name)
    return True

def assert_bool(name, actual, expected):
    if actual != expected:
        print("FAIL: " + name)
        return False
    print("PASS: " + name)
    return True

print("Testing additional math functions...")

success = True

# cbrt
success = assert_val("cbrt(8)", math.cbrt(8), 2.0) and success
success = assert_val("cbrt(-27)", math.cbrt(-27), -3.0) and success

# exp2
success = assert_val("exp2(3)", math.exp2(3), 8.0) and success
success = assert_val("exp2(-1)", math.exp2(-1), 0.5) and success

# isclose
success = assert_bool("isclose(1.0, 1.0000000001)", math.isclose(1.0, 1.0000000001), True) and success
success = assert_bool("isclose(1.0, 1.1)", math.isclose(1.0, 1.1), False) and success
success = assert_bool("isclose(1.0, 1.1, rel_tol=0.2)", math.isclose(1.0, 1.1, 0.2), True) and success

# isqrt
success = assert_bool("isqrt(10)", math.isqrt(10), 3) and success
success = assert_bool("isqrt(16)", math.isqrt(16), 4) and success
success = assert_bool("isqrt(15)", math.isqrt(15), 3) and success
success = assert_bool("isqrt(10**12)", math.isqrt(1000000000000), 1000000) and success
success = assert_bool("isqrt(10**12 - 1)", math.isqrt(999999999999), 999999) and success

# lcm
success = assert_bool("lcm(12, 18)", math.lcm(12, 18), 36) and success
success = assert_bool("lcm(5, 7, 11)", math.lcm(5, 7, 11), 385) and success

# ldexp
success = assert_val("ldexp(3, 2)", math.ldexp(3, 2), 12.0) and success

# frexp
res_frexp = math.frexp(12.0)
success = assert_val("frexp(12.0) mantissa", res_frexp[0], 0.75) and success
success = assert_bool("frexp(12.0) exponent", res_frexp[1], 4) and success

# modf
res_modf = math.modf(3.5)
success = assert_val("modf(3.5) fractional", res_modf[0], 0.5) and success
success = assert_val("modf(3.5) integer", res_modf[1], 3.0) and success

# ulp
success = assert_val("ulp(1.0)", math.ulp(1.0), 2.220446049250313e-16) and success

# comb
success = assert_bool("comb(5, 2)", math.comb(5, 2), 10) and success
success = assert_bool("comb(10, 3)", math.comb(10, 3), 120) and success
success = assert_bool("comb(20, 10)", math.comb(20, 10), 184756) and success

# perm
success = assert_bool("perm(5, 2)", math.perm(5, 2), 20) and success
success = assert_bool("perm(5)", math.perm(5), 120) and success
success = assert_bool("perm(10, 3)", math.perm(10, 3), 720) and success

# dist
success = assert_val("dist([0,0], [3,4])", math.dist([0,0], [3,4]), 5.0) and success

# prod
success = assert_val("prod([1, 2, 3, 4])", math.prod([1, 2, 3, 4]), 24.0) and success
success = assert_val("prod([1, 2, 3], start=10)", math.prod([1, 2, 3], 10), 60.0) and success

# sumprod
success = assert_val("sumprod([1, 2, 3], [4, 5, 6])", math.sumprod([1, 2, 3], [4, 5, 6]), 32.0) and success

if success:
    print("\nAdditional math tests completed successfully")
else:
    print("\nSome tests FAILED")
    exit(1)
