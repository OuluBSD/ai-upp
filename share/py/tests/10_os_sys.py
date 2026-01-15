import os
import sys

print("System Information:")
print("Platform:", sys.platform)
print("Version:", sys.version)

cwd = os.getcwd()
print("Current directory:", cwd)

# Test os.path
print("os.path.exists('.'):", os.path.exists('.'))
print("os.path.isdir('.'):", os.path.isdir('.'))
print("os.path.isfile('.'):", os.path.isfile('.'))

test_dir = "test_dir_python"
if os.path.exists(test_dir):
    print("Removing existing test directory")
    os.rmdir(test_dir)

os.mkdir(test_dir)
print("Created directory:", test_dir)

# Test listdir
files = os.listdir(".")
print("Files in current directory count:", len(files) > 0)
found_test_dir = False
for f in files:
    if f == test_dir:
        found_test_dir = True
print("test_dir found in listdir:", found_test_dir)

test_file = os.path.join(test_dir, "test.txt")
# Create a dummy file if we had file IO, but we can't yet easily.
# But we can check listdir of the new dir
dir_files = os.listdir(test_dir)
print("Files in test_dir:", len(dir_files))

# Test chdir
os.chdir(test_dir)
print("Changed directory to:", os.getcwd())
os.chdir("..")
print("Changed back to:", os.getcwd())

os.rmdir(test_dir)
print("Removed directory:", test_dir)
print("os.path.exists(test_dir):", os.path.exists(test_dir))

print("OS and SYS tests completed successfully")
