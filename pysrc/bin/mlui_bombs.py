#!/usr/bin/env python3
import argparse
import json
import re
import shlex
import signal
import socket
import subprocess
import sys
import time
from pathlib import Path

DEFAULT_ADDR = "127.0.0.1:8082"
PACKAGE = "examples/Bombs"
APP_NAME = "Bombs"


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(
        description="Build and launch Bombs + MLUI Viewer",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    p.add_argument(
        "-gui",
        action="store_true",
        help=(
            "Run visible GUI backend for Bombs while also enabling --mlui-server__ "
            "(if MLUI-only mainconfig exists, both builds are done)"
        ),
    )
    p.add_argument(
        "-dumpcmd",
        action="store_true",
        help="Print the resolved Bombs launch command and exit",
    )
    p.add_argument(
        "-gdb",
        action="store_true",
        help=(
            "Launch Bombs through gdb in batch mode and dump full backtrace "
            "after stop/crash"
        ),
    )
    p.add_argument(
        "-gdb-viewer",
        action="store_true",
        help=(
            "Launch MluiViewer through gdb in batch mode and dump full backtrace "
            "after stop/crash"
        ),
    )
    p.add_argument(
        "--addr",
        default=DEFAULT_ADDR,
        help="MLUI server bind address host:port",
    )
    p.add_argument(
        "--refresh-ms",
        type=int,
        default=700,
        help="MluiViewer auto-refresh period in milliseconds",
    )
    return p.parse_args()


def repo_root_from_script() -> Path:
    # .../pysrc/bin/mlui_bombs.py -> repo root is parents[2]
    return Path(__file__).resolve().parents[2]


def run_cmd(repo_root: Path, args: list[str]) -> None:
    print("+", " ".join(args), flush=True)
    subprocess.run(args, cwd=repo_root, check=True)


def parse_addr(addr: str) -> tuple[str, int]:
    s = addr.strip()
    q = s.rfind(":")
    if q <= 0 or q == len(s) - 1:
        raise ValueError(f"Invalid --addr '{addr}', expected host:port")
    host = s[:q].strip()
    port_txt = s[q + 1 :].strip()
    try:
        port = int(port_txt)
    except ValueError as exc:
        raise ValueError(f"Invalid port in --addr '{addr}'") from exc
    if port <= 0 or port > 65535:
        raise ValueError(f"Invalid port in --addr '{addr}'")
    return host, port


def wait_mlui_server(host: str, port: int, timeout_s: float = 20.0) -> bool:
    deadline = time.time() + timeout_s
    req = b'{"id":1,"method":"ping"}\n'
    while time.time() < deadline:
        try:
            with socket.create_connection((host, port), timeout=1.0) as sock:
                sock.settimeout(1.0)
                sock.sendall(req)
                line = b""
                while b"\n" not in line:
                    chunk = sock.recv(65536)
                    if not chunk:
                        break
                    line += chunk
            if not line:
                time.sleep(0.2)
                continue
            obj = json.loads(line.split(b"\n", 1)[0].decode("utf-8"))
            if isinstance(obj, dict) and obj.get("ok"):
                return True
        except Exception:
            pass
        time.sleep(0.2)
    return False


def terminate_process(proc: subprocess.Popen) -> None:
    if proc.poll() is not None:
        return
    proc.terminate()
    try:
        proc.wait(timeout=5)
    except subprocess.TimeoutExpired:
        proc.kill()
        proc.wait(timeout=5)


def build_app_args(app_bin: Path, addr: str) -> list[str]:
    return [str(app_bin), "--mlui-server__", addr]


def build_viewer_args(viewer_bin: Path, addr: str, refresh_ms: int) -> list[str]:
    return [
        str(viewer_bin),
        "--mlui",
        addr,
        "--auto-refresh",
        "--auto-refresh-ms",
        str(refresh_ms),
    ]


def build_gdb_args(target_name: str, target_args: list[str]) -> list[str]:
    gdb_args = [
        "gdb",
        "-q",
        "--batch",
        "--return-child-result",
    ]
    gdb_args += [
        "-ex",
        "set pagination off",
        "-ex",
        "set confirm off",
        "-ex",
        "run",
        "-ex",
        f"echo \\n[mlui_bombs] gdb post-run stack dump ({target_name}):\\n",
        "-ex",
        "thread apply all bt full",
        "--args",
        *target_args,
    ]
    return gdb_args


def wait_until_exit(
    app_proc: subprocess.Popen, viewer_proc: subprocess.Popen
) -> tuple[str, int]:
    while True:
        app_rc = app_proc.poll()
        viewer_rc = viewer_proc.poll()
        if app_rc is not None:
            return "app", app_rc
        if viewer_rc is not None:
            return "viewer", viewer_rc
        time.sleep(0.2)


def list_main_configs(repo_root: Path, build_py: Path, package: str) -> dict[int, str]:
    cp = subprocess.run(
        [str(build_py), "--list-conf", package],
        cwd=repo_root,
        check=True,
        capture_output=True,
        text=True,
    )
    out: dict[int, str] = {}
    line_re = re.compile(r"^\[(\d+)\]\s*=\s*(.*)$")
    for line in cp.stdout.splitlines():
        m = line_re.match(line.strip())
        if not m:
            continue
        out[int(m.group(1))] = m.group(2).strip()
    return out


def pick_build_configs(configs: dict[int, str]) -> tuple[int, int, bool]:
    # Returns: runtime_config, gui_config, has_explicit_mlui_config
    if not configs:
        return 0, 0, False

    sorted_idx = sorted(configs.keys())
    mlui_idx = None
    gui_idx = None

    for idx in sorted_idx:
        flags = configs[idx].upper()
        if "MLUI" in flags and mlui_idx is None:
            mlui_idx = idx
        if "GUI" in flags and "MLUI" not in flags and gui_idx is None:
            gui_idx = idx

    if gui_idx is None:
        gui_idx = sorted_idx[0]

    if mlui_idx is not None:
        return mlui_idx, gui_idx, True
    return gui_idx, gui_idx, False


def main() -> int:
    args = parse_args()

    try:
        host, port = parse_addr(args.addr)
    except ValueError as exc:
        print(exc, file=sys.stderr)
        return 2
    if args.refresh_ms <= 0:
        print("--refresh-ms must be positive", file=sys.stderr)
        return 2

    repo_root = repo_root_from_script()
    build_py = repo_root / "script" / "build.py"
    app_bin = repo_root / "bin" / APP_NAME
    viewer_bin = repo_root / "bin" / "MluiViewer"
    app_args = build_app_args(app_bin, args.addr)
    viewer_args = build_viewer_args(viewer_bin, args.addr, args.refresh_ms)
    app_launch_args = build_gdb_args("bombs", app_args) if args.gdb else app_args
    viewer_launch_args = (
        build_gdb_args("viewer", viewer_args) if args.gdb_viewer else viewer_args
    )

    if args.dumpcmd:
        print(f"+ {APP_NAME}:", shlex.join(app_launch_args), flush=True)
        print("+ MluiViewer:", shlex.join(viewer_launch_args), flush=True)
        return 0

    if not build_py.exists():
        print(f"Missing build script: {build_py}", file=sys.stderr)
        return 2

    try:
        configs = list_main_configs(repo_root, build_py, PACKAGE)
        runtime_mc, gui_mc, has_mlui_mc = pick_build_configs(configs)
        if not has_mlui_mc:
            print(
                f"Warning: {PACKAGE} has no explicit MLUI mainconfig, "
                f"using -mc {runtime_mc} and --mlui-server__ only.",
                file=sys.stderr,
            )

        # 1) Build runtime config.
        run_cmd(repo_root, [str(build_py), "-mc", str(runtime_mc), PACKAGE])

        # 2) -gui mode may need visible backend config as additional build.
        if args.gui and gui_mc != runtime_mc:
            run_cmd(repo_root, [str(build_py), "-mc", str(gui_mc), PACKAGE])

        # 3) Build viewer.
        run_cmd(repo_root, [str(build_py), "MluiViewer"])
    except subprocess.CalledProcessError as exc:
        return exc.returncode or 1

    if not app_bin.exists():
        print(f"Missing executable: {app_bin}", file=sys.stderr)
        return 1
    if not viewer_bin.exists():
        print(f"Missing executable: {viewer_bin}", file=sys.stderr)
        return 1

    print("+", " ".join(app_launch_args), flush=True)
    app_proc = subprocess.Popen(app_launch_args, cwd=repo_root)

    child_procs: list[subprocess.Popen] = [app_proc]

    def _forward_signal(signum, _frame):
        for proc in child_procs:
            if proc.poll() is None:
                proc.send_signal(signum)

    signal.signal(signal.SIGINT, _forward_signal)
    signal.signal(signal.SIGTERM, _forward_signal)

    viewer_proc: subprocess.Popen | None = None
    try:
        if not wait_mlui_server(host, port, timeout_s=20.0):
            rc = app_proc.poll()
            if rc is not None:
                if rc != 0:
                    print(f"{APP_NAME} exited with code {rc}", file=sys.stderr)
                return rc
            print(
                f"Warning: MLUI server did not respond at {args.addr} in time",
                file=sys.stderr,
            )

        print("+", " ".join(viewer_launch_args), flush=True)
        viewer_proc = subprocess.Popen(viewer_launch_args, cwd=repo_root)
        child_procs.append(viewer_proc)

        who, rc = wait_until_exit(app_proc, viewer_proc)
        if who == "app":
            if viewer_proc.poll() is None:
                terminate_process(viewer_proc)
            if rc != 0:
                print(f"{APP_NAME} exited with code {rc}", file=sys.stderr)
            return rc

        if viewer_rc := rc:
            print(f"MluiViewer exited with code {viewer_rc}", file=sys.stderr)
            return viewer_rc
        return 0
    finally:
        if viewer_proc is not None:
            terminate_process(viewer_proc)
        terminate_process(app_proc)


if __name__ == "__main__":
    raise SystemExit(main())
