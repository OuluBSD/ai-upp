#!/usr/bin/env bash
name="Eon01"

# Build:
umk ./upptst,./uppsrc $name ~/.config/u++/theide/CLANG.bm -bsH1 +AI,SCREEN,DEBUG_RT,DEBUG_VFS,USEMALLOC bin/$name

# Run ide:
echo Executable compiled: .bin/$name
