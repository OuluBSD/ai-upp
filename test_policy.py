import os
import subprocess

print("Testing os.listdir in sandbox...")
os.listdir(".")
print("SUCCESS (SHOULD NOT HAPPEN in sandbox)")