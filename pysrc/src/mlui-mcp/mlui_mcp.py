#!/usr/bin/env python3
import argparse
import json
import os
import socket
import sys
from typing import Any, Dict, Optional


def make_result(req_id: Any, result: Any) -> Dict[str, Any]:
    return {"jsonrpc": "2.0", "id": req_id, "result": result}


def make_error(req_id: Any, code: int, message: str) -> Dict[str, Any]:
    return {
        "jsonrpc": "2.0",
        "id": req_id,
        "error": {"code": code, "message": message},
    }


class MluiClient:
    def __init__(self, host: str, port: int, timeout: float) -> None:
        self.host = host
        self.port = port
        self.timeout = timeout

    def request(self, method: str, params: Optional[Dict[str, Any]] = None, req_id: Any = None) -> Dict[str, Any]:
        payload = {
            "id": req_id,
            "method": method,
            "params": params or {},
        }
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


def handle_rpc(client: MluiClient, req: Dict[str, Any]) -> Dict[str, Any]:
    req_id = req.get("id")
    method = req.get("method")
    params = req.get("params") or {}

    if not isinstance(method, str) or not method:
        return make_error(req_id, -32600, "Invalid request: missing method")
    if not isinstance(params, dict):
        return make_error(req_id, -32602, "Invalid params")

    if method == "mcp.ping":
        return make_result(req_id, {"text": "pong"})

    if method == "mcp.capabilities":
        return make_result(
            req_id,
            {
                "protocol": "jsonrpc-2.0",
                "methods": [
                    "mcp.ping",
                    "mcp.capabilities",
                    "mlui.snapshot",
                    "mlui.click",
                    "mlui.set",
                    "mlui.key",
                    "mlui.mouse",
                    "mlui.raw",
                ],
            },
        )

    if method == "mlui.raw":
        target_method = params.get("method")
        target_params = params.get("params") or {}
        if not isinstance(target_method, str) or not target_method:
            return make_error(req_id, -32602, "mlui.raw requires params.method")
        if not isinstance(target_params, dict):
            return make_error(req_id, -32602, "mlui.raw params.params must be object")
        backend = client.request(target_method, target_params, req_id=req_id)
        return make_result(req_id, backend)

    method_map = {
        "mlui.snapshot": "snapshot",
        "mlui.click": "click",
        "mlui.set": "set",
        "mlui.key": "key",
        "mlui.mouse": "mouse",
    }

    if method not in method_map:
        return make_error(req_id, -32601, f"Method not found: {method}")

    backend = client.request(method_map[method], params, req_id=req_id)
    return make_result(req_id, backend)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="MLUI MCP bridge")
    parser.add_argument("--host", default=os.getenv("MLUI_HOST", "127.0.0.1"))
    parser.add_argument("--port", type=int, default=int(os.getenv("MLUI_PORT", "8082")))
    parser.add_argument("--timeout", type=float, default=float(os.getenv("MLUI_TIMEOUT", "5.0")))
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    client = MluiClient(args.host, args.port, args.timeout)

    for line in sys.stdin:
        line = line.strip()
        if not line:
            continue

        try:
            req = json.loads(line)
        except Exception as exc:  # noqa: BLE001
            resp = make_error(None, -32700, f"Parse error: {exc}")
            sys.stdout.write(json.dumps(resp, ensure_ascii=False) + "\n")
            sys.stdout.flush()
            continue

        if not isinstance(req, dict):
            resp = make_error(None, -32600, "Invalid request object")
            sys.stdout.write(json.dumps(resp, ensure_ascii=False) + "\n")
            sys.stdout.flush()
            continue

        try:
            resp = handle_rpc(client, req)
        except Exception as exc:  # noqa: BLE001
            resp = make_error(req.get("id"), -32000, str(exc))

        sys.stdout.write(json.dumps(resp, ensure_ascii=False) + "\n")
        sys.stdout.flush()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
