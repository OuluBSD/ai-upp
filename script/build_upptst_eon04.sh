#!/bin/sh

# Build:
umk ./upptst,./uppsrc Eon04 ~/.config/u++/theide/CLANG.bm -bsH1 +AI,SCREEN,SDL2,OGL,FFMPEG,OPENCV,USEMALLOC,SYS_PORTAUDIO,DEBUG_RT,DEBUG_FULL bin/Eon04

# Run ide:
echo Executable compiled: bin/Eon04