#!/bin/bash

# Build script for QwenManager package
cd "$(dirname "$0")"/..

echo "Building QwenManager package..."

# Check if theide exists
if [ ! -f ./bin/theide ]; then
    echo "Error: theide executable not found in bin/ directory"
    exit 1
fi

echo "Using theide to build QwenManager..."
./bin/theide uppsrc/QwenManager

if [ $? -eq 0 ]; then
    echo "Build completed successfully"
else
    echo "Build failed"
    exit 1
fi