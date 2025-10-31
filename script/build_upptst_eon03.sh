#!/bin/sh

build_flags="-bsH1"

for arg in "$@"; do
	case "$arg" in
		-c|--clean)
			build_flags="-bsH1a"
			;;
	esac
done

# Build:
umk ./upptst,./uppsrc Eon03 ~/.config/u++/theide/CLANG.bm "$build_flags" +AI,SCREEN,BUILTIN_PORTAUDIO,BUILTIN_PORTMIDI,X11,OGL,USEMALLOC,DEBUG_RT,DEBUG_FULL bin/Eon03

# Run ide:
echo Executable compiled: bin/Eon03
