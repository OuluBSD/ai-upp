#!/bin/sh

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)

"$SCRIPT_DIR/build_upptst_eon_generic.sh" Eon03 "AI,SCREEN,AUDIO,VIDEO,MEDIA,BUILTIN_PORTAUDIO,BUILTIN_PORTMIDI,X11,OGL,ASSIMP,FFMPEG" "$@"
