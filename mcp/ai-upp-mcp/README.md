# AI-UPP MCP Server

This directory contains a Node.js-based MCP (Model Context Protocol) server that exposes AI-UPP CLI functionality to LLM agents like Qwen, Claude, and others.

## Installation and Setup

1. Navigate to this directory:
   ```bash
   cd mcp/ai-upp-mcp
   ```

2. Install dependencies:
   ```bash
   npm install
   ```

3. Make sure the `theide-cli` binary is available in your PATH or in this directory as an executable file.

## Running the Server

Start the server directly:
```bash
npm start
# or
node server.js
```

The server implements the MCP protocol over stdin/stdout, processing JSON-RPC requests and returning responses in the same format.

## Available Tools

The server exposes the following tools:

### `workspace_overview`
- Description: Get high-level stats and graph info about the current workspace.
- Parameters:
  - `workspace_root` (string, required): Path to the workspace root

### `optimization_proposal`
- Description: Generate an AI optimization proposal for a package.
- Parameters:
  - `workspace_root` (string, required): Path to the workspace root
  - `package` (string, required): Package name to generate proposal for
  - `max_actions` (number, optional): Maximum number of actions in proposal
  - `with_futures` (boolean, optional): Include future scenario planning

### `explore_futures`
- Description: Explore multi-branch futures for the current negotiated scenario.
- Parameters:
  - `workspace_root` (string, required): Path to the workspace root

### `apply_scenario`
- Description: Apply an existing scenario plan.
- Parameters:
  - `workspace_root` (string, required): Path to the workspace root
  - `plan_file` (string, required): Path to the plan file to apply

### `revert_patch`
- Description: Revert a previously applied patch.
- Parameters:
  - `workspace_root` (string, required): Path to the workspace root
  - `patch` (string, required): Unified diff text to revert

### `evolution_summary`
- Description: Summarize historical evolution of the codebase.
- Parameters:
  - `workspace_root` (string, required): Path to the workspace root

### `lifecycle_status`
- Description: Report lifecycle phase, drift and stability.
- Parameters:
  - `workspace_root` (string, required): Path to the workspace root

## Integration with MCP Clients

To use this server with MCP-aware agents like Qwen:

1. Register the server in your MCP client configuration
2. Reference the `mcp.json` manifest file to describe the available tools
3. Start the agent with permission to call the ai-upp tools

Example for Qwen:
```bash
qwen mcp  # Add this server to your MCP configuration
qwen -i "You can use the ai-upp MCP tools to analyze and refactor this workspace..." \
  --allowed-mcp-server-names ai-upp
```

## Implementation Details

- The server communicates with AI-UPP via the CLI (`theide-cli`) in `--json` mode
- All operations are deterministic and avoid any GUI dependencies
- Parallel CLI operations are supported using `Promise.all`
- The server properly handles ongoing operations when stdin closes
- JSON-RPC requests are processed asynchronously while maintaining proper request/response correlation