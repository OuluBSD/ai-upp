#!/bin/bash

# Build:
umk upptst,examples,tutorial,reference,uppsrc "${1}" $HOME/.config/u++/theide/CLANG.bm -bsH1 +NOGUI,USEMALLOC,DEBUG_FULL "bin/${1}"

# Run ide:
echo "Executable compiled: bin/${1}"
