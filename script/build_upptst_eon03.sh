#!/bin/sh

# Build:
umk ./upptst,./uppsrc Eon03 ~/.config/u++/theide/CLANG.bm -bsH1 +AI,SCREEN,BUILTIN_PORTAUDIO,BUILTIN_PORTMIDI,X11,OGL,USEMALLOC,DEBUG_RT,DEBUG_FULL bin/Eon03

# Run ide:
echo Executable compiled: bin/Eon03