#!/usr/bin/env python3
import argparse
import json
import os
import shlex
import signal
import socket
import subprocess
import sys
import threading
import time
from collections import deque
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Deque, Dict, List, Optional, Union


def _result(req_id: Any, result: Any) -> Dict[str, Any]:
    return {"jsonrpc": "2.0", "id": req_id, "result": result}


def _error(req_id: Any, code: int, message: str) -> Dict[str, Any]:
    return {
        "jsonrpc": "2.0",
        "id": req_id,
        "error": {"code": code, "message": message},
    }


def _trim(v: Any) -> str:
    return str(v if v is not None else "").strip()


def _safe_int(v: Any, default: int) -> int:
    try:
        return int(v)
    except Exception:  # noqa: BLE001
        return default


@dataclass
class ProcEntry:
    handle: str
    popen: Union[subprocess.Popen, "_DetachedProcess"]
    cmd: List[str]
    started_at: float
    log: Deque[str] = field(default_factory=lambda: deque(maxlen=5000))
    reader_thread: Optional[threading.Thread] = None


class _DetachedProcess:
    """Wraps a pid we re-attached to but didn't spawn."""

    def __init__(self, pid: int) -> None:
        self.pid = pid
        self.returncode: Optional[int] = None

    def poll(self) -> Optional[int]:
        if self.returncode is not None:
            return self.returncode
        try:
            os.kill(self.pid, 0)
            return None  # still alive
        except ProcessLookupError:
            self.returncode = -1
            return -1
        except PermissionError:
            return None  # alive but not ours

    def terminate(self) -> None:
        try:
            os.kill(self.pid, signal.SIGTERM)
        except OSError:
            pass

    def kill(self) -> None:
        try:
            os.kill(self.pid, signal.SIGKILL)
        except OSError:
            pass

    def wait(self, timeout: Optional[float] = None) -> int:
        deadline = time.time() + (timeout or 10)
        while time.time() < deadline:
            if self.poll() is not None:
                return self.returncode or -1
            time.sleep(0.2)
        return self.returncode or -1


class BuildRuntime:
    def __init__(self, repo_root: Path) -> None:
        self.repo_root = repo_root
        self.build_py = repo_root / "script" / "build.py"
        self.next_handle = 1
        self.procs: Dict[str, ProcEntry] = {}
        self._load_handles()

        if not self.build_py.exists():
            raise RuntimeError(f"missing build script: {self.build_py}")

    def _save_handles(self) -> None:
        path = self.repo_root / ".tmp" / "build_handles.json"
        path.parent.mkdir(exist_ok=True)
        data = {}
        for handle, entry in self.procs.items():
            if entry.popen.poll() is None:
                data[handle] = {
                    "pid": entry.popen.pid,
                    "cmd": entry.cmd,
                    "started_at": entry.started_at,
                }
        try:
            path.write_text(json.dumps(data, indent=2))
        except OSError:
            pass

    def _load_handles(self) -> None:
        path = self.repo_root / ".tmp" / "build_handles.json"
        if not path.exists():
            return
        try:
            data = json.loads(path.read_text())
        except Exception:
            return
        for handle, info in data.items():
            pid = info.get("pid")
            if not pid:
                continue
            try:
                os.kill(pid, 0)
            except OSError:
                continue
            popen = _DetachedProcess(pid)
            entry = ProcEntry(
                handle=handle,
                popen=popen,
                cmd=info.get("cmd", []),
                started_at=info.get("started_at", 0.0),
            )
            self.procs[handle] = entry
            try:
                self.next_handle = max(self.next_handle, int(handle) + 1)
            except ValueError:
                pass

    def _run_capture(self, cmd: List[str], timeout: Optional[float] = None) -> Dict[str, Any]:
        cp = subprocess.run(
            cmd,
            cwd=self.repo_root,
            text=True,
            capture_output=True,
            timeout=timeout,
            check=False,
        )
        return {
            "cmd": cmd,
            "cmd_str": shlex.join(cmd),
            "returncode": cp.returncode,
            "stdout": cp.stdout,
            "stderr": cp.stderr,
        }

    def list_conf(self, package: str) -> Dict[str, Any]:
        pkg = _trim(package)
        if not pkg:
            raise RuntimeError("package is required")
        out = self._run_capture([str(self.build_py), "--list-conf", pkg], timeout=120)
        return out

    def compile(
        self,
        package: str,
        mainconfig: Optional[int],
        jobs: Optional[int],
        rainbow: bool,
        extra_args: Optional[List[str]] = None,
    ) -> Dict[str, Any]:
        pkg = _trim(package)
        if not pkg:
            raise RuntimeError("package is required")

        cmd: List[str] = [str(self.build_py)]
        if rainbow:
            cmd.append("-R")
        if mainconfig is not None:
            cmd += ["-mc", str(mainconfig)]
        if jobs is not None and jobs > 0:
            cmd += ["-j", str(jobs)]
        if extra_args:
            cmd += [str(x) for x in extra_args]
        cmd.append(pkg)

        return self._run_capture(cmd, timeout=None)

    def _start_log_reader(self, entry: ProcEntry) -> None:
        def _reader() -> None:
            stream = entry.popen.stdout
            if stream is None:
                return
            try:
                for line in iter(stream.readline, ""):
                    if not line:
                        break
                    entry.log.append(line.rstrip("\n"))
            except Exception:  # noqa: BLE001
                pass

        t = threading.Thread(target=_reader, daemon=True)
        entry.reader_thread = t
        t.start()

    def launch(
        self,
        package: Optional[str],
        executable: Optional[str],
        args: List[str],
        mlui_server: Optional[str],
        build_first: bool,
        mainconfig: Optional[int],
        jobs: Optional[int],
        rainbow: bool,
        mlui_timeout: float,
    ) -> Dict[str, Any]:
        pkg = _trim(package)
        exe = _trim(executable)

        compile_result: Optional[Dict[str, Any]] = None
        if build_first:
            target_pkg = pkg if pkg else exe
            if not target_pkg:
                raise RuntimeError("launch requires package or executable")
            compile_result = self.compile(target_pkg, mainconfig, jobs, rainbow)
            if compile_result["returncode"] != 0:
                return {
                    "built": False,
                    "compile": compile_result,
                    "error": "build failed",
                }

        if not exe:
            if not pkg:
                raise RuntimeError("launch requires package or executable")
            exe = Path(pkg).name

        if "/" in exe:
            exe_path = (self.repo_root / exe).resolve()
        else:
            exe_path = (self.repo_root / "bin" / exe).resolve()

        if not exe_path.exists():
            raise RuntimeError(f"executable not found: {exe_path}")

        cmd = [str(exe_path)]
        if mlui_server:
            cmd += ["--mlui-server__", mlui_server]
        cmd += [str(x) for x in args]

        popen = subprocess.Popen(
            cmd,
            cwd=self.repo_root,
            stdin=subprocess.DEVNULL,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,
            preexec_fn=os.setsid,
        )

        handle = str(self.next_handle)
        self.next_handle += 1

        entry = ProcEntry(
            handle=handle,
            popen=popen,
            cmd=cmd,
            started_at=time.time(),
        )
        self.procs[handle] = entry
        self._start_log_reader(entry)
        time.sleep(1)
        self._save_handles()

        out: Dict[str, Any] = {
            "handle": handle,
            "pid": popen.pid,
            "cmd": cmd,
            "cmd_str": shlex.join(cmd),
            "built": compile_result is not None,
            "compile": compile_result,
        }

        if mlui_server:
            host = "127.0.0.1"
            port = 8082
            if ":" in mlui_server:
                h, p = mlui_server.rsplit(":", 1)
                host = h
                port = int(p)
            else:
                host = mlui_server

            out["mlui_port"] = port
            deadline = time.time() + mlui_timeout
            wait_start = time.time()
            ready = False
            while time.time() < deadline:
                if popen.poll() is not None:
                    out["mlui_ready"] = False
                    out["mlui_error"] = f"process exited with returncode {popen.returncode} before MLUI port became ready"
                    return out
                try:
                    with socket.create_connection((host, port), timeout=0.3):
                        ready = True
                        break
                except OSError:
                    time.sleep(0.3)
            
            out["mlui_ready"] = ready
            out["mlui_wait_sec"] = time.time() - wait_start
            if not ready:
                out["mlui_error"] = "timeout waiting for MLUI port to become ready"

        return out

    def status(self, handle: str) -> Dict[str, Any]:
        h = _trim(handle)
        if not h:
            raise RuntimeError("handle is required")
        entry = self.procs.get(h)
        if entry is None:
            raise RuntimeError(f"unknown handle: {h}")

        rc = entry.popen.poll()
        out: Dict[str, Any] = {
            "handle": h,
            "pid": entry.popen.pid,
            "running": rc is None,
            "returncode": rc,
            "uptime_sec": max(0.0, time.time() - entry.started_at),
            "cmd": entry.cmd,
            "cmd_str": shlex.join(entry.cmd),
            "log_lines": len(entry.log),
        }

        # Scan for MLUI bind line
        if not isinstance(entry.popen, _DetachedProcess):
            for line in entry.log:
                if "MLUI: listening at" in line:
                    out["mlui_bind_line"] = line.strip()
                    break

        return out

    def stop(self, handle: str, force: bool) -> Dict[str, Any]:
        h = _trim(handle)
        if not h:
            raise RuntimeError("handle is required")
        entry = self.procs.get(h)
        if entry is None:
            raise RuntimeError(f"unknown handle: {h}")

        if entry.popen.poll() is None:
            if force:
                entry.popen.kill()
            else:
                entry.popen.terminate()
            try:
                entry.popen.wait(timeout=5)
            except subprocess.TimeoutExpired:
                entry.popen.kill()
                entry.popen.wait(timeout=5)

        rc = entry.popen.poll()
        self._save_handles()
        return {
            "handle": h,
            "pid": entry.popen.pid,
            "running": False,
            "returncode": rc,
        }

    def logs(self, handle: str, tail: int) -> Dict[str, Any]:
        h = _trim(handle)
        if not h:
            raise RuntimeError("handle is required")
        entry = self.procs.get(h)
        if entry is None:
            raise RuntimeError(f"unknown handle: {h}")

        if entry.reader_thread is None:
            return {
                "handle": h,
                "tail": 0,
                "line_count": 0,
                "lines": [],
                "note": "process re-attached from previous session, logs not available",
            }

        tail = max(1, int(tail))
        lines = list(entry.log)[-tail:]
        return {
            "handle": h,
            "tail": tail,
            "line_count": len(lines),
            "lines": lines,
        }

    def list_running(self) -> Dict[str, Any]:
        items = []
        for h in sorted(self.procs.keys(), key=lambda x: int(x)):
            try:
                items.append(self.status(h))
            except Exception:  # noqa: BLE001
                pass
        return {"count": len(items), "items": items}


class BuildMcpServer:
    def __init__(self, runtime: BuildRuntime) -> None:
        self.runtime = runtime

    def handle(self, req: Dict[str, Any]) -> Dict[str, Any]:
        req_id = req.get("id")
        method = req.get("method")
        params = req.get("params") or {}

        if not isinstance(method, str) or not method:
            return _error(req_id, -32600, "invalid request: missing method")
        if not isinstance(params, dict):
            return _error(req_id, -32602, "invalid params")

        try:
            if method == "mcp.ping":
                return _result(req_id, {"text": "pong"})

            if method == "mcp.help":
                return _result(
                    req_id,
                    {
                        "summary": "MLUI build and launch server. Handles are per-process — store the handle from build.launch and use it in the same MCP session. Use build.running to rediscover handles if a session is lost.",
                        "methods": {
                            "mcp.ping": {"description": "Check if server is alive.", "params": {}},
                            "mcp.capabilities": {"description": "List all available methods.", "params": {}},
                            "build.list_conf": {
                                "description": "List all main configurations for a package.",
                                "params": {"package": {"type": "string", "required": True}},
                            },
                            "build.compile": {
                                "description": "Compile a package.",
                                "params": {
                                    "package": {"type": "string", "required": True},
                                    "mainconfig": {"type": "integer", "required": False},
                                    "jobs": {"type": "integer", "required": False},
                                },
                            },
                            "build.launch": {
                                "description": "Launch an application.",
                                "params": {
                                    "package": {"type": "string", "required": False},
                                    "executable": {"type": "string", "required": False},
                                    "args": {"type": "array", "required": False},
                                    "mlui_server": {"type": "string", "required": False, "description": "e.g. '127.0.0.1:18082'"},
                                    "mlui_timeout": {"type": "number", "required": False, "default": 15.0},
                                    "build": {"type": "boolean", "required": False, "default": True},
                                    "mainconfig": {"type": "integer", "required": False},
                                    "jobs": {"type": "integer", "required": False},
                                },
                            },
                            "build.status": {
                                "description": "Check the status of a launched application.",
                                "params": {"handle": {"type": "string", "required": True}},
                            },
                            "build.stop": {
                                "description": "Stop a launched application.",
                                "params": {
                                    "handle": {"type": "string", "required": True},
                                    "force": {"type": "boolean", "required": False},
                                },
                            },
                            "build.logs": {
                                "description": "Get logs from a launched application.",
                                "params": {
                                    "handle": {"type": "string", "required": True},
                                    "tail": {"type": "integer", "required": False, "default": 200},
                                },
                            },
                            "build.running": {"description": "List all running applications.", "params": {}},
                        },
                        "examples": [
                            {"method": "build.compile", "params": {"package": "reference/MLUI_Focus", "mainconfig": 0}},
                            {
                                "method": "build.launch",
                                "params": {
                                    "package": "reference/MLUI_Focus",
                                    "mainconfig": 0,
                                    "build": True,
                                    "mlui_server": "127.0.0.1:18082",
                                },
                            },
                            {"method": "build.status", "params": {"handle": "<handle from launch>"}},
                            {"method": "build.logs", "params": {"handle": "<handle>", "tail": 50}},
                            {"method": "build.stop", "params": {"handle": "<handle>", "force": False}},
                            {"method": "build.running", "params": {}},
                        ],
                    },
                )

            if method == "mcp.capabilities":
                return _result(
                    req_id,
                    {
                        "protocol": "jsonrpc-2.0",
                        "methods": [
                            "mcp.ping",
                            "mcp.capabilities",
                            "build.list_conf",
                            "build.compile",
                            "build.launch",
                            "build.status",
                            "build.stop",
                            "build.logs",
                            "build.running",
                        ],
                    },
                )

            if method == "build.list_conf":
                package = _trim(params.get("package", ""))
                return _result(req_id, self.runtime.list_conf(package))

            if method == "build.compile":
                package = _trim(params.get("package", ""))
                mc = params.get("mainconfig")
                mainconfig = None if mc is None else _safe_int(mc, 0)
                jobs = _safe_int(params.get("jobs"), 0)
                if jobs <= 0:
                    jobs = None
                rainbow = bool(params.get("rainbow", False))
                extra = params.get("extra_args")
                extra_args = extra if isinstance(extra, list) else None
                return _result(
                    req_id,
                    self.runtime.compile(package, mainconfig, jobs, rainbow, extra_args),
                )

            if method == "build.launch":
                package = _trim(params.get("package", "")) or None
                executable = _trim(params.get("executable", "")) or None
                args = params.get("args")
                app_args = [str(x) for x in args] if isinstance(args, list) else []
                mlui_server = _trim(params.get("mlui_server", "")) or None
                mlui_timeout = float(params.get("mlui_timeout", 15.0))
                build_first = bool(params.get("build", True))
                mc = params.get("mainconfig")
                mainconfig = None if mc is None else _safe_int(mc, 0)
                jobs = _safe_int(params.get("jobs"), 0)
                if jobs <= 0:
                    jobs = None
                rainbow = bool(params.get("rainbow", False))

                return _result(
                    req_id,
                    self.runtime.launch(
                        package,
                        executable,
                        app_args,
                        mlui_server,
                        build_first,
                        mainconfig,
                        jobs,
                        rainbow,
                        mlui_timeout,
                    ),
                )

            if method == "build.status":
                return _result(req_id, self.runtime.status(_trim(params.get("handle", ""))))

            if method == "build.stop":
                handle = _trim(params.get("handle", ""))
                force = bool(params.get("force", False))
                return _result(req_id, self.runtime.stop(handle, force))

            if method == "build.logs":
                handle = _trim(params.get("handle", ""))
                tail = _safe_int(params.get("tail"), 200)
                return _result(req_id, self.runtime.logs(handle, tail))

            if method == "build.running":
                return _result(req_id, self.runtime.list_running())

            return _error(req_id, -32601, f"method not found: {method}")
        except Exception as exc:  # noqa: BLE001
            return _error(req_id, -32000, str(exc))


def _parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="MCP build+launch helper using script/build.py")
    default_root = Path(__file__).resolve().parents[2]
    p.add_argument("--repo-root", default=str(default_root))
    return p.parse_args()


def main() -> int:
    args = _parse_args()
    runtime = BuildRuntime(Path(args.repo_root).resolve())
    server = BuildMcpServer(runtime)

    def _shutdown(*_a: Any) -> None:
        for h in list(runtime.procs.keys()):
            try:
                runtime.stop(h, force=True)
            except Exception:  # noqa: BLE001
                pass

    def _on_signal(*_a: Any) -> None:
        _shutdown()
        raise SystemExit(0)

    signal.signal(signal.SIGINT, _on_signal)
    signal.signal(signal.SIGTERM, _on_signal)

    for line in sys.stdin:
        line = line.strip()
        if not line:
            continue

        try:
            req = json.loads(line)
        except Exception as exc:  # noqa: BLE001
            resp = _error(None, -32700, f"parse error: {exc}")
            sys.stdout.write(json.dumps(resp, ensure_ascii=False) + "\n")
            sys.stdout.flush()
            continue

        if not isinstance(req, dict):
            resp = _error(None, -32600, "invalid request object")
        else:
            resp = server.handle(req)

        sys.stdout.write(json.dumps(resp, ensure_ascii=False) + "\n")
        sys.stdout.flush()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
