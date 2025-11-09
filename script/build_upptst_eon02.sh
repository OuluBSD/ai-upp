#!/bin/sh

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)

"$SCRIPT_DIR/build_upptst_eon_generic.sh" Eon02 "AI,SCREEN,BUILTIN_PORTAUDIO,BUILTIN_PORTMIDI,FLUIDLITE" "$@"
