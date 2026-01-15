import os
import sys
import math
import time
import json

def test_math_expanded():
    print("Testing math expanded...")
    print("math.pow(2, 3) =", math.pow(2, 3))
    print("math.isinf(1e308 * 2) =", math.isinf(1e308 * 2))
    print("math.isnan(math.nan) =", math.isnan(math.nan))
    print("math.isfinite(1.23) =", math.isfinite(1.23))
    print("math.gcd(48, 18) =", math.gcd(48, 18))
    print("math.factorial(5) =", math.factorial(5))
    print("math.hypot(3, 4) =", math.hypot(3, 4))
    print("math.trunc(3.9) =", math.trunc(3.9))
    print("math.fsum([0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1]) =", math.fsum([0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1]))

def test_os_expanded():
    print("Testing os expanded...")
    print("os.name =", os.name)
    print("os.sep =", os.sep)
    print("os.path.isabs('/') =", os.path.isabs('/'))
    print("os.getlogin() =", os.getlogin())
    # print("os.uname() =", os.uname()) # Returns tuple

def test_sys_expanded():
    print("Testing sys expanded...")
    print("sys.byteorder =", sys.byteorder)
    print("sys.platform =", sys.platform)
    print("sys.path =", sys.path)
    print("sys.modules =", sys.modules)

def test_time_expanded():
    print("Testing time expanded...")
    start = time.perf_counter()
    time.sleep(0.1)
    end = time.perf_counter()
    print("time.perf_counter difference (should be ~0.1):", end - start)
    print("time.gmtime() =", time.gmtime())
    print("time.localtime() =", time.localtime())

def test_json_expanded():
    print("Testing json expanded...")
    data = {"a": 1, "b": [2, 3]}
    print("json.dumps(data, 4):")
    print(json.dumps(data, 4))

test_math_expanded()
test_os_expanded()
test_sys_expanded()
test_time_expanded()
test_json_expanded()
