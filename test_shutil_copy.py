import shutil
import os
print("Copying file...")
shutil.copy("test_shutil_only.py", "test_shutil_only_copy.py")
if os.path.exists("test_shutil_only_copy.py"):
    print("Success")
    os.remove("test_shutil_only_copy.py")
