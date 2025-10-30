#!/bin/sh

# Build:
umk ./upptst,./uppsrc Eon02 ~/.config/u++/theide/CLANG.bm -bsH1 +AI,SCREEN,BUILTIN_PORTAUDIO,BUILTIN_PORTMIDI,FLUIDLITE,USEMALLOC,DEBUG_RT bin/Eon02

# Run ide:
echo Executable compiled: bin/Eon02