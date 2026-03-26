#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

OUT_DIR="${ROOT_DIR}/.tmp/test-scriptide-export"
BIN_DIR="${OUT_DIR}/bin"
EXPORT_DIR="${OUT_DIR}/exported"
mkdir -p "${BIN_DIR}" "${EXPORT_DIR}"

script/build.py -mc 1 -c -j8 -od "${BIN_DIR}" uppsrc/ScriptIDE
SCRIPTIDE_BIN="${BIN_DIR}/ScriptIDE"

TEST_FILE="uppsrc/ScriptIDE/reference/Hearts/game.gamestate"
EXPORT_BIN="${EXPORT_DIR}/HeartsEmbedded"

status=0
if command -v xvfb-run >/dev/null 2>&1; then
  timeout 300s xvfb-run -a "${SCRIPTIDE_BIN}" "${TEST_FILE}" \
    "--export-standalone=${EXPORT_BIN}" || status=$?
elif [[ -n "${DISPLAY:-}" ]] && command -v xset >/dev/null 2>&1 && xset q >/dev/null 2>&1; then
  timeout 300s "${SCRIPTIDE_BIN}" "${TEST_FILE}" \
    "--export-standalone=${EXPORT_BIN}" || status=$?
else
  echo "No usable X11 display and xvfb-run is not available; cannot run ScriptIDE export automation test." >&2
  exit 2
fi

if [[ "${status}" -eq 0 ]] && [[ ! -f "${EXPORT_BIN}" ]]; then
  echo "Standalone export did not produce expected binary: ${EXPORT_BIN}" >&2
  status=1
fi

if [[ "${status}" -eq 0 ]]; then
  RUN_DIR="${OUT_DIR}/clean-run"
  rm -rf "${RUN_DIR}"
  mkdir -p "${RUN_DIR}"

  run_status=0
  if command -v xvfb-run >/dev/null 2>&1; then
    (
      cd "${RUN_DIR}"
      timeout 12s xvfb-run -a "${EXPORT_BIN}"
    ) || run_status=$?
  elif [[ -n "${DISPLAY:-}" ]] && command -v xset >/dev/null 2>&1 && xset q >/dev/null 2>&1; then
    (
      cd "${RUN_DIR}"
      timeout 12s "${EXPORT_BIN}"
    ) || run_status=$?
  else
    echo "No usable X11 display and xvfb-run is not available; cannot smoke-run exported binary." >&2
    run_status=2
  fi

  if [[ "${run_status}" -ne 124 ]]; then
    echo "Exported binary did not stay alive under clean-dir smoke run (expected timeout 124, got ${run_status})." >&2
    status=1
  fi
fi

if [[ "${status}" -ne 0 ]]; then
  echo "ScriptIDE standalone export automation failed (exit=${status})." >&2
  LOG_FILE="${HOME}/.local/state/u++/log/ScriptIDE.log"
  if [[ -f "${LOG_FILE}" ]]; then
    echo "--- ScriptIDE.log (tail -n 200) ---" >&2
    tail -n 200 "${LOG_FILE}" >&2 || true
    echo "--- end ScriptIDE.log ---" >&2
  fi
fi

exit "${status}"
