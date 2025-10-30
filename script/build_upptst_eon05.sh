#!/bin/sh

# Build:
umk ./upptst,./uppsrc Eon05 ~/.config/u++/theide/CLANG.bm -bsH1 +AI,SCREEN,SDL2,OGL,FFMPEG,OPENCV,USEMALLOC,SYS_PORTAUDIO,DEBUG_RT bin/Eon05

# Run ide:
echo Executable compiled: bin/Eon05