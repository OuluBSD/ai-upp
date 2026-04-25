# mlui-mcp

Small stdin/stdout JSON-RPC bridge to AI-UBB MLUI runtime.

## Run

```bash
python3 pysrc/src/mlui-mcp/mlui_mcp.py --host 127.0.0.1 --port 8082
```

Environment variables are also supported:
- `MLUI_HOST`
- `MLUI_PORT`
- `MLUI_TIMEOUT`

## Supported methods
- `mcp.ping`
- `mcp.capabilities`
- `mlui.snapshot`
- `mlui.click`
- `mlui.set`
- `mlui.key`
- `mlui.mouse`
- `mlui.raw`
