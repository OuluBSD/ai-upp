import subprocess
import sys

def main():
    print("Building...")
    build_cmd = ["python3", "script/build.py", "-j", "12", "-mc", "1", "StereoCalibrationTool"]
    if subprocess.call(build_cmd) != 0:
        print("Build failed!")
        sys.exit(1)
    print("Build SUCCESS")

if __name__ == "__main__":
    main()
