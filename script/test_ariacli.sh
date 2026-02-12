#!/bin/bash
set -e

echo "--- AriaCLI Functional Test Suite ---"

# 1. Build
echo "Building AriaCLI..."
script/build.py AriaCLI

# 2. Help
echo "Testing help output..."
bin/AriaCLI | grep -q "Aria CLI"
echo "✓ Help works."

# 3. News Path
echo "Checking config path..."
PATH_BIN=$(bin/AriaCLI news path)
echo "Config path: $PATH_BIN"
if [[ "$PATH_BIN" != *"/AriaHub/News.bin" ]]; then
    echo "Error: Config path mismatch ($PATH_BIN)"
    exit 1
fi
echo "✓ Config path OK."

# 4. News Scrape (Dry run or limited)
echo "Testing news scrape (ZeroHedge)..."
# We skip full scrape to save time, but check if command exists
bin/AriaCLI news 2>&1 | grep -q "scrape"
echo "✓ News scrape command available."

# 5. News List
echo "Testing news list..."
bin/AriaCLI news list > /tmp/news_list.txt
if [ -s /tmp/news_list.txt ]; then
    echo "✓ News list has data."
    head -n 5 /tmp/news_list.txt
else
    echo "? News list empty, might be first run."
fi

# 6. Navigator
echo "Testing navigator (google.com title)..."
# Simple eval check - REQUIRES 'return' in script
TITLE=$(bin/AriaCLI nav https://www.google.com eval "return document.title" | grep "Result:" | cut -d' ' -f2-)
if [[ "$TITLE" == *"Google"* ]]; then
    echo "✓ Navigator eval works (Title: $TITLE)"
else
    echo "Error: Navigator eval failed or unexpected title ($TITLE)"
    exit 1
fi

echo "--- ALL TESTS PASSED ---"
