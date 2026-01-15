import os
import sys

print("System Information:")
print("Platform:", sys.platform)
print("Version:", sys.version)
print("Executable:", sys.executable())

print("PID:", os.getpid())

# Environment
print("HOME:", os.getenv("HOME", "not found"))
os.putenv("UPPY_TEST", "123")
print("UPPY_TEST:", os.getenv("UPPY_TEST"))
# Note: os.environ is a snapshot at startup in our implementation
print("Environ count:", len(os.environ) > 0)

cwd = os.getcwd()
print("Current directory:", cwd)

# Test os.path
print("os.path.exists('.'):", os.path.exists('.'))
print("os.path.isdir('.'):", os.path.isdir('.'))
print("os.path.isfile('.'):", os.path.isfile('.'))
print("os.path.abspath('.'):", os.path.abspath('.'))

path = "/a/b/c.txt"
print("os.path.basename:", os.path.basename(path))
print("os.path.dirname:", os.path.dirname(path))
print("os.path.split:", os.path.split(path))
print("os.path.splitext:", os.path.splitext(path))

test_dir = "test_dir_python"
if os.path.exists(test_dir):
    print("Removing existing test directory")
    os.rmdir(test_dir)

os.mkdir(test_dir)
print("Created directory:", test_dir)

# Test rename
new_test_dir = "test_dir_python_renamed"
if os.path.exists(new_test_dir):
    os.rmdir(new_test_dir)
os.rename(test_dir, new_test_dir)
print("Renamed directory exists:", os.path.exists(new_test_dir))

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

os.rmdir(new_test_dir)
print("Removed directory:", new_test_dir)

print("OS and SYS tests completed successfully")
