import glob
print("Starting glob with import...")
files = glob.glob("*.py")
print("Glob finished. Found:", len(files))
