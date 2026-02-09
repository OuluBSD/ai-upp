#!/bin/bash
set -e

# Ensure we are in the repo root
if [ ! -d "bin" ]; then
    echo "Error: This script must be run from the repository root."
    exit 1
fi

if [ ! -f "bin/MaestroHub" ]; then
    echo "Error: bin/MaestroHub not found. Please build it first."
    exit 1
fi

echo "Building crash test binary..."
g++ -g share/py/maestro/tests/crash.cpp -o crash

echo "Running MaestroHub automation test..."
# We use '|| true' to ensure cleanup happens even if test fails, 
# but we capture the exit code to return it later.
set +e
bin/MaestroHub share/py/maestro/tests/test_gdb_crash.py
EXIT_CODE=$?
set -e

echo "Cleaning up..."
rm -f crash

if [ $EXIT_CODE -eq 0 ]; then
    echo "Test passed."
else
    echo "Test failed with exit code $EXIT_CODE."
    exit $EXIT_CODE
fi
