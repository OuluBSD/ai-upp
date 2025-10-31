#!/bin/sh

# Build:
umk ./upptst,./uppsrc Eon00 ~/.config/u++/theide/CLANG.bm -bsH1 +AI,DEBUG_RT,DEBUG_VFS,USEMALLOC,DEBUG_FULL bin/Eon00

# Copy data files to bin directory:
cp upptst/Eon00/*.eon bin/

# Run ide:
echo Executable compiled: bin/Eon00
