# pysrc/bin

This directory contains runnable helper scripts. More scripts are expected to be added here over time.

## Scripts

- `mlui_uword.py`: Build + run `examples/UWord` with MLUI viewer orchestration.
- `mlui_bombs.py`: Build + run `examples/Bombs` with MLUI viewer orchestration.
- `mcp_build.py`: JSON-RPC (MCP-style) build/launch helper using `script/build.py`.
- `mcp_mlui.py`: JSON-RPC (MCP-style) MLUI bridge with path navigation (`pwd/cd/ls/tree/find`) and click by path.
- `mcp_generalist.py`: Global MCP bridge for multi-agent coordination (Planner/Worker grid).

## Register To AI CLIs

All examples below register the stdio MCP servers globally (user scope):

- `aiupp-build` -> `python3 /common/active/sblo/Dev/ai-upp/pysrc/bin/mcp_build.py`
- `aiupp-mlui` -> `python3 /common/active/sblo/Dev/ai-upp/pysrc/bin/mcp_mlui.py`
- `mcp_generalist` -> `python3 /common/active/sblo/Dev/ai-upp/pysrc/bin/mcp_generalist.py`

### Codex

```bash
codex mcp add mcp_generalist -- python3 /common/active/sblo/Dev/ai-upp/pysrc/bin/mcp_generalist.py
```

### Gemini

```bash
gemini mcp add --scope user mcp_generalist python3 /common/active/sblo/Dev/ai-upp/pysrc/bin/mcp_generalist.py
```

### Qwen

```bash
qwen mcp add --scope user mcp_generalist python3 /common/active/sblo/Dev/ai-upp/pysrc/bin/mcp_generalist.py
```

### Claude

```bash
claude mcp add --scope user mcp_generalist -- python3 /common/active/sblo/Dev/ai-upp/pysrc/bin/mcp_generalist.py
```

## `mcp_generalist.py`

A global bridge allowing one agent (Planner) to coordinate tasks across one or more other agents (Workers) running in different workspaces.

It uses a global storage directory: `~/.config/mcp_generalist/` (Linux) or `%APPDATA%\mcp_generalist` (Windows).

### Workflow

1. **Start Workers**: In each worker workspace, the agent calls `generalist_register_worker` and then enters a blocking `generalist_worker_listen` loop.
2. **Discover**: The planner calls `generalist_list_active_workers` to find listening agents.
3. **Delegate**: The planner calls `generalist_assign` to send a task to a worker identity.
4. **Execute**: The worker's listen call unblocks instantly (via IPC/FIFO on Linux). The worker performs the task.
5. **Report**: The worker calls `generalist_report_task` with results.
6. **Retrieve**: The planner calls `generalist_listen` to wait for and receive the final report.

### Tools for Planners

- `generalist_planner_help`: Detailed coordination guidance.
- `generalist_list_active_workers`: Shows only workers currently holding a listener lock and heartbeating.
- `generalist_list_registered_workers`: Shows all workers known in the last 24 hours.
- `generalist_assign`: Send a task text to a worker identity.
- `generalist_listen`: Block until a specific task is reported complete.
- `generalist_status`: Check current task status and see report if available.

### Tools for Workers

- `generalist_worker_help`: Onboarding instructions for worker agents.
- `generalist_get_identity`: Calculate the exact identity string for registration.
- `generalist_register_worker`: Announce presence to the global grid.
- `generalist_worker_listen`: Block until a task is assigned to your identity.
- `generalist_report_task`: Submit task results back to the planner.

### Global Configuration

- `MCP_GENERALIST_STORAGE`: Environment variable to override the default global config path (useful for sandboxed agents).

---

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
- `mlui.focus.batch`
- `mlui.focus.detail`
- `mlui.focus.page.set`
- `mlui.focus.page.get`
- `mlui.focus.actions`
- `mlui.focus.run`
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
- `mlui.focus.page.set` stores current page context in the MCP bridge.
- `mlui.focus.actions` and `mlui.focus.run` use current page when `page` is omitted.
- `mlui.focus.actions` includes richer action metadata when backend provides it:
  `args_schema`, `requires`, `disabled_reason`, `side_effects`, `writes_to`, `examples`.
- `mlui.focus.page.get` includes `empty_state`, `workflow`, and lightweight action-history summary.
- `mlui.focus.batch` executes multi-step action flows in a single request.
- `mlui.focus.search` (default enriched mode) returns:
  `matches`, `best_match`, and `select_suggestion` in addition to raw backend payload.
- For raw passthrough behavior, call `mlui.focus.search` with `"raw": true`.

Example:

```bash
printf '%s\n' \
'{"jsonrpc":"2.0","id":1,"method":"mlui.refresh","params":{"include_hidden":true}}' \
'{"jsonrpc":"2.0","id":2,"method":"mlui.focus.page.set","params":{"page":"file_tree"}}' \
'{"jsonrpc":"2.0","id":3,"method":"mlui.focus.actions","params":{}}' \
'{"jsonrpc":"2.0","id":4,"method":"mlui.focus.run","params":{"action":"reload"}}' \
'{"jsonrpc":"2.0","id":5,"method":"mlui.focus.batch","params":{"steps":[{"page":"file_tree","action":"select","args":{"path":"uppsrc/Overviewer/Main.cpp"}},{"page":"active_file","action":"set_priority","args":{"value":3}}]}}' \
'{"jsonrpc":"2.0","id":6,"method":"fs.tree","params":{"path":"/","depth":2}}' \
'{"jsonrpc":"2.0","id":7,"method":"fs.find","params":{"pattern":"*Cell 0,0*","field":"semantic","limit":5}}' \
| python3 pysrc/bin/mcp_mlui.py --host 127.0.0.1 --port 8082
```
