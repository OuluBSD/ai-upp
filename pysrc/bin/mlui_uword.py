#!/usr/bin/env python3
import argparse
import json
import shlex
import signal
import socket
import subprocess
import sys
import time
from pathlib import Path

DEFAULT_ADDR = "127.0.0.1:8082"


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(
        description="Build and launch UWord + MLUI Viewer",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    p.add_argument(
        "-gui",
        action="store_true",
        help=(
            "Run visible GUI backend for UWord (builds and runs GUI config) "
            "while also enabling --mlui-server__"
        ),
    )
    p.add_argument(
        "-dumpcmd",
        action="store_true",
        help="Print the resolved UWord launch command and exit",
    )
    p.add_argument(
        "-gdb",
        action="store_true",
        help=(
            "Launch UWord through gdb in batch mode and dump full backtrace "
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
    # .../pysrc/bin/mlui_uword.py -> repo root is parents[2]
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


def build_uword_args(uword_bin: Path, addr: str) -> list[str]:
    return [str(uword_bin), "--mlui-server__", addr]


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
        f"echo \\n[mlui_uword] gdb post-run stack dump ({target_name}):\\n",
        "-ex",
        "thread apply all bt full",
        "--args",
        *target_args,
    ]
    return gdb_args


def wait_until_exit(
    uword_proc: subprocess.Popen, viewer_proc: subprocess.Popen
) -> tuple[str, int]:
    while True:
        uword_rc = uword_proc.poll()
        viewer_rc = viewer_proc.poll()
        if uword_rc is not None:
            return "uword", uword_rc
        if viewer_rc is not None:
            return "viewer", viewer_rc
        time.sleep(0.2)


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
    uword_bin = repo_root / "bin" / "UWord"
    viewer_bin = repo_root / "bin" / "MluiViewer"
    uword_args = build_uword_args(uword_bin, args.addr)
    viewer_args = build_viewer_args(viewer_bin, args.addr, args.refresh_ms)
    uword_launch_args = build_gdb_args("uword", uword_args) if args.gdb else uword_args
    viewer_launch_args = (
        build_gdb_args("viewer", viewer_args) if args.gdb_viewer else viewer_args
    )

    if args.dumpcmd:
        print("+ UWord:", shlex.join(uword_launch_args), flush=True)
        print("+ MluiViewer:", shlex.join(viewer_launch_args), flush=True)
        return 0

    if not build_py.exists():
        print(f"Missing build script: {build_py}", file=sys.stderr)
        return 2

    try:
        # 1) Build MLUI UWord exactly as requested.
        run_cmd(repo_root, [str(build_py), "-mc", "3", "examples/UWord"])

        # -gui mode needs visible backend build too.
        if args.gui:
            run_cmd(repo_root, [str(build_py), "-mc", "0", "examples/UWord"])

        # 2) Build viewer.
        run_cmd(repo_root, [str(build_py), "MluiViewer"])
    except subprocess.CalledProcessError as exc:
        return exc.returncode or 1

    if not uword_bin.exists():
        print(f"Missing executable: {uword_bin}", file=sys.stderr)
        return 1
    if not viewer_bin.exists():
        print(f"Missing executable: {viewer_bin}", file=sys.stderr)
        return 1

    print("+", " ".join(uword_launch_args), flush=True)
    uword_proc = subprocess.Popen(uword_launch_args, cwd=repo_root)

    child_procs: list[subprocess.Popen] = [uword_proc]

    def _forward_signal(signum, _frame):
        for proc in child_procs:
            if proc.poll() is None:
                proc.send_signal(signum)

    signal.signal(signal.SIGINT, _forward_signal)
    signal.signal(signal.SIGTERM, _forward_signal)

    viewer_proc: subprocess.Popen | None = None
    try:
        if not wait_mlui_server(host, port, timeout_s=20.0):
            rc = uword_proc.poll()
            if rc is not None:
                if rc != 0:
                    print(f"UWord exited with code {rc}", file=sys.stderr)
                return rc
            print(f"Warning: MLUI server did not respond at {args.addr} in time", file=sys.stderr)

        print("+", " ".join(viewer_launch_args), flush=True)
        viewer_proc = subprocess.Popen(viewer_launch_args, cwd=repo_root)
        child_procs.append(viewer_proc)

        who, rc = wait_until_exit(uword_proc, viewer_proc)
        if who == "uword":
            if viewer_proc.poll() is None:
                terminate_process(viewer_proc)
            if rc != 0:
                print(f"UWord exited with code {rc}", file=sys.stderr)
            return rc

        if viewer_rc := rc:
            print(f"MluiViewer exited with code {viewer_rc}", file=sys.stderr)
            return viewer_rc
        return 0
    finally:
        if viewer_proc is not None:
            terminate_process(viewer_proc)
        terminate_process(uword_proc)


if __name__ == "__main__":
    raise SystemExit(main())
