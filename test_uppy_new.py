import subprocess
import os

res = subprocess.run(["echo", "Hello", "from", "Uppy"])
print("Return code:", res["returncode"])

s = "test.py"
print("Ends with .py:", s.endswith(".py"))
print("Ends with .txt:", s.endswith(".txt"))

l = ["a", "b", "c"]
print("Joined:", ", ".join(l))
