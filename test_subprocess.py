import subprocess

print("Testing subprocess.run with string command:")
res = subprocess.run("echo hello world")
print("Return code:", res["returncode"])
print("Stdout:", res["stdout"].strip())

print("\nTesting subprocess.run with list command:")
res = subprocess.run(["echo", "hello", "from", "list"])
print("Return code:", res["returncode"])
print("Stdout:", res["stdout"].strip())

print("\nTesting failed command:")
res = subprocess.run("false")
print("Return code:", res["returncode"])
