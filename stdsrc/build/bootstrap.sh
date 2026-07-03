#!/usr/bin/env sh
# Portable POSIX bootstrap for bin/build (see bootstrap.ps1 for the Windows/MSVC
# equivalent). Compiles stdsrc/build/main.cpp with a plain C++ compiler (no umk,
# no script/build.py needed yet) and then runs `bin/build --bootstrap`, which
# builds umk itself via uppsrc/umk/Makefile.linux64 / Makefile.macos15.
#
# Usage: stdsrc/build/bootstrap.sh [--source PATH] [--output PATH] [-- extra args passed to `bin/build --bootstrap`]

set -e

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
REPO_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)

SOURCE="$SCRIPT_DIR/main.cpp"
OUTPUT="$REPO_ROOT/bin/build"

while [ $# -gt 0 ]; do
	case "$1" in
		--source)
			SOURCE="$2"
			shift 2
			;;
		--output)
			OUTPUT="$2"
			shift 2
			;;
		--)
			shift
			break
			;;
		*)
			break
			;;
	esac
done

if [ ! -f "$SOURCE" ]; then
	echo "Source file not found: $SOURCE" >&2
	exit 1
fi

mkdir -p "$(dirname "$OUTPUT")"

CXX_BIN="${CXX:-}"
if [ -z "$CXX_BIN" ]; then
	if command -v clang++ >/dev/null 2>&1; then
		CXX_BIN=clang++
	elif command -v g++ >/dev/null 2>&1; then
		CXX_BIN=g++
	else
		echo "No C++ compiler found (tried \$CXX, clang++, g++)." >&2
		exit 1
	fi
fi

echo "Using compiler: $CXX_BIN"
"$CXX_BIN" -std=c++11 -O2 -Wall -o "$OUTPUT" "$SOURCE"

echo "Bootstrapping umk via $OUTPUT --bootstrap"
exec "$OUTPUT" --bootstrap "$@"
