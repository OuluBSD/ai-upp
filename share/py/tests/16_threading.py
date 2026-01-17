import threading
import time

def thread_func(name, delay):
    print("Thread %s: starting" % name)
    time.sleep(delay)
    print("Thread %s: finishing" % name)

print("Main    : before creating thread")
x = threading.Thread(thread_func, ["A", 0.1])
y = threading.Thread(thread_func, ["B", 0.2])
print("Main    : all done")
