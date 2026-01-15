print("Functions")
def add(a, b):
    return a + b

print(add(5, 7))

def fib(n):
    if n <= 1:
        return n
    return fib(n-1) + fib(n-2)

print("Fibonacci")
for i in range(10):
    print(fib(i))
