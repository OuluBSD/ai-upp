import subprocess
import os
import sys

def run_aria(args):
    cmd = ["bin/AriaCLI"] + args
    print(">> Running: " + " ".join(cmd))
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.stdout:
        print(result.stdout)
    if result.stderr:
        print("ERROR: " + result.stderr, file=sys.stderr)
    return result.returncode == 0

def main():
    print("=== Aria News Extraction & Dump Python Script ===")
    
    # 1. Ensure binary exists
    if not os.path.exists("bin/AriaCLI"):
        print("AriaCLI not found. Building...")
        subprocess.run(["script/build.py", "AriaCLI"], check=True)

    # 2. Clear old data
    print("\n[Step 1] Clearing news database...")
    run_aria(["news", "clear"])

    # 3. Perform scraping
    print("\n[Step 2] Triggering all news providers...")
    if not run_aria(["news", "scrape"]):
        print("Scrape failed.")
        sys.exit(1)

    # 4. Dump everything
    print("\n[Step 3] Final Data Dump (All Sources):")
    print("=" * 60)
    run_aria(["news", "list", "all"])
    print("=" * 60)
    
    print("\nTest completed successfully.")

if __name__ == "__main__":
    main()