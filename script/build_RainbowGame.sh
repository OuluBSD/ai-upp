#!/bin/sh

build_flags="-bsH1"

for arg in "$@"; do
	case "$arg" in
		-c|--clean)
			build_flags="-bsH1a" # Force rebuild
			;;
	esac
done

# Get SDL library flags
SDL_LIBS="-lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf"

# Build the Gdx package first (as a library)
umk ./uppsrc Gdx ~/.config/u++/theide/CLANG.bm "$build_flags" +DRAW,GLDRAW,IMAGE,SDRAW,RICHTEXT,XML,JSON,Z,PLATFORM_X11,GUI,APPS,HTTP,NET,CURL,X11,USEMALLOC,DEBUG_RT,DEBUG_FULL,WITH_SDL2 lib/libGdx.a

# Build the RainbowGame package
umk ./uppsrc RainbowGame ~/.config/u++/theide/CLANG.bm "$build_flags" +DRAW,GLDRAW,IMAGE,SDRAW,RICHTEXT,XML,JSON,Z,PLATFORM_X11,GUI,APPS,HTTP,NET,CURL,X11,USEMALLOC,DEBUG_RT,DEBUG_FULL,WITH_SDL2 bin/RainbowGame

echo "Build completed. Executable: bin/RainbowGame"
