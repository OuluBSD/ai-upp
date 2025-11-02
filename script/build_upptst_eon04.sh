#!/bin/sh

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)

"$SCRIPT_DIR/build_upptst_eon_generic.sh" Eon04 "AI,SCREEN,SDL2,OGL,FFMPEG,OPENCV,SYS_PORTAUDIO" "$@"
