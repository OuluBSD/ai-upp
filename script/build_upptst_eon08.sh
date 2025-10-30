#!/bin/sh

# Build:
umk ./upptst,./uppsrc Eon08 ~/.config/u++/theide/CLANG.bm -bsH1 +AI,SCREEN,SDL2,OGL,FFMPEG,OPENCV,X11,USEMALLOC,DEBUG_RT bin/Eon08

# Run ide:
echo Executable compiled: bin/Eon08