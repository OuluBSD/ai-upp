#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PKG="${1:-examples/Bombs}"
MAINCONFIG="${MAINCONFIG:-0}"
MLUI_ADDR="${MLUI_ADDR:-127.0.0.1:8123}"

if [[ -z "${DISPLAY:-}" ]]; then
	echo "test_mlui_focus_smoke: DISPLAY is not set; GUI target cannot run."
	exit 2
fi

echo "[smoke] building ${PKG} (mc=${MAINCONFIG})"
"${ROOT_DIR}/script/build.py" -mc "${MAINCONFIG}" "${PKG}" >/dev/null

APP_NAME="$(basename "${PKG}")"
APP_BIN="${ROOT_DIR}/bin/${APP_NAME}"
if [[ ! -x "${APP_BIN}" ]]; then
	echo "test_mlui_focus_smoke: missing executable ${APP_BIN}"
	exit 1
fi

echo "[smoke] launching ${APP_BIN} --mlui-server__ ${MLUI_ADDR}"
python3 - "${APP_BIN}" "${MLUI_ADDR}" <<'PY'
import json
import socket
import subprocess
import sys
import time

app_bin = sys.argv[1]
addr = sys.argv[2]
host, port_s = addr.rsplit(":", 1)
port = int(port_s)

p = subprocess.Popen([app_bin, "--mlui-server__", addr], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

def req(method, params=None, timeout=4.0):
    payload = {"id": 1, "method": method, "params": params or {}}
    with socket.create_connection((host, port), timeout=timeout) as sock:
        sock.settimeout(timeout)
        sock.sendall((json.dumps(payload) + "\n").encode("utf-8"))
        buf = b""
        while b"\n" not in buf:
            c = sock.recv(65536)
            if not c:
                break
            buf += c
    if not buf.strip():
        raise RuntimeError(f"empty response for {method}")
    return json.loads(buf.split(b"\n", 1)[0].decode("utf-8"))

def wait_ping():
    for _ in range(120):
        try:
            r = req("ping", timeout=1.0)
            if r.get("ok"):
                return
        except Exception:
            pass
        time.sleep(0.1)
    raise RuntimeError("MLUI server did not answer ping in time")

def expect_ok(method, params=None):
    r = req(method, params)
    if not r.get("ok"):
        raise RuntimeError(f"{method} failed: {r.get('error')}")
    return r

def expect_known_error(method, params=None):
    r = req(method, params)
    if r.get("ok"):
        return r
    err = str(r.get("error", ""))
    if "Unknown method" in err:
        raise RuntimeError(f"{method} returned unknown-method error unexpectedly: {err}")
    return r

try:
    wait_ping()
    expect_ok("snapshot", {"include_hidden": True, "include_focus": True})

    for method, params in [
        ("click", {"path": "__smoke__/missing"}),
        ("set", {"path": "__smoke__/missing", "value": 1}),
        ("key", {"key": 13}),
        ("mouse", {"event": "click", "x": -99999, "y": -99999}),
        ("focus.get", {"page": "__missing__"}),
        ("focus.action", {"page": "__missing__", "action": "__missing__"}),
    ]:
        expect_known_error(method, params)

    expect_ok("focus.list")
    expect_ok("focus.tree", {"depth": 2})
    expect_ok("focus.search", {"query": "*", "limit": 16})

    print("MLUI focus smoke OK")
finally:
    p.terminate()
    try:
        p.wait(timeout=5)
    except Exception:
        p.kill()
PY
