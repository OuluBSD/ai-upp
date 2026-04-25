#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

OUT_DIR="${ROOT_DIR}/.tmp/test-scriptide-usemalloc"
mkdir -p "${OUT_DIR}"
script/build.py -mc 1 -c -j8 -od "${OUT_DIR}" uppsrc/ScriptIDE
SCRIPTIDE_BIN="${OUT_DIR}/ScriptIDE"

TEST_FILE="uppsrc/ScriptIDE/reference/Hearts/game.gamestate"
CMD=(
  "${SCRIPTIDE_BIN}"
  "${TEST_FILE}"
  "--separate-run-target=local.game_window"
  "--run-separate-after-ms=300"
  "--expect-separate-open-after-ms=1200"
  "--close-separate-after-ms=1800"
  "--expect-separate-closed-after-ms=3200"
  "--force-close-after-ms=5000"
)

status=0
if command -v xvfb-run >/dev/null 2>&1; then
  timeout 20s xvfb-run -a "${CMD[@]}" || status=$?
elif [[ -n "${DISPLAY:-}" ]] && command -v xset >/dev/null 2>&1 && xset q >/dev/null 2>&1; then
  timeout 20s "${CMD[@]}" || status=$?
else
  echo "No usable X11 display and xvfb-run is not available; cannot run ScriptIDE GUI automation test." >&2
  exit 2
fi

if [[ "${status}" -ne 0 ]]; then
  echo "ScriptIDE separate-window automation failed (exit=${status})." >&2
  LOG_FILE="${HOME}/.local/state/u++/log/ScriptIDE.log"
  if [[ -f "${LOG_FILE}" ]]; then
    echo "--- ScriptIDE.log (tail -n 200) ---" >&2
    tail -n 200 "${LOG_FILE}" >&2 || true
    echo "--- end ScriptIDE.log ---" >&2
  fi
fi

exit "${status}"
