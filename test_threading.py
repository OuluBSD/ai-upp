import threading
import time

def thread_func(name, delay):
    print("Thread %s: starting" % name)
    time.sleep(delay)
    print("Thread %s: finishing" % name)

print("Main    : before creating thread")
x = threading.Thread(thread_func, ["A", 1])
y = threading.Thread(thread_func, ["B", 2])
print("Main    : before running threads")
# In Uppy, creating the thread already starts it for now (simplified)
print("Main    : all done")
