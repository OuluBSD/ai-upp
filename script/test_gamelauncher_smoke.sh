#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

OUT_DIR="${ROOT_DIR}/.tmp/test-gamelauncher-usemalloc"
mkdir -p "${OUT_DIR}"
script/build.py -mc 1 -c -j8 -od "${OUT_DIR}" uppsrc/GameLauncher
LAUNCHER_BIN="${OUT_DIR}/GameLauncher"

TEST_FILE="uppsrc/ScriptIDE/reference/Hearts/game.gamestate"
CMD=(
  "${LAUNCHER_BIN}"
  "${TEST_FILE}"
  "--force-close-after-ms=1800"
)

status=0
if command -v xvfb-run >/dev/null 2>&1; then
  timeout 20s xvfb-run -a "${CMD[@]}" || status=$?
elif [[ -n "${DISPLAY:-}" ]] && command -v xset >/dev/null 2>&1 && xset q >/dev/null 2>&1; then
  timeout 20s "${CMD[@]}" || status=$?
else
  echo "No usable X11 display and xvfb-run is not available; cannot run GameLauncher smoke test." >&2
  exit 2
fi

if [[ "${status}" -ne 0 ]]; then
  echo "GameLauncher smoke test failed (exit=${status})." >&2
  LOG_FILE="${HOME}/.local/state/u++/log/GameLauncher.log"
  if [[ -f "${LOG_FILE}" ]]; then
    echo "--- GameLauncher.log (tail -n 200) ---" >&2
    tail -n 200 "${LOG_FILE}" >&2 || true
    echo "--- end GameLauncher.log ---" >&2
  fi
fi

exit "${status}"
