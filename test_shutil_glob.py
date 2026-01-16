import shutil
import glob
import os

print("Testing glob.glob:")
files = glob.glob("*.py")
print("Python files in current dir:", files)

found = False
for f in files:
    if os.path.basename(f) == "test_shutil_glob.py":
        found = True
        break
if found:
    print("Found test_shutil_glob.py in glob results")

print("\nTesting shutil.copy:")
shutil.copy("test_shutil_glob.py", "test_shutil_glob_copy.py")
if os.path.exists("test_shutil_glob_copy.py"):
    print("test_shutil_glob_copy.py created successfully")
    os.remove("test_shutil_glob_copy.py")
    print("test_shutil_glob_copy.py removed")
