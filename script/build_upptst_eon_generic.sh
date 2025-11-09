#!/bin/sh

strip_flag() {
	printf '%s\n' "$1" | awk -v flag="$2" 'BEGIN { FS="," } {
		out = ""
		for(i = 1; i <= NF; i++) {
			if($i == flag || $i == "")
				continue
			if(out == "")
				out = $i
			else
				out = out "," $i
		}
		print out
	}'
}

append_flag() {
	current=$(strip_flag "$1" "$2")
	if [ -z "$current" ]; then
		printf '%s\n' "$2"
	else
		printf '%s,%s\n' "$current" "$2"
	fi
}

if [ $# -lt 2 ]; then
	echo "Usage: $0 <target> <base_flags> [options]" >&2
	exit 1
fi

target=$1
shift
base_flags=$1
shift

release=0
enable_debug_rt=1
enable_debug_full=1
use_malloc=1
clean=0

while [ $# -gt 0 ]; do
	case "$1" in
		--no-debug-rt)
			enable_debug_rt=0
			;;
		--release)
			release=1
			enable_debug_rt=0
			enable_debug_full=0
			;;
		--uppheap)
			use_malloc=0
			;;
		--clean|-c)
			clean=1
			;;
		*)
			echo "Unknown option: $1" >&2
			exit 1
			;;
	esac
	shift
done

if [ "$release" -eq 1 ]; then
	build_flags="-bsH1"
else
	build_flags="-bsdH1"
fi

if [ "$clean" -eq 1 ]; then
	build_flags="${build_flags}a"
fi

final_flags=$(strip_flag "$base_flags" "USEMALLOC")
final_flags=$(strip_flag "$final_flags" "DEBUG_RT")
final_flags=$(strip_flag "$final_flags" "DEBUG_FULL")

if [ "$use_malloc" -eq 1 ]; then
	final_flags=$(append_flag "$final_flags" "USEMALLOC")
fi

if [ "$enable_debug_full" -eq 1 ]; then
	final_flags=$(append_flag "$final_flags" "DEBUG_FULL")
fi

if [ "$enable_debug_rt" -eq 1 ]; then
	final_flags=$(append_flag "$final_flags" "DEBUG_RT")
fi

if [ -n "$final_flags" ]; then
	umk ./upptst,./uppsrc "$target" ~/.config/u++/theide/CLANG.bm "$build_flags" "+$final_flags" "bin/$target"
else
	umk ./upptst,./uppsrc "$target" ~/.config/u++/theide/CLANG.bm "$build_flags" "bin/$target"
fi

echo "Executable compiled: bin/$target"
