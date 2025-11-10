#!/bin/bash

# Build:
umk upptst,examples,tutorial,reference,uppsrc "${1}" $HOME/.config/u++/theide/CLANG.bm -bsH1 +GUI,USEMALLOC,DEBUG_FULL "bin/${1}"

# Check if build was successful (umk returned exit code 0)
if [ $? -eq 0 ]; then
    # Run ide:
    echo "Executable compiled: bin/${1}"
fi
