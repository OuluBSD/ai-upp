#!/bin/sh

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)

"$SCRIPT_DIR/build_upptst_eon_generic.sh" Eon08 "GUI,AI,SCREEN,HAL,OGL,FFMPEG,OPENCV,AUDIO,VIDEO,MEDIA" "$@"
