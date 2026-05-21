#!/bin/bash

# remote_build_macos_ide.sh
# Usage: ./script/remote_build_macos_ide.sh user@hostname

REMOTE=$1

if [ -z "$REMOTE" ]; then
    echo "Usage: $0 user@hostname"
    exit 1
fi

echo "--- Synchronizing files to $REMOTE ---"
rsync -avz --delete \
    --exclude '.git' \
    --exclude '.gemini' \
    --exclude '.tmp' \
    --exclude '_out' \
    --exclude 'out' \
    --exclude 'bin' \
    . "$REMOTE:Dev/ai-upp/"

echo "--- Building on remote ---"
ssh "$REMOTE" "cd Dev/ai-upp && \
    echo 'Checking build tool...' && \
    ( [ -f bin/build ] || ( mkdir -p bin && g++ -std=c++11 stdsrc/build/main.cpp -o bin/build ) ) && \
    echo 'Bootstrapping umk...' && \
    bin/build -bs && \
    echo 'Building ide...' && \
    bin/build -m 0 ide && \
    echo 'Refreshing LaunchServices database...' && \
    /System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/LaunchServices.framework/Versions/A/Support/lsregister -u bin/ide.app && \
    /System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/LaunchServices.framework/Versions/A/Support/lsregister -f bin/ide.app && \
    touch bin/ide.app && \
    echo 'Build complete. If Finder still shows an error, try moving bin/ide.app to Desktop and back.'"
