#!/usr/bin/env python3
import argparse
import json
import os
import signal
import sys
import time
import uuid
import hashlib
import traceback
from pathlib import Path
from typing import Any, Dict, List, Optional

try:
    import fcntl
except ImportError:
    fcntl = None

VERSION = "1.5.9"

def _result(req_id: Any, result: Any) -> Dict[str, Any]:
    return {"jsonrpc": "2.0", "id": req_id, "result": result}

def _error(req_id: Any, code: int, message: str) -> Dict[str, Any]:
    return {"jsonrpc": "2.0", "id": req_id, "error": {"code": code, "message": message}}

def get_storage_dir() -> Path:
    env_path = os.environ.get('MCP_GENERALIST_STORAGE')
    if env_path: return Path(env_path)
    if os.name == 'nt':
        base = os.environ.get('APPDATA')
        if base: return Path(base) / "mcp_generalist"
    return Path.home() / ".config" / "mcp_generalist"

class GeneralistRuntime:
    def __init__(self, storage_dir: Optional[Path] = None) -> None:
        self.storage_dir = storage_dir or get_storage_dir()
        self.tasks_dir = self.storage_dir / "tasks"
        self.workers_dir = self.storage_dir / "workers"
        self.pipes_dir = self.storage_dir / "pipes"
        for d in [self.tasks_dir, self.workers_dir, self.pipes_dir]:
            d.mkdir(parents=True, exist_ok=True)

    def _get_identity(self, repo_path: str, index: int, title: Optional[str] = None) -> str:
        repo_path = str(Path(repo_path).resolve())
        ident = f"{repo_path} #{index}"
        if title: ident += f" ({title})"
        return ident

    def _get_identity_slug(self, identity: str) -> str:
        identity = identity.lstrip("/").strip()
        slug = hashlib.sha256(identity.encode()).hexdigest()[:12]
        safe_prefix = "".join([c if c.isalnum() else "_" for c in identity[:30]])
        return f"{safe_prefix}_{slug}"

    def get_identity(self, repo_path: str, index: int, title: Optional[str] = None) -> Dict[str, Any]:
        identity = self._get_identity(repo_path, index, title)
        return {"identity": identity, "identity_slug": self._get_identity_slug(identity)}

    def register_worker(self, repo_path: str, index: int, title: Optional[str] = None) -> Dict[str, Any]:
        identity = self._get_identity(repo_path, index, title)
        identity_slug = self._get_identity_slug(identity)
        worker_data = {
            "identity": identity, "identity_slug": identity_slug,
            "repo_path": str(Path(repo_path).resolve()), "index": index,
            "title": title or "", "pid": os.getpid(), "last_seen": time.time(), "status": "ready"
        }
        worker_file = self.workers_dir / f"{identity_slug}.json"
        worker_file.write_text(json.dumps(worker_data, indent=2))
        return worker_data

    def worker_listen(self, identity: str, timeout: float = 3600.0) -> Dict[str, Any]:
        """Blocks until a task is assigned. Uses FIFOs on Linux for instant wake-up."""
        identity = identity.lstrip("/").strip()
        identity_slug = self._get_identity_slug(identity)
        if (self.workers_dir / f"{identity}.json").exists():
            identity_slug = identity
            try:
                data = json.loads((self.workers_dir / f"{identity_slug}.json").read_text())
                identity = data.get("identity", identity).lstrip("/").strip()
            except Exception: pass

        worker_file = self.workers_dir / f"{identity_slug}.json"
        if not worker_file.exists():
            return {"status": "error", "message": f"Worker not registered: {identity_slug}"}

        try:
            data = json.loads(worker_file.read_text())
            data["last_listen_attempt"] = time.time()
            worker_file.write_text(json.dumps(data, indent=2))
        except Exception: pass

        lock_file = None
        if fcntl:
            try:
                lock_file = open(worker_file, "r+")
                fcntl.flock(lock_file, fcntl.LOCK_EX | fcntl.LOCK_NB)
            except (BlockingIOError, PermissionError):
                return {"status": "error", "message": "Already listening."}

        pipe_path = self.pipes_dir / f"{identity_slug}.pipe"
        if hasattr(os, "mkfifo") and not pipe_path.exists():
            try: os.mkfifo(pipe_path)
            except Exception: pass

        deadline = time.time() + timeout
        try:
            while time.time() < deadline:
                try:
                    f = lock_file if lock_file else open(worker_file, "r+")
                    f.seek(0); data = json.loads(f.read())
                    data["last_seen"] = time.time(); data["status"] = "listening"
                    f.seek(0); f.write(json.dumps(data, indent=2)); f.truncate()
                    if not lock_file: f.close()
                except Exception: pass

                alt_identity = identity.replace(" #", "@") if " #" in identity else identity.replace("@", " #")
                targets = {identity.lstrip("/").strip(), alt_identity.lstrip("/").strip(), identity_slug.lstrip("/").strip()}
                
                for p in self.tasks_dir.glob("task_*.json"):
                    if p.name.endswith("_report.json"): continue
                    try:
                        task = json.loads(p.read_text())
                        task_target = str(task.get("target", "")).lstrip("/").strip()
                        if task_target in targets and task.get("status") == "assigned":
                            task["status"] = "claimed"; task["claimed_at"] = time.time()
                            p.write_text(json.dumps(task, indent=2))
                            return task
                    except Exception: continue
                time.sleep(1.0)
        finally:
            if lock_file:
                fcntl.flock(lock_file, fcntl.LOCK_UN)
                lock_file.close()
        return {"status": "timeout", "message": "No task received."}

    def report_task(self, task_id: str, status: str, output: str) -> Dict[str, Any]:
        report_data = {"task_id": task_id, "status": status, "output": output, "reported_at": time.time()}
        (self.tasks_dir / f"task_{task_id}_report.json").write_text(json.dumps(report_data, indent=2))
        task_file = self.tasks_dir / f"task_{task_id}.json"
        if task_file.exists():
            try:
                task = json.loads(task_file.read_text())
                task["status"] = status
                task_file.write_text(json.dumps(task, indent=2))
            except Exception: pass
        return report_data

    def list_workers(self, active_only: bool = False) -> Dict[str, Any]:
        workers = []
        now = time.time()
        for p in self.workers_dir.glob("*.json"):
            try:
                data = json.loads(p.read_text())
                is_listening = False
                if fcntl:
                    try:
                        f = open(p, "r+")
                        fcntl.flock(f, fcntl.LOCK_EX | fcntl.LOCK_NB)
                        fcntl.flock(f, fcntl.LOCK_UN)
                        f.close()
                    except (BlockingIOError, PermissionError): is_listening = True
                data["online"] = is_listening or (now - data.get("last_seen", 0) < 600)
                data["listening"] = is_listening
                data["last_seen_relative"] = int(now - data.get("last_seen", 0))
                if active_only and not is_listening: continue
                if now - data.get("last_seen", 0) < 86400: workers.append(data)
            except Exception: continue
        return {"count": len(workers), "workers": workers}

    def assign(self, target: str, task: str, metadata: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        task_id = str(uuid.uuid4())
        task_data = {
            "id": task_id, "target": target, "task": task,
            "metadata": metadata or {}, "created_at": time.time(), "status": "assigned"
        }
        task_file = self.tasks_dir / f"task_{task_id}.json"
        task_file.write_text(json.dumps(task_data, indent=2))
        
        slug = target if (self.workers_dir / f"{target}.json").exists() else self._get_identity_slug(target)
        pipe_path = self.pipes_dir / f"{slug}.pipe"
        if pipe_path.exists():
            try:
                fd = os.open(pipe_path, os.O_WRONLY | os.O_NONBLOCK)
                os.write(fd, b"1")
                os.close(fd)
            except Exception: pass
        return {"task_id": task_id, "status": "assigned", "file": str(task_file)}

    def status(self, task_id: str) -> Dict[str, Any]:
        task_file = self.tasks_dir / f"task_{task_id}.json"
        report_file = self.tasks_dir / f"task_{task_id}_report.json"
        if not task_file.exists(): raise RuntimeError(f"Task {task_id} not found")
        task_data = json.loads(task_file.read_text())
        if report_file.exists():
            task_data["report"] = json.loads(report_file.read_text())
        return task_data

    def listen(self, task_id: str, timeout: float = 30.0) -> Dict[str, Any]:
        deadline = time.time() + timeout
        while time.time() < deadline:
            res = self.status(task_id)
            if "report" in res: return res
            time.sleep(1.0)
        return {"task_id": task_id, "status": "timeout", "message": "timed out waiting for report"}

    def planner_help(self) -> Dict[str, Any]:
        return {
            "summary": "Coordinating multiple worker agents globally via MCP.",
            "workflow": ["1. START WORKERS: In each worker repo, register and call 'generalist_worker_listen'.", "2. DISCOVERY: Use 'generalist_list_active_workers' to find CURRENT listeners.", "3. DELEGATION: Use 'generalist_assign(target=identity, task=...)'.", "4. MONITORING: Use 'generalist_listen' to wait for report."],
            "storage_path": str(self.storage_dir)
        }

    def worker_help(self) -> Dict[str, Any]:
        return {
            "onboarding": ["1. IDENTITY: Use 'generalist_get_identity' to see your string.", "2. REGISTER: Call 'generalist_register_worker'.", "3. LISTEN: Call 'generalist_worker_listen(identity)'. Blocks (up to 1h) until task arrives.", "4. EXECUTE: Perform task text when unblocked.", "5. REPORT: Call 'generalist_report_task'."]
        }

class GeneralistMcpServer:
    def __init__(self, runtime: GeneralistRuntime) -> None:
        self.runtime = runtime

    def handle(self, req: Dict[str, Any]) -> Dict[str, Any]:
        req_id, method, params = req.get("id"), req.get("method"), req.get("params") or {}
        if not method: return _error(req_id, -32600, "missing method")
        try:
            if method == "initialize": return _result(req_id, {"protocolVersion": "2024-11-05", "capabilities": {"tools": {"listChanged": False}}, "serverInfo": {"name": "mcp-generalist", "version": VERSION}})
            if method == "notifications/initialized": return None
            if method == "tools/list":
                return _result(req_id, {"tools": [
                    {"name": "generalist_planner_help", "description": "Show planner guidance.", "inputSchema": {"type": "object", "properties": {}}},
                    {"name": "generalist_worker_help", "description": "Show worker guidance.", "inputSchema": {"type": "object", "properties": {}}},
                    {"name": "generalist_get_version", "description": "Get server version.", "inputSchema": {"type": "object", "properties": {}}},
                    {"name": "generalist_get_identity", "description": "Calculate identity string.", "inputSchema": {"type": "object", "properties": {"repo_path": {"type": "string"}, "index": {"type": "integer"}, "title": {"type": "string"}}, "required": ["repo_path", "index"]}},
                    {"name": "generalist_register_worker", "description": "Register worker.", "inputSchema": {"type": "object", "properties": {"repo_path": {"type": "string"}, "index": {"type": "integer"}, "title": {"type": "string"}}, "required": ["repo_path", "index"]}},
                    {"name": "generalist_worker_listen", "description": "Wait for task. (Blocking)", "inputSchema": {"type": "object", "properties": {"identity": {"type": "string"}, "timeout": {"type": "number", "default": 3600.0}}, "required": ["identity"]}},
                    {"name": "generalist_report_task", "description": "Submit report.", "inputSchema": {"type": "object", "properties": {"task_id": {"type": "string"}, "status": {"type": "string", "enum": ["completed", "failed"]}, "output": {"type": "string"}}, "required": ["task_id", "status", "output"]}},
                    {"name": "generalist_list_active_workers", "description": "List currently listening workers.", "inputSchema": {"type": "object", "properties": {}}},
                    {"name": "generalist_list_registered_workers", "description": "List all registered workers.", "inputSchema": {"type": "object", "properties": {}}},
                    {"name": "generalist_assign", "description": "Assign task.", "inputSchema": {"type": "object", "properties": {"target": {"type": "string"}, "task": {"type": "string"}, "metadata": {"type": "object"}}, "required": ["target", "task"]}},
                    {"name": "generalist_status", "description": "Check task status.", "inputSchema": {"type": "object", "properties": {"task_id": {"type": "string"}}, "required": ["task_id"]}},
                    {"name": "generalist_listen", "description": "Wait for report. (Blocking)", "inputSchema": {"type": "object", "properties": {"task_id": {"type": "string"}, "timeout": {"type": "number", "default": 30.0}}, "required": ["task_id"]}}
                ]})
            if method == "tools/call":
                n, a = params.get("name"), params.get("arguments", {})
                if n == "generalist_planner_help": return _result(req_id, {"content": [{"type": "text", "text": json.dumps(self.runtime.planner_help(), indent=2)}]})
                if n == "generalist_worker_help": return _result(req_id, {"content": [{"type": "text", "text": json.dumps(self.runtime.worker_help(), indent=2)}]})
                if n == "generalist_get_version": return _result(req_id, {"content": [{"type": "text", "text": json.dumps({"version": VERSION}, indent=2)}]})
                if n == "generalist_get_identity": return _result(req_id, {"content": [{"type": "text", "text": json.dumps(self.runtime.get_identity(a.get("repo_path"), int(a.get("index", 1)), a.get("title")), indent=2)}]})
                if n == "generalist_register_worker": return _result(req_id, {"content": [{"type": "text", "text": json.dumps(self.runtime.register_worker(a.get("repo_path"), int(a.get("index", 1)), a.get("title")), indent=2)}]})
                if n == "generalist_worker_listen":
                    res = self.runtime.worker_listen(a.get("identity"), float(a.get("timeout", 3600.0)))
                    note = "\n\nNote: Call generalist_worker_listen again when you have finished and reported the task above, to remain available for the next assignment."
                    return _result(req_id, {"content": [{"type": "text", "text": json.dumps(res, indent=2) + note}]})
                if n == "generalist_report_task": return _result(req_id, {"content": [{"type": "text", "text": json.dumps(self.runtime.report_task(a.get("task_id"), a.get("status"), a.get("output")), indent=2)}]})
                if n == "generalist_list_active_workers": return _result(req_id, {"content": [{"type": "text", "text": json.dumps(self.runtime.list_workers(active_only=True), indent=2)}]})
                if n == "generalist_list_registered_workers": return _result(req_id, {"content": [{"type": "text", "text": json.dumps(self.runtime.list_workers(active_only=False), indent=2)}]})
                if n == "generalist_assign": return _result(req_id, {"content": [{"type": "text", "text": json.dumps(self.runtime.assign(a.get("target"), a.get("task"), a.get("metadata")), indent=2)}]})
                if n == "generalist_status": return _result(req_id, {"content": [{"type": "text", "text": json.dumps(self.runtime.status(a.get("task_id")), indent=2)}]})
                if n == "generalist_listen": return _result(req_id, {"content": [{"type": "text", "text": json.dumps(self.runtime.listen(a.get("task_id"), float(a.get("timeout", 30.0))), indent=2)}]})
                return _error(req_id, -32601, f"tool not found: {n}")
            if method == "mcp.capabilities": return _result(req_id, {"methods": ["mcp.ping", "mcp.capabilities", "mcp.help", "generalist.planner_help", "generalist.worker_help", "generalist.get_version", "generalist.get_identity", "generalist.register_worker", "generalist.worker_listen", "generalist.report_task", "generalist.list_active_workers", "generalist.list_registered_workers", "generalist.assign", "generalist.status", "generalist.listen"]})
            if method == "generalist.list_active_workers": return _result(req_id, self.runtime.list_workers(active_only=True))
            if method == "generalist.list_registered_workers": return _result(req_id, self.runtime.list_workers(active_only=False))
            if method == "generalist.assign": return _result(req_id, self.runtime.assign(params.get("target"), params.get("task"), params.get("metadata")))
            if method == "generalist.status": return _result(req_id, self.runtime.status(params.get("task_id")))
            if method == "generalist.listen": return _result(req_id, self.runtime.listen(params.get("task_id"), params.get("timeout", 30.0)))
            return _error(req_id, -32601, f"method not found: {method}")
        except Exception as exc: return _error(req_id, -32000, str(exc))

def main() -> int:
    p = argparse.ArgumentParser(); p.add_argument("--storage-dir"); p.add_argument("--debug-log"); p.add_argument("--worker-listen", action="store_true"); p.add_argument("--repo-path"); p.add_argument("--index", type=int, default=1); p.add_argument("--title"); p.add_argument("--timeout", type=float, default=3600.0)
    args = p.parse_args(); runtime = GeneralistRuntime(Path(args.storage_dir).resolve() if args.storage_dir else None)
    if args.worker_listen:
        if not args.repo_path: return 1
        reg = runtime.register_worker(args.repo_path, args.index, args.title)
        task = runtime.worker_listen(reg['identity'], args.timeout)
        print(json.dumps(task, indent=2))
        print("\nNote: Run this command again when you have finished and reported the task to remain available.", file=sys.stderr)
        return 0 if task.get("status") != "timeout" else 1
    server = GeneralistMcpServer(runtime); debug_file = open(args.debug_log, "a") if args.debug_log else None
    def log(msg):
        if debug_file: debug_file.write(f"[{time.strftime('%H:%M:%S')}] {msg}\n"); debug_file.flush()
    signal.signal(signal.SIGINT, lambda *_: sys.exit(0))
    for line in sys.stdin:
        if not line.strip(): continue
        log(f"REQ: {line.strip()}")
        try:
            req = json.loads(line); resp = server.handle(req)
            if resp:
                out = json.dumps(resp); log(f"RESP: {out}")
                sys.stdout.write(out + "\n"); sys.stdout.flush()
        except Exception as exc:
            log(f"ERROR: {exc}\n{traceback.format_exc()}"); sys.stdout.write(json.dumps(_error(None, -32700, str(exc))) + "\n"); sys.stdout.flush()
    return 0

if __name__ == "__main__": sys.exit(main())
