#!/bin/bash

# Build script for GuboDemo
echo "Building GuboDemo..."

# Navigate to the project root
cd /common/active/sblo/Dev/ai-upp

# Build the GuboDemo package
echo "Building GuboDemo package..."
if [ -f "uppsrc/GuboDemo/GuboDemo.upp" ]; then
    echo "Found GuboDemo.upp package file"
else
    echo "Creating GuboDemo.upp package file..."
    cat > uppsrc/GuboDemo/GuboDemo.upp << EOF
<?xml version="1.0" encoding="UTF-8"?>
<Project>
    <Description>Gubo Demo - 3D GUI demonstration</Description>
    <GUID>{E48F4702-7272-4A34-A122-8EF02C6EC4A4}</GUID>
    <Package>GuboDemo</Package>
    <Target>GuboDemo</Target>
    <Type>Console</Type>
    <Include>
        <File>GuboDemo.h</File>
        <File>GuboDemo.cpp</File>
    </Include>
    <Library>
        <Pkg>Draw</Pkg>
        <Pkg>Cuboid</Pkg>
        <Pkg>GuboCore</Pkg>
        <Pkg>GuboLib</Pkg>
        <Pkg>Core</Pkg>
        <Pkg>CtrlCore</Pkg>
        <Pkg>CtrlLib</Pkg>
    </Library>
</Project>
EOF
fi

# Build the project
echo "Attempting to build GuboDemo..."
./bin/ide -n -g -w -q "upptst/GuboDemo/GuboDemo.upp"

if [ $? -eq 0 ]; then
    echo "Build successful!"
else
    echo "Build failed. Attempting to build with UPP Studio tools..."
    # Alternative build method using uppm
    if command -v uppm &> /dev/null; then
        uppm build upptst/GuboDemo/GuboDemo.upp
    else
        echo "Could not find build tools. Please ensure UPP Studio is installed."
    fi
fi