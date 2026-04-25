#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

OUT_DIR="${ROOT_DIR}/.tmp/test-scriptide-usemalloc"
mkdir -p "${OUT_DIR}"
script/build.py -mc 1 -c -j8 -od "${OUT_DIR}" uppsrc/ScriptIDE
SCRIPTIDE_BIN="${OUT_DIR}/ScriptIDE"

TEST_FILE="uppsrc/ScriptIDE/reference/Hearts/game.gamestate"
SENTINEL="/tmp/scriptide_external_process_target.$$.sentinel"
rm -f "${SENTINEL}"

CMD=(
  "${SCRIPTIDE_BIN}"
  "${TEST_FILE}"
  "--separate-run-target=local.external_process"
  "--external-process-binary=/usr/bin/touch"
  "--external-process-extra-args=${SENTINEL}"
  "--external-process-hide-terminal"
  "--external-process-no-wait-for-exit"
  "--run-separate-after-ms=300"
  "--expect-external-launch-after-ms=900"
  "--force-close-after-ms=1600"
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

if [[ "${status}" -eq 0 ]] && [[ ! -f "${SENTINEL}" ]]; then
  echo "External process target did not create sentinel file: ${SENTINEL}" >&2
  status=1
fi
rm -f "${SENTINEL}"

if [[ "${status}" -ne 0 ]]; then
  echo "ScriptIDE external-process target automation failed (exit=${status})." >&2
  LOG_FILE="${HOME}/.local/state/u++/log/ScriptIDE.log"
  if [[ -f "${LOG_FILE}" ]]; then
    echo "--- ScriptIDE.log (tail -n 200) ---" >&2
    tail -n 200 "${LOG_FILE}" >&2 || true
    echo "--- end ScriptIDE.log ---" >&2
  fi
fi

exit "${status}"
