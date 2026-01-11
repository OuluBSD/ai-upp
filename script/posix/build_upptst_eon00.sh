#!/bin/sh

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)

"$SCRIPT_DIR/build_upptst_eon_generic.sh" Eon00 "AI,AUDIO,DEBUG_VFS" "$@"
status=$?

if [ $status -ne 0 ]; then
	exit $status
fi

if ls upptst/Eon00/*.eon >/dev/null 2>&1; then
	cp upptst/Eon00/*.eon bin/
fi
