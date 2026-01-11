#!/bin/sh

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)

"$SCRIPT_DIR/build_upptst_eon_generic.sh" Eon05 "AI,SCREEN,SDL2,HAL,AUDIO,VIDEO,MEDIA,VOLUMETRIC,FBO,OGL,FFMPEG,OPENCV,CAMERA,X11,SYS_PORTAUDIO" "$@"
