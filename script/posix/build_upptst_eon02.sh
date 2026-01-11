#!/bin/sh

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)

"$SCRIPT_DIR/build_upptst_eon_generic.sh" Eon02 "AUDIO,MIDI,MEDIA,BUILTIN_PORTAUDIO,BUILTIN_PORTMIDI,FFMPEG,FLUIDLITE,SCREEN,AI" "$@"
