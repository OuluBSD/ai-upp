import time

print("Time module tests:")
t1 = time.time()
print("Current time (Unix epoch):", t1)

print("Sleeping for 0.5 seconds...")
time.sleep(0.5)

t2 = time.time()
print("Time after sleep:", t2)
print("Difference:", t2 - t1)

print("Time tests completed successfully")
