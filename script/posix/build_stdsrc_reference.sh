#!/bin/bash

# Build:
umk ./reference,./stdsrc "${1}" ~/.config/u++/ide/CLANG.bm -bsH1 +GUI,USEMALLOC "bin/${1}"

# Run ide:
echo "Executable compiled: bin/${1}"
