#!/bin/bash

CLEAN_ARG=""
if [ "$1" == "--clean" ]; then
  CLEAN_ARG="a"
  shift # Remove --clean from arguments
fi

# Build:
umk upptst,examples,tutorial,reference,uppsrc "${1}" $HOME/.config/u++/theide/CLANG.bm -bsH1"${CLEAN_ARG}" +NOGUI,USEMALLOC,DEBUG_FULL "bin/${1}"

# Run ide:
echo "Executable compiled: bin/${1}"