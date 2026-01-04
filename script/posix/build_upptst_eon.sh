#!/bin/sh

if [ $# -eq 0 ]; then
	echo "Usage: $0 <target> [options]" >&2
	exit 1
fi

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)

target=""
extra_args=""

while [ $# -gt 0 ]; do
	case "$1" in
		--no-debug-rt|--release|--uppheap|--clean|-c)
			if [ -z "$extra_args" ]; then
				extra_args=$1
			else
				extra_args="$extra_args $1"
			fi
			;;
		*)
			if [ -z "$target" ]; then
				target="$1"
			else
				if [ -z "$extra_args" ]; then
					extra_args=$1
				else
					extra_args="$extra_args $1"
				fi
			fi
			;;
	esac
	shift
done

if [ -z "$target" ]; then
	echo "Usage: $0 <target> [options]" >&2
	exit 1
fi

if [ -n "$extra_args" ]; then
	# shellcheck disable=SC2086
	set -- $extra_args
else
	set --
fi

exec "$SCRIPT_DIR/build_upptst_eon_generic.sh" "$target" "AI,DEBUG_VFS" "$@"
