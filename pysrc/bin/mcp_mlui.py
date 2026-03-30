#!/usr/bin/env python3
import argparse
import json
import os
import re
import socket
import sys
import time
from dataclasses import dataclass, field
from typing import Any, Dict, List, Optional, Tuple


def _result(req_id: Any, result: Any) -> Dict[str, Any]:
    return {"jsonrpc": "2.0", "id": req_id, "result": result}


def _error(req_id: Any, code: int, message: str) -> Dict[str, Any]:
    return {
        "jsonrpc": "2.0",
        "id": req_id,
        "error": {"code": code, "message": message},
    }


def _trim(s: Any) -> str:
    return str(s if s is not None else "").strip()


def _split_semantic(semantic_path: str) -> List[str]:
    s = _trim(semantic_path)
    if not s:
        return []
    out: List[str] = []
    pos = 0
    while pos < len(s):
        q = s.find("->", pos)
        if q < 0:
            q = len(s)
        part = s[pos:q].strip()
        if part:
            out.append(part)
        pos = q + 2
    return out


def _semantic_type_rank(semantic_path: str) -> int:
    parts = _split_semantic(semantic_path)
    if not parts:
        return 10
    last = parts[-1]
    q = last.find(":")
    stype = (last[:q] if q >= 0 else last).strip().lower()
    if stype == "action":
        return 0
    if stype == "option":
        return 1
    if stype == "button":
        return 2
    if stype == "bar":
        return 3
    return 10


def _semantic_depth(semantic_path: str) -> int:
    return len(_split_semantic(semantic_path))


def _normalize_fs_path(path: str, cwd: str) -> str:
    p = _trim(path)
    if not p:
        p = cwd

    if p.startswith("/"):
        parts: List[str] = []
    else:
        parts = [x for x in cwd.split("/") if x]

    for seg in p.split("/"):
        seg = seg.strip()
        if not seg or seg == ".":
            continue
        if seg == "..":
            if parts:
                parts.pop()
            continue
        parts.append(seg)

    return "/" + "/".join(parts)


def _wildcard_regex(pattern: str, regex: bool) -> Tuple[re.Pattern, bool]:
    p = _trim(pattern)
    if not p:
        raise ValueError("pattern is required")

    if regex:
        return re.compile(p, re.IGNORECASE), True

    if "*" in p:
        rx = "^" + re.escape(p).replace("\\*", ".*") + "$"
        return re.compile(rx, re.IGNORECASE), True

    return re.compile(re.escape(p), re.IGNORECASE), False


class MluiClient:
    def __init__(self, host: str, port: int, timeout: float) -> None:
        self.host = host
        self.port = port
        self.timeout = timeout

    def request(self, method: str, params: Optional[Dict[str, Any]] = None, req_id: Any = None) -> Dict[str, Any]:
        payload = {"id": req_id, "method": method, "params": params or {}}
        raw = json.dumps(payload, ensure_ascii=False) + "\n"

        with socket.create_connection((self.host, self.port), timeout=self.timeout) as sock:
            sock.settimeout(self.timeout)
            sock.sendall(raw.encode("utf-8"))
            buf = b""
            while b"\n" not in buf:
                chunk = sock.recv(65536)
                if not chunk:
                    break
                buf += chunk

        line = buf.split(b"\n", 1)[0].strip()
        if not line:
            raise RuntimeError("empty response from MLUI server")

        try:
            data = json.loads(line.decode("utf-8"))
        except Exception as exc:  # noqa: BLE001
            raise RuntimeError(f"invalid JSON from MLUI server: {exc}") from exc

        if not isinstance(data, dict):
            raise RuntimeError("MLUI response is not an object")
        return data


@dataclass
class PathNode:
    name: str
    children: Dict[str, "PathNode"] = field(default_factory=dict)
    element_ids: List[int] = field(default_factory=list)


class SnapshotIndex:
    def __init__(self) -> None:
        self.elements: List[Dict[str, Any]] = []
        self.root = PathNode(name="")
        self.path_to_ids: Dict[str, List[int]] = {}
        self.timestamp: str = ""
        self.count: int = 0

    def rebuild(self, snapshot_result: Dict[str, Any]) -> None:
        self.root = PathNode(name="")
        self.path_to_ids = {}
        self.timestamp = str(snapshot_result.get("timestamp", ""))
        elems = snapshot_result.get("elements")
        if not isinstance(elems, list):
            elems = []
        self.elements = [e for e in elems if isinstance(e, dict)]
        self.count = len(self.elements)

        for i, el in enumerate(self.elements):
            raw_path = _trim(el.get("path", ""))
            if raw_path:
                fs_path = "/" + "/".join([s for s in raw_path.split("/") if s])
            else:
                sem = _trim(el.get("semantic_path", ""))
                slug = re.sub(r"[^a-zA-Z0-9._-]+", "_", sem)[:64] or f"item_{i}"
                fs_path = f"/@semantic/{slug}_{i}"

            parts = [s for s in fs_path.split("/") if s]
            node = self.root
            for part in parts:
                child = node.children.get(part)
                if child is None:
                    child = PathNode(name=part)
                    node.children[part] = child
                node = child

            node.element_ids.append(i)
            self.path_to_ids.setdefault(fs_path, []).append(i)

    def _node(self, fs_path: str) -> Optional[PathNode]:
        if fs_path == "/":
            return self.root
        node = self.root
        for seg in [s for s in fs_path.split("/") if s]:
            node = node.children.get(seg)
            if node is None:
                return None
        return node

    def exists(self, fs_path: str) -> bool:
        return self._node(fs_path) is not None

    def _node_semantic(self, node: PathNode) -> str:
        if not node.element_ids:
            return ""
        el = self.elements[node.element_ids[0]]
        return _trim(el.get("semantic_path", ""))

    def ls(self, fs_path: str, depth: int, limit: int) -> Dict[str, Any]:
        node = self._node(fs_path)
        if node is None:
            raise RuntimeError(f"path not found: {fs_path}")

        depth = max(1, int(depth))
        limit = max(1, int(limit))

        items: List[Dict[str, Any]] = []

        def walk(cur: PathNode, cur_path: str, d: int) -> None:
            nonlocal items
            if len(items) >= limit:
                return
            if d <= 0:
                return
            for name in sorted(cur.children.keys()):
                if len(items) >= limit:
                    break
                ch = cur.children[name]
                p = cur_path.rstrip("/") + "/" + name if cur_path != "/" else "/" + name
                semantic = self._node_semantic(ch)
                entry = {
                    "name": name,
                    "path": p,
                    "kind": "node",
                    "child_count": len(ch.children),
                    "element_count": len(ch.element_ids),
                    "clickable": any(_trim(self.elements[i].get("path", "")) for i in ch.element_ids),
                    "semantic": semantic,
                }
                items.append(entry)
                walk(ch, p, d - 1)

        walk(node, fs_path, depth)

        node_elements = [self.elements[i] for i in node.element_ids]
        return {
            "path": fs_path,
            "depth": depth,
            "node_child_count": len(node.children),
            "node_element_count": len(node.element_ids),
            "items": items,
            "elements_at_path": node_elements,
        }

    def tree(self, fs_path: str, depth: int, limit: int) -> Dict[str, Any]:
        node = self._node(fs_path)
        if node is None:
            raise RuntimeError(f"path not found: {fs_path}")

        depth = max(1, int(depth))
        limit = max(1, int(limit))

        lines: List[str] = [fs_path]
        used = 1

        def walk(cur: PathNode, prefix: str, d: int) -> None:
            nonlocal used
            if used >= limit or d <= 0:
                return
            names = sorted(cur.children.keys())
            for idx, name in enumerate(names):
                if used >= limit:
                    break
                ch = cur.children[name]
                last = idx == len(names) - 1
                branch = "└─ " if last else "├─ "
                marker = " *" if ch.element_ids else ""
                lines.append(prefix + branch + name + marker)
                used += 1
                walk(ch, prefix + ("   " if last else "│  "), d - 1)

        walk(node, "", depth)
        return {"path": fs_path, "depth": depth, "lines": lines}

    def find(self, pattern: str, base_path: str, field: str, regex: bool, limit: int) -> Dict[str, Any]:
        base = _normalize_fs_path(base_path, "/")
        if not self.exists(base):
            raise RuntimeError(f"base path not found: {base}")

        field = _trim(field).lower() or "any"
        limit = max(1, int(limit))
        rx, full_match = _wildcard_regex(pattern, regex)

        out: List[Dict[str, Any]] = []
        for fs_path, ids in self.path_to_ids.items():
            if not (fs_path == base or fs_path.startswith(base.rstrip("/") + "/") or base == "/"):
                continue
            for i in ids:
                el = self.elements[i]
                candidates: Dict[str, str] = {
                    "path": _trim(el.get("path", "")),
                    "semantic": _trim(el.get("semantic_path", "")),
                    "type": _trim(el.get("type", "")),
                    "text": _trim(el.get("text", "")),
                    "visible_text": _trim(el.get("visible_text", "")),
                    "value": _trim(el.get("value", "")),
                }

                if field == "any":
                    hay = "\n".join(candidates.values())
                else:
                    hay = candidates.get(field, "")

                ok = bool(rx.fullmatch(hay) if full_match else rx.search(hay))
                if not ok:
                    continue

                out.append(
                    {
                        "fs_path": fs_path,
                        "path": candidates["path"],
                        "semantic": candidates["semantic"],
                        "type": candidates["type"],
                        "text": candidates["text"],
                        "visible": bool(el.get("visible", False)),
                    }
                )
                if len(out) >= limit:
                    return {"base_path": base, "pattern": pattern, "count": len(out), "matches": out}

        return {"base_path": base, "pattern": pattern, "count": len(out), "matches": out}

    def choose_click_target(self, fs_path: str) -> Optional[str]:
        node = self._node(fs_path)
        if node is None:
            return None

        stack: List[PathNode] = [node]
        best: Optional[Tuple[int, int, int, str]] = None

        while stack:
            cur = stack.pop()
            for i in cur.element_ids:
                el = self.elements[i]
                target = _trim(el.get("path", ""))
                if not target:
                    continue
                semantic = _trim(el.get("semantic_path", ""))
                rank = _semantic_type_rank(semantic)
                depth = _semantic_depth(semantic)
                visible_bias = 0 if bool(el.get("visible", False)) else 1
                score = (rank, visible_bias, -depth, target)
                if best is None or score < best:
                    best = score
            for name in sorted(cur.children.keys(), reverse=True):
                stack.append(cur.children[name])

        return None if best is None else best[3]


class MluiMcpServer:
    def __init__(self, host: str, port: int, timeout: float) -> None:
        self.client = MluiClient(host, port, timeout)
        self.index = SnapshotIndex()
        self.cwd = "/"
        self.last_refresh = 0.0

    def refresh(self, include_hidden: bool = True) -> Dict[str, Any]:
        backend = self.client.request(
            "snapshot",
            {
                "include_hidden": include_hidden,
                "include_layout": True,
                "include_visible_text": True,
                "include_visible_text_ratio": True,
            },
            req_id="mcp-refresh",
        )
        if not backend.get("ok"):
            raise RuntimeError(_trim(backend.get("error", "snapshot failed")))

        result = backend.get("result")
        if not isinstance(result, dict):
            raise RuntimeError("snapshot result is not object")

        self.index.rebuild(result)
        self.last_refresh = time.time()
        if not self.index.exists(self.cwd):
            self.cwd = "/"

        return {
            "timestamp": self.index.timestamp,
            "count": self.index.count,
            "cwd": self.cwd,
        }

    def ensure_snapshot(self) -> None:
        if self.index.count == 0:
            self.refresh(include_hidden=True)

    def focus_detail(self, page_id: str, kind: str, key: str) -> Dict[str, Any]:
        page = _trim(page_id)
        if not page:
            raise RuntimeError("page is required")

        backend = self.client.request("focus.get", {"page": page}, req_id="mcp-focus-detail")
        if not backend.get("ok"):
            raise RuntimeError(_trim(backend.get("error", "focus.get failed")))

        result = backend.get("result")
        if not isinstance(result, dict):
            raise RuntimeError("focus.get result is not object")

        runtime = result.get("runtime")
        if not isinstance(runtime, dict):
            runtime = {}

        values = runtime.get("values")
        if not isinstance(values, list):
            values = []
        ctrls = runtime.get("ctrls")
        if not isinstance(ctrls, list):
            ctrls = []
        actions = result.get("actions")
        if not isinstance(actions, list):
            actions = []

        out: Dict[str, Any] = {
            "id": _trim(result.get("id", "")),
            "title": _trim(result.get("title", "")),
            "description": _trim(result.get("description", "")),
            "counts": {
                "runtime_values": len(values),
                "runtime_ctrls": len(ctrls),
                "actions": len(actions),
            },
        }

        k = _trim(kind).lower()
        q = _trim(key)
        if k in {"value", "runtime.value"} and q:
            out["item"] = next((x for x in values if isinstance(x, dict) and _trim(x.get("key", "")) == q), None)
        elif k in {"ctrl", "runtime.ctrl"} and q:
            out["item"] = next((x for x in ctrls if isinstance(x, dict) and _trim(x.get("key", "")) == q), None)
        elif k == "action" and q:
            out["item"] = next((x for x in actions if isinstance(x, dict) and _trim(x.get("id", "")) == q), None)
        else:
            out["sample"] = {
                "values": values[:5],
                "ctrls": ctrls[:5],
                "actions": actions[:5],
            }

        return out

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
                        "summary": "MLUI MCP bridge with path + focus navigation",
                        "examples": [
                            {"method": "mlui.refresh", "params": {"include_hidden": True}},
                            {"method": "fs.tree", "params": {"path": "/", "depth": 2}},
                            {"method": "fs.find", "params": {"pattern": "*File Tree*", "field": "semantic"}},
                            {"method": "mlui.focus.list", "params": {}},
                            {"method": "mlui.focus.detail", "params": {"page": "file_tree"}},
                            {"method": "mlui.focus.action", "params": {"page": "file_tree", "action": "reload"}},
                            {
                                "method": "mlui.focus.action",
                                "params": {
                                    "page": "file_tree",
                                    "action": "select",
                                    "args": {"path": "uppsrc/Overviewer/Main.cpp"},
                                },
                            },
                            {
                                "method": "mlui.focus.action",
                                "params": {
                                    "page": "active_file",
                                    "action": "set_priority",
                                    "args": {"value": 3},
                                },
                            },
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
                            "mcp.help",
                            "mcp.capabilities",
                            "mlui.refresh",
                            "mlui.snapshot",
                            "mlui.click",
                            "mlui.set",
                            "mlui.key",
                            "mlui.mouse",
                            "mlui.click_path",
                            "mlui.focus.list",
                            "mlui.focus.get",
                            "mlui.focus.tree",
                            "mlui.focus.search",
                            "mlui.focus.action",
                            "mlui.focus.detail",
                            "fs.pwd",
                            "fs.cd",
                            "fs.ls",
                            "fs.tree",
                            "fs.find",
                        ],
                    },
                )

            if method == "mlui.refresh":
                include_hidden = bool(params.get("include_hidden", True))
                return _result(req_id, self.refresh(include_hidden=include_hidden))

            if method == "mlui.snapshot":
                backend = self.client.request("snapshot", params, req_id=req_id)
                return _result(req_id, backend)

            if method in {
                "mlui.focus.list",
                "mlui.focus.get",
                "mlui.focus.tree",
                "mlui.focus.search",
                "mlui.focus.action",
            }:
                backend_method = method[len("mlui.") :]
                backend = self.client.request(backend_method, params, req_id=req_id)
                return _result(req_id, backend)

            if method == "mlui.focus.detail":
                page_id = _trim(params.get("page", "")) or _trim(params.get("page_id", ""))
                kind = _trim(params.get("kind", ""))
                key = _trim(params.get("key", ""))
                return _result(req_id, self.focus_detail(page_id, kind, key))

            if method in {"mlui.click", "mlui.set", "mlui.key", "mlui.mouse"}:
                backend_method = method.split(".", 1)[1]
                backend = self.client.request(backend_method, params, req_id=req_id)
                return _result(req_id, backend)

            if method == "fs.pwd":
                return _result(req_id, {"cwd": self.cwd})

            if method == "fs.cd":
                self.ensure_snapshot()
                target = _normalize_fs_path(_trim(params.get("path", "")), self.cwd)
                if not self.index.exists(target):
                    return _error(req_id, -32000, f"path not found: {target}")
                self.cwd = target
                return _result(req_id, {"cwd": self.cwd})

            if method == "fs.ls":
                self.ensure_snapshot()
                path = _normalize_fs_path(_trim(params.get("path", "")), self.cwd)
                depth = int(params.get("depth", 1))
                limit = int(params.get("limit", 200))
                return _result(req_id, self.index.ls(path, depth, limit))

            if method == "fs.tree":
                self.ensure_snapshot()
                path = _normalize_fs_path(_trim(params.get("path", "")), self.cwd)
                depth = int(params.get("depth", 2))
                limit = int(params.get("limit", 400))
                return _result(req_id, self.index.tree(path, depth, limit))

            if method == "fs.find":
                self.ensure_snapshot()
                pattern = _trim(params.get("pattern", ""))
                base_path = _trim(params.get("path", "")) or self.cwd
                field = _trim(params.get("field", "any"))
                regex = bool(params.get("regex", False))
                limit = int(params.get("limit", 100))
                return _result(req_id, self.index.find(pattern, base_path, field, regex, limit))

            if method == "mlui.click_path":
                self.ensure_snapshot()
                fs_path = _normalize_fs_path(_trim(params.get("path", "")), self.cwd)
                target = self.index.choose_click_target(fs_path)
                if not target:
                    return _error(req_id, -32000, f"no clickable target under: {fs_path}")
                backend = self.client.request(
                    "click",
                    {
                        "path": target,
                        "include_hidden": bool(params.get("include_hidden", True)),
                    },
                    req_id=req_id,
                )
                return _result(req_id, {"fs_path": fs_path, "target": target, "backend": backend})

            return _error(req_id, -32601, f"method not found: {method}")
        except Exception as exc:  # noqa: BLE001
            return _error(req_id, -32000, str(exc))


def _parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="MCP bridge for MLUI with path navigation")
    p.add_argument("--host", default=os.getenv("MLUI_HOST", "127.0.0.1"))
    p.add_argument("--port", type=int, default=int(os.getenv("MLUI_PORT", "8082")))
    p.add_argument("--timeout", type=float, default=float(os.getenv("MLUI_TIMEOUT", "5.0")))
    return p.parse_args()


def main() -> int:
    args = _parse_args()
    server = MluiMcpServer(args.host, args.port, args.timeout)

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
