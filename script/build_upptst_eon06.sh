#!/bin/sh

# Build:
umk ./upptst,./uppsrc Eon06 ~/.config/u++/theide/CLANG.bm -bsH1 +AI,SCREEN,SDL2,OGL,FFMPEG,OPENCV,USEMALLOC,SYS_PORTAUDIO,DEBUG_RT bin/Eon06

# Run ide:
echo Executable compiled: bin/Eon06