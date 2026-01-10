#!/bin/sh

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)

"$SCRIPT_DIR/build_upptst_eon_generic.sh" Eon05 "AI,SCREEN,SDL2,HAL,AUDIO,VIDEO,FBO,OGL,FFMPEG,OPENCV,CAMERA,SYS_PORTAUDIO" "$@"
