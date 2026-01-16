import threading

def thread_func(name):
    for i in range(5):
        print("Thread %s: %d" % (name, i))

print("Main    : starting threads")
threading.Thread(thread_func, ["A"])
threading.Thread(thread_func, ["B"])
print("Main    : all done")
