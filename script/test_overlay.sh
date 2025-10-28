#!/bin/bash

# Build and run the MetaEnvironment overlay tests
echo "Building MetaEnvironment overlay tests..."

# Change to the project root directory
cd /home/sblo/Dev/Topside

# Build the test
./umk MetaEnvironment

if [ $? -eq 0 ]; then
    echo "Build successful. Running tests..."
    
    # Run the tests
    ./out/MetaEnvironment
    
    if [ $? -eq 0 ]; then
        echo "All tests passed!"
    else
        echo "Tests failed!"
        exit 1
    fi
else
    echo "Build failed!"
    exit 1
fi