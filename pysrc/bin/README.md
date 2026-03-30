# pysrc/bin

This directory contains runnable helper scripts. More scripts are expected to be added here over time.

## Scripts

- `mlui_uword.py`: Build + run `examples/UWord` with MLUI viewer orchestration.
- `mlui_bombs.py`: Build + run `examples/Bombs` with MLUI viewer orchestration.
- `mcp_build.py`: JSON-RPC (MCP-style) build/launch helper using `script/build.py`.
- `mcp_mlui.py`: JSON-RPC (MCP-style) MLUI bridge with path navigation (`pwd/cd/ls/tree/find`) and click by path.

## Register To AI CLIs

All examples below register two stdio MCP servers:

- `aiupp-build` -> `python3 /common/active/sblo/Dev/ai-upp/pysrc/bin/mcp_build.py`
- `aiupp-mlui` -> `python3 /common/active/sblo/Dev/ai-upp/pysrc/bin/mcp_mlui.py`

### Codex

```bash
codex mcp add aiupp-build -- python3 /common/active/sblo/Dev/ai-upp/pysrc/bin/mcp_build.py
codex mcp add aiupp-mlui -- python3 /common/active/sblo/Dev/ai-upp/pysrc/bin/mcp_mlui.py
codex mcp list
```

### Gemini

```bash
gemini mcp add -s project -t stdio aiupp-build python3 /common/active/sblo/Dev/ai-upp/pysrc/bin/mcp_build.py
gemini mcp add -s project -t stdio aiupp-mlui  python3 /common/active/sblo/Dev/ai-upp/pysrc/bin/mcp_mlui.py
gemini mcp list
```

### Qwen

```bash
qwen mcp add -s project -t stdio aiupp-build python3 /common/active/sblo/Dev/ai-upp/pysrc/bin/mcp_build.py
qwen mcp add -s project -t stdio aiupp-mlui  python3 /common/active/sblo/Dev/ai-upp/pysrc/bin/mcp_mlui.py
qwen mcp list
```

### Claude

```bash
claude mcp add -s project -t stdio aiupp-build -- python3 /common/active/sblo/Dev/ai-upp/pysrc/bin/mcp_build.py
claude mcp add -s project -t stdio aiupp-mlui  -- python3 /common/active/sblo/Dev/ai-upp/pysrc/bin/mcp_mlui.py
claude mcp list
```

## `mcp_build.py`

Run:

```bash
python3 pysrc/bin/mcp_build.py
```

Supported methods:

- `mcp.ping`
- `mcp.capabilities`
- `build.list_conf`
- `build.compile`
- `build.launch`
- `build.status`
- `build.stop`
- `build.logs`
- `build.running`

Example request/response flow (line-delimited JSON):

```bash
printf '%s\n' \
'{"jsonrpc":"2.0","id":1,"method":"build.list_conf","params":{"package":"examples/Bombs"}}' \
'{"jsonrpc":"2.0","id":2,"method":"build.launch","params":{"package":"examples/Bombs","mainconfig":0,"build":true,"mlui_server":"127.0.0.1:8082"}}' \
| python3 pysrc/bin/mcp_build.py
```

## `mcp_mlui.py`

Run:

```bash
python3 pysrc/bin/mcp_mlui.py --host 127.0.0.1 --port 8082
```

Supported methods:

- `mcp.ping`
- `mcp.help`
- `mcp.capabilities`
- `mlui.refresh`
- `mlui.snapshot`
- `mlui.click`
- `mlui.set`
- `mlui.key`
- `mlui.mouse`
- `mlui.click_path`
- `mlui.focus.list`
- `mlui.focus.get`
- `mlui.focus.tree`
- `mlui.focus.search`
- `mlui.focus.action`
- `mlui.focus.detail`
- `fs.pwd`
- `fs.cd`
- `fs.ls`
- `fs.tree`
- `fs.find`

Notes:

- Path navigation is virtualized from the latest snapshot.
- `fs.find` supports simple wildcard search via `*` (for example `*Cell*`).
- `mlui.click_path` picks the best clickable target under a path (useful for semantic parent nodes).
- `mlui.focus.detail` returns a compact per-page summary (and optionally one value/ctrl/action item).

Example:

```bash
printf '%s\n' \
'{"jsonrpc":"2.0","id":1,"method":"mlui.refresh","params":{"include_hidden":true}}' \
'{"jsonrpc":"2.0","id":2,"method":"fs.tree","params":{"path":"/","depth":2}}' \
'{"jsonrpc":"2.0","id":3,"method":"fs.find","params":{"pattern":"*Cell 0,0*","field":"semantic","limit":5}}' \
| python3 pysrc/bin/mcp_mlui.py --host 127.0.0.1 --port 8082
```
