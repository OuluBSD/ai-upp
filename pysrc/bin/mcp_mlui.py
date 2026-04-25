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
        self.current_focus_page = ""
        self.session_token = ""

    def _session_path(self) -> str:
        return f"/tmp/mlui_session_{self.session_token}.json"

    def _session_load(self) -> None:
        if not self.session_token:
            return
        path = self._session_path()
        try:
            with open(path, "r", encoding="utf-8") as f:
                data = json.load(f)
            page = data.get("current_focus_page", "")
            if isinstance(page, str) and page:
                self.current_focus_page = page
        except (OSError, ValueError):
            pass

    def _session_save(self) -> None:
        if not self.session_token:
            return
        path = self._session_path()
        try:
            with open(path, "w", encoding="utf-8") as f:
                json.dump({"current_focus_page": self.current_focus_page}, f)
        except OSError:
            pass

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

    def _focus_get_result(self, page: str, req_id: str = "mcp-focus-get") -> Dict[str, Any]:
        backend = self.client.request("focus.get", {"page": page}, req_id=req_id)
        if not backend.get("ok"):
            raise RuntimeError(_trim(backend.get("error", "focus.get failed")))

        result = backend.get("result")
        if not isinstance(result, dict):
            raise RuntimeError("focus.get result is not object")
        return result

    def _resolve_focus_page(self, params: Dict[str, Any], required: bool = True) -> str:
        page = (
            _trim(params.get("page", ""))
            or _trim(params.get("page_id", ""))
            or self.current_focus_page
        )
        if required and not page:
            raise RuntimeError("page is required (or set current with mlui.focus.page.set)")
        return page

    def focus_page_set(self, page: str) -> Dict[str, Any]:
        result = self._focus_get_result(page, req_id="mcp-focus-page-set")
        resolved_id = _trim(result.get("id", "")) or _trim(page)
        self.current_focus_page = resolved_id
        self._session_save()
        return self.focus_page_get()

    def focus_page_get(self) -> Dict[str, Any]:
        page = _trim(self.current_focus_page)
        if not page:
            return {"current_page": "", "set": False}

        result = self._focus_get_result(page, req_id="mcp-focus-page-get")

        actions = result.get("actions")
        if not isinstance(actions, list):
            actions = []
        runtime = result.get("runtime")
        if not isinstance(runtime, dict):
            runtime = {}
        values = runtime.get("values")
        if not isinstance(values, list):
            values = []
        ctrls = runtime.get("ctrls")
        if not isinstance(ctrls, list):
            ctrls = []

        resolved_id = _trim(result.get("id", "")) or page
        self.current_focus_page = resolved_id
        self._session_save()
        workflow = result.get("workflow")
        if not isinstance(workflow, dict):
            workflow = {}
        runtime_last = runtime.get("last_result")
        if not isinstance(runtime_last, dict):
            runtime_last = {}
        history = runtime.get("action_history")
        if not isinstance(history, list):
            history = []
        return {
            "current_page": resolved_id,
            "set": True,
            "title": _trim(result.get("title", "")),
            "description": _trim(result.get("description", "")),
            "empty_state": _trim(result.get("empty_state", "")),
            "workflow": workflow,
            "counts": {
                "runtime_values": len(values),
                "runtime_ctrls": len(ctrls),
                "actions": len(actions),
            },
            "last_result": runtime_last,
            "action_history_count": len(history),
        }

    def focus_actions(self, page: str) -> Dict[str, Any]:
        result = self._focus_get_result(page, req_id="mcp-focus-actions")
        resolved_id = _trim(result.get("id", "")) or _trim(page)
        self.current_focus_page = resolved_id

        actions = result.get("actions")
        if not isinstance(actions, list):
            actions = []

        out_actions: List[Dict[str, Any]] = []
        for action in actions:
            if not isinstance(action, dict):
                continue
            args_schema = action.get("args_schema")
            if not isinstance(args_schema, list):
                args_schema = []
            writes_to = action.get("writes_to")
            if not isinstance(writes_to, list):
                writes_to = []
            examples = action.get("examples")
            if not isinstance(examples, list):
                examples = []
            out_actions.append(
                {
                    "id": _trim(action.get("id", "")),
                    "label": _trim(action.get("label", "")),
                    "description": _trim(action.get("description", "")),
                    "enabled": bool(action.get("enabled", True)),
                    "requires": _trim(action.get("requires", "")),
                    "disabled_reason": _trim(action.get("disabled_reason", "")),
                    "side_effects": _trim(action.get("side_effects", "")),
                    "writes_to": writes_to,
                    "args_schema": args_schema,
                    "examples": examples,
                }
            )

        runtime = result.get("runtime")
        if not isinstance(runtime, dict):
            runtime = {}
        last_result = runtime.get("last_result")
        if not isinstance(last_result, dict):
            last_result = {}
        history = runtime.get("action_history")
        if not isinstance(history, list):
            history = []

        return {
            "page": resolved_id,
            "count": len(out_actions),
            "actions": out_actions,
            "last_result": last_result,
            "action_history_count": len(history),
        }

    def focus_run(self, page: str, action: str, args: Dict[str, Any]) -> Dict[str, Any]:
        act = _trim(action)
        if not act:
            raise RuntimeError("action is required")

        resolved_page = _trim(page)
        if not resolved_page:
            raise RuntimeError("page is required (or set current with mlui.focus.page.set)")

        self.current_focus_page = resolved_page
        self._session_save()
        backend = self.client.request(
            "focus.action",
            {
                "page": resolved_page,
                "action": act,
                "args": args,
            },
            req_id="mcp-focus-run",
        )
        return {
            "page": resolved_page,
            "action": act,
            "backend": backend,
        }

    def focus_detail(self, page_id: str, kind: str, key: str) -> Dict[str, Any]:
        page = _trim(page_id)
        if not page:
            raise RuntimeError("page is required")
        result = self._focus_get_result(page, req_id="mcp-focus-detail")

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
        content_items = runtime.get("content_items")
        if not isinstance(content_items, list):
            content_items = []

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
        elif k in {"content", "content_item", "path_match"} and q:
            out["item"] = next(
                (
                    x
                    for x in content_items
                    if isinstance(x, dict)
                    and (
                        _trim(x.get("key", "")) == q
                        or (_trim(x.get("type", "")) == k and _trim(x.get("text", "")) == q)
                    )
                ),
                None,
            )
        else:
            out["sample"] = {
                "values": values[:5],
                "ctrls": ctrls[:5],
                "actions": actions[:5],
                "content_items": content_items[:5],
            }

        return out

    def _extract_focus_search_matches(self, backend: Dict[str, Any]) -> List[Dict[str, Any]]:
        if not isinstance(backend, dict) or not backend.get("ok"):
            return []
        result = backend.get("result")
        if isinstance(result, list):
            return [x for x in result if isinstance(x, dict)]
        if isinstance(result, dict):
            arr = result.get("results")
            if isinstance(arr, list):
                return [x for x in arr if isinstance(x, dict)]
        return []

    def _search_kind_rank(self, kind: str) -> int:
        k = _trim(kind).lower()
        if k == "action":
            return 0
        if k == "path_match":
            return 1
        if k == "content_item":
            return 2
        if k == "runtime.value":
            return 3
        if k == "runtime.ctrl":
            return 4
        if k == "page":
            return 5
        return 9

    def _best_focus_search_match(self, matches: List[Dict[str, Any]], query: str = "") -> Dict[str, Any]:
        if not matches:
            return {}

        q = _trim(query).lower()

        def score_tuple(m: Dict[str, Any]) -> tuple:
            kind_rank = self._search_kind_rank(_trim(m.get("kind", "")))
            key = _trim(m.get("key", ""))
            key_lower = key.lower()
            # 0 = exact key match, 1 = key starts with query, 2 = no prefix match
            if q and key_lower == q:
                key_match = 0
            elif q and key_lower.startswith(q):
                key_match = 1
            else:
                key_match = 2
            text_len = len(_trim(m.get("text", "")))
            score = m.get("score")
            score_v = int(score) if isinstance(score, (int, float)) else 9999
            return (kind_rank, key_match, text_len, score_v, key)

        return min(matches, key=score_tuple)

    def _select_suggestion_from_match(self, match: Dict[str, Any]) -> Dict[str, Any]:
        if not isinstance(match, dict) or not match:
            return {}
        page_id = _trim(match.get("page_id", ""))
        kind = _trim(match.get("kind", "")).lower()
        key = _trim(match.get("key", ""))
        action = _trim(match.get("action", ""))
        args = match.get("args")
        if not isinstance(args, dict):
            args = {}

        # Explicit action field takes priority (legacy path)
        if action:
            return {
                "call": "mlui.focus.action",
                "page": page_id,
                "action": action,
                "args_example": args,
            }

        # kind="action": key holds the action id
        if kind == "action" and key:
            return {
                "call": "mlui.focus.action",
                "page": page_id,
                "action": key,
                "args_example": args,
            }

        # kind="runtime.value": point caller at the page+key
        if kind == "runtime.value" and (page_id or key):
            return {
                "call": "mlui.focus.get",
                "page": page_id,
                "note": "value key: " + key,
            }

        if kind == "path_match":
            path = _trim(match.get("path", ""))
            if not path:
                if key.startswith("scan:"):
                    path = key[len("scan:"):]
                elif key.startswith("path:"):
                    path = key[len("path:"):]
            if path:
                return {
                    "call": "mlui.focus.action",
                    "page": page_id or "file_tree",
                    "action": "select",
                    "args_example": {"path": path},
                }
        return {}

    def handle(self, req: Dict[str, Any]) -> Dict[str, Any]:
        req_id = req.get("id")
        method = req.get("method")
        params = req.get("params") or {}

        if not isinstance(method, str) or not method:
            return _error(req_id, -32600, "invalid request: missing method")
        if not isinstance(params, dict):
            return _error(req_id, -32602, "invalid params")

        session = _trim(params.get("session", ""))
        if session and session != self.session_token:
            self.session_token = session
            self._session_load()

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
                            {"method": "mlui.focus.page.set", "params": {"page": "file_tree"}},
                            {"method": "mlui.focus.actions", "params": {}},
                            {"method": "mlui.focus.run", "params": {"action": "reload"}},
                            {"method": "mlui.focus.search", "params": {"page": "file_tree", "query": "*Main.cpp*", "limit": 10}},
                            {
                                "method": "mlui.focus.batch",
                                "params": {
                                    "steps": [
                                        {"page": "file_tree", "action": "select", "args": {"path": "uppsrc/Overviewer/Main.cpp"}},
                                        {"page": "active_file", "action": "set_priority", "args": {"value": 3}},
                                    ]
                                },
                            },
                            {"method": "mlui.focus.detail", "params": {"page": "file_tree"}},
                            {"method": "mlui.focus.action", "params": {"page": "file_tree", "action": "reload"}},
                            {
                                "method": "mlui.focus.run",
                                "params": {
                                    "action": "select",
                                    "args": {"path": "uppsrc/Overviewer/Main.cpp"},
                                },
                            },
                            {
                                "method": "mlui.focus.run",
                                "params": {
                                    "action": "set_priority",
                                    "args": {"value": 3},
                                },
                            },
                        ],
                        "methods": {
                            "mlui.focus.list": {
                                "description": "List all registered focus pages with metadata.",
                                "params": {}
                            },
                            "mlui.focus.get": {
                                "description": "Get full details for one focus page: runtime values, ctrls, actions with schemas.",
                                "params": {
                                    "page": {"type": "string", "required": True, "description": "Focus page id, e.g. 'issue_board'"}
                                }
                            },
                            "mlui.focus.tree": {
                                "description": "Compact text tree of all focus pages and their action/value counts.",
                                "params": {
                                    "page": {"type": "string", "required": False, "description": "If set, show only this page"},
                                    "depth": {"type": "integer", "required": False, "description": "Tree depth (default 2)"}
                                }
                            },
                            "mlui.focus.search": {
                                "description": "Search across focus pages. Returns matches with kind, best_match, and select_suggestion.",
                                "params": {
                                    "query": {"type": "string", "required": True, "description": "Search term"},
                                    "page": {"type": "string", "required": False, "description": "Restrict search to one page id"},
                                    "limit": {"type": "integer", "required": False, "description": "Max results (default 10)"},
                                    "raw": {"type": "boolean", "required": False, "description": "Return raw backend response without best_match/select_suggestion"}
                                }
                            },
                            "mlui.focus.action": {
                                "description": "Run a named action on a focus page.",
                                "params": {
                                    "page": {"type": "string", "required": True, "description": "Focus page id"},
                                    "action": {"type": "string", "required": True, "description": "Action id, e.g. 'select_issue'"},
                                    "args": {"type": "object", "required": False, "description": "Action arguments as key-value pairs"}
                                }
                            },
                            "mlui.focus.run": {
                                "description": "Run an action using the current focus page (set via mlui.focus.page.set or session).",
                                "params": {
                                    "action": {"type": "string", "required": True, "description": "Action id"},
                                    "args": {"type": "object", "required": False, "description": "Action arguments"},
                                    "page": {"type": "string", "required": False, "description": "Override current page for this call"}
                                }
                            },
                            "mlui.focus.batch": {
                                "description": "Run multiple actions sequentially. Returns per-step results.",
                                "params": {
                                    "steps": {
                                        "type": "array",
                                        "required": True,
                                        "description": "List of steps. Each step: {page, action, args}",
                                        "items": {"type": "object", "properties": {"page": "string", "action": "string", "args": "object"}}
                                    },
                                    "continue_on_error": {"type": "boolean", "required": False, "description": "Continue remaining steps if one fails (default false)"}
                                }
                            },
                            "mlui.focus.actions": {
                                "description": "List all actions for a page with schemas, side_effects, and recent history.",
                                "params": {
                                    "page": {"type": "string", "required": True, "description": "Focus page id"}
                                }
                            },
                            "mlui.focus.detail": {
                                "description": "Get compact detail for a page, or drill into one value/ctrl/action by kind+key.",
                                "params": {
                                    "page": {"type": "string", "required": True, "description": "Focus page id"},
                                    "kind": {"type": "string", "required": False, "description": "Item kind: 'action', 'value', 'ctrl'"},
                                    "key": {"type": "string", "required": False, "description": "Item key within the page"}
                                }
                            },
                            "mlui.focus.page.set": {
                                "description": "Set the current focus page for this session. Subsequent mlui.focus.run calls use it.",
                                "params": {
                                    "page": {"type": "string", "required": True, "description": "Focus page id to set as current"},
                                    "session": {"type": "string", "required": False, "description": "Session token for cross-process persistence"}
                                }
                            },
                            "mlui.focus.page.get": {
                                "description": "Get the currently active focus page for this session.",
                                "params": {
                                    "session": {"type": "string", "required": False, "description": "Session token to restore persisted page"}
                                }
                            },
                            "mlui.focus.validate": {
                                "description": "Validate a focus page registration completeness. Returns warnings for missing schemas, effects, and raw pointer refs.",
                                "params": {
                                    "page": {"type": "string", "required": True, "description": "Focus page id to validate"}
                                }
                            }
                        },
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
                            "mlui.focus.batch",
                            "mlui.focus.detail",
                            "mlui.focus.page.set",
                            "mlui.focus.page.get",
                            "mlui.focus.actions",
                            "mlui.focus.run",
                            "mlui.focus.validate",
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

            if method == "mlui.focus.get":
                call_params = dict(params)
                page = self._resolve_focus_page(call_params, required=False)
                if page:
                    call_params["page"] = page
                backend = self.client.request("focus.get", call_params, req_id=req_id)
                if backend.get("ok"):
                    result = backend.get("result")
                    if isinstance(result, dict):
                        resolved_id = _trim(result.get("id", "")) or _trim(call_params.get("page", ""))
                        if resolved_id:
                            self.current_focus_page = resolved_id
                            self._session_save()
                            return _result(req_id, backend)
            if method == "mlui.focus.search":
                backend = self.client.request("focus.search", params, req_id=req_id)
                if bool(params.get("raw", False)):
                    return _result(req_id, backend)

                matches = self._extract_focus_search_matches(backend)
                best = self._best_focus_search_match(matches, query=_trim(params.get("query", "")))
                suggestion = self._select_suggestion_from_match(best)
                return _result(
                    req_id,
                    {
                        "backend": backend,
                        "count": len(matches),
                        "matches": matches,
                        "best_match": best,
                        "select_suggestion": suggestion,
                    },
                )

            if method in {
                "mlui.focus.list",
                "mlui.focus.tree",
                "mlui.focus.action",
                "mlui.focus.batch",
            }:
                backend_method = method[len("mlui.") :]
                backend = self.client.request(backend_method, params, req_id=req_id)
                return _result(req_id, backend)

            if method == "mlui.focus.page.set":
                page = _trim(params.get("page", "")) or _trim(params.get("page_id", ""))
                if not page:
                    raise RuntimeError("page is required")
                return _result(req_id, self.focus_page_set(page))

            if method == "mlui.focus.page.get":
                return _result(req_id, self.focus_page_get())

            if method == "mlui.focus.actions":
                page = self._resolve_focus_page(params, required=True)
                return _result(req_id, self.focus_actions(page))

            if method == "mlui.focus.run":
                page = self._resolve_focus_page(params, required=False)
                action = _trim(params.get("action", ""))
                args = params.get("args")
                if not isinstance(args, dict):
                    args = {}
                return _result(req_id, self.focus_run(page, action, args))

            if method == "mlui.focus.detail":
                page_id = self._resolve_focus_page(params, required=True)
                kind = _trim(params.get("kind", ""))
                key = _trim(params.get("key", ""))
                return _result(req_id, self.focus_detail(page_id, kind, key))

            if method == "mlui.focus.validate":
                call_params = dict(params)
                page = self._resolve_focus_page(call_params, required=False)
                if page:
                    call_params["page"] = page
                backend = self.client.request("focus.validate", call_params, req_id=req_id)
                return _result(req_id, backend)

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
