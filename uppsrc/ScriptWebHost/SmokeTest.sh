#!/usr/bin/env bash
set -euo pipefail

url="${1:-http://127.0.0.1:9247/}"
out="${2:-/tmp/scriptwebhost-smoke.png}"

npx playwright screenshot \
  -b firefox \
  --wait-for-selector "body[data-runtime='running'][data-sprite-count='52']" \
  --wait-for-timeout 1500 \
  --timeout 20000 \
  --viewport-size "1400,1000" \
  "$url" "$out" >/dev/null

python3 uppsrc/ScriptWebHost/BrowserRegression.py "${url}?autoplay=1" "${out%.png}-autoplay.png" >/dev/null
python3 uppsrc/ScriptWebHost/BrowserDragRegression.py "${url}" "${out%.png}-drag.png" >/dev/null

file "$out"
