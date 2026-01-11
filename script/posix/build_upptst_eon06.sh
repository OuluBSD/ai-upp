#!/bin/sh

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)

"$SCRIPT_DIR/build_upptst_eon_generic.sh" Eon06 "AI,SCREEN,SDL2,HAL,AUDIO,VIDEO,MEDIA,FBO,OGL,FFMPEG,OPENCV,CAMERA,X11,SYS_PORTAUDIO" "$@"
