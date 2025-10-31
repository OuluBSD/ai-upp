#!/bin/bash
# Compare uppsrc/Core and stdsrc/Core to find missing .h files

echo "Files in uppsrc/Core but not in stdsrc/Core:"
for file in /home/sblo/Dev/ai-upp/uppsrc/Core/*.h; do
    filename=$(basename "$file")
    if [ ! -f "/home/sblo/Dev/ai-upp/stdsrc/Core/$filename" ]; then
        echo "  - [ ] $filename"
    fi
done