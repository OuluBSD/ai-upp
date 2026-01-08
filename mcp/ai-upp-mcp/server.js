#!/usr/bin/env node

import { spawn } from 'child_process';
import readline from 'readline';

// MCP Protocol handler
const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout
});

const tools = {
  workspace_overview: {
    description: "Get high-level stats and graph info about the current workspace.",
    input_schema: {
      type: "object",
      properties: {
        workspace_root: { type: "string" }
      },
      required: ["workspace_root"]
    }
  },
  optimization_proposal: {
    description: "Generate an AI optimization proposal for a package.",
    input_schema: {
      type: "object",
      properties: {
        workspace_root: { type: "string" },
        package: { type: "string" },
        max_actions: { type: "number" },
        with_futures: { type: "boolean" }
      },
      required: ["workspace_root", "package"]
    }
  },
  explore_futures: {
    description: "Explore multi-branch futures for the current negotiated scenario.",
    input_schema: {
      type: "object",
      properties: {
        workspace_root: { type: "string" }
      },
      required: ["workspace_root"]
    }
  },
  apply_scenario: {
    description: "Apply an existing scenario plan.",
    input_schema: {
      type: "object",
      properties: {
        workspace_root: { type: "string" },
        plan_file: { type: "string" }
      },
      required: ["workspace_root", "plan_file"]
    }
  },
  revert_patch: {
    description: "Revert a previously applied patch.",
    input_schema: {
      type: "object",
      properties: {
        workspace_root: { type: "string" },
        patch: { type: "string" }
      },
      required: ["workspace_root", "patch"]
    }
  },
  evolution_summary: {
    description: "Summarize historical evolution of the codebase.",
    input_schema: {
      type: "object",
      properties: {
        workspace_root: { type: "string" }
      },
      required: ["workspace_root"]
    }
  },
  lifecycle_status: {
    description: "Report lifecycle phase, drift and stability.",
    input_schema: {
      type: "object",
      properties: {
        workspace_root: { type: "string" }
      },
      required: ["workspace_root"]
    }
  },
  list_playbooks: {
    description: "List available playbooks for automation.",
    input_schema: {
      type: "object",
      properties: {
        workspace_root: { type: "string" }
      },
      required: ["workspace_root"]
    }
  },
  run_playbook: {
    description: "Run a specific playbook.",
    input_schema: {
      type: "object",
      properties: {
        workspace_root: { type: "string" },
        playbook_id: { type: "string" }
      },
      required: ["workspace_root", "playbook_id"]
    }
  }
};

// Function to validate input arguments based on schema
function validateInput(toolName, params) {
  const tool = tools[toolName];
  if (!tool) {
    return { valid: false, error: `Unknown tool: ${toolName}` };
  }

  const { required, properties } = tool.input_schema;
  
  // Check required parameters
  for (const param of required) {
    if (!(param in params)) {
      return { valid: false, error: `Missing required parameter: ${param}` };
    }
  }
  
  // Validate parameter types
  for (const [key, value] of Object.entries(params)) {
    if (!(key in properties)) {
      continue; // Skip validation for extra parameters
    }
    
    const propDef = properties[key];
    if (typeof value !== propDef.type) {
      return { valid: false, error: `Invalid type for parameter ${key}: expected ${propDef.type}, got ${typeof value}` };
    }
  }
  
  return { valid: true };
}

// Function to execute CLI command and return result
async function executeCliCommand(cmdArgs) {
  return new Promise((resolve, reject) => {
    // Use the local theide-cli executable
    const child = spawn('./theide-cli', cmdArgs, { cwd: process.cwd() });

    let stdout = '';
    let stderr = '';

    child.stdout.on('data', (data) => {
      stdout += data.toString();
    });

    child.stderr.on('data', (data) => {
      stderr += data.toString();
    });

    child.on('error', (err) => {
      reject(new Error(`Failed to start theide-cli: ${err.message}`));
    });

    child.on('close', (code) => {
      if (code !== 0) {
        reject(new Error(`Command failed with code ${code}: ${stderr}`));
      } else {
        try {
          const result = JSON.parse(stdout);
          resolve(result);
        } catch (parseErr) {
          reject(new Error(`Failed to parse JSON response: ${parseErr.message}\n${stdout}`));
        }
      }
    });
  });
}

// Tool implementations
const toolHandlers = {
  async workspace_overview(params) {
    const { workspace_root } = params;
    
    // Run workspace_stats and graph_stats
    const [statsResult, graphResult] = await Promise.all([
      executeCliCommand(["--workspace-root", workspace_root, "--json", "workspace_stats"]),
      executeCliCommand(["--workspace-root", workspace_root, "--json", "graph_stats"])
    ]);
    
    return {
      workspace_stats: statsResult,
      graph_stats: graphResult
    };
  },

  async optimization_proposal(params) {
    const { workspace_root, package: pkg, max_actions, with_futures } = params;
    
    const args = ["--workspace-root", workspace_root, "--json", "export_proposal", "--package", pkg];
    
    if (max_actions !== undefined) {
      args.push("--max_actions", max_actions.toString());
    }
    
    if (with_futures) {
      args.push("--with-futures");
    }
    
    return await executeCliCommand(args);
  },

  async explore_futures(params) {
    const { workspace_root } = params;
    
    return await executeCliCommand(["--workspace-root", workspace_root, "--json", "explore_futures"]);
  },

  async apply_scenario(params) {
    const { workspace_root, plan_file } = params;
    
    return await executeCliCommand(["--workspace-root", workspace_root, "--json", "apply_scenario", "--plan-file", plan_file]);
  },

  async revert_patch(params) {
    const { workspace_root, patch } = params;
    
    // Write patch to temporary file
    const fs = await import('fs');
    const os = await import('os');
    const path = await import('path');
    
    const tmpDir = os.tmpdir();
    const patchFileName = path.join(tmpDir, `ai-upp-patch-${Date.now()}.diff`);
    
    try {
      await fs.promises.writeFile(patchFileName, patch);
      const result = await executeCliCommand(["--workspace-root", workspace_root, "--json", "scenario_revert", "--patch-file", patchFileName]);
      return result;
    } finally {
      // Clean up temporary file
      try {
        await fs.promises.unlink(patchFileName);
      } catch (err) {
        console.error(`Warning: Could not remove temporary patch file: ${err.message}`);
      }
    }
  },

  async evolution_summary(params) {
    const { workspace_root } = params;
    
    return await executeCliCommand(["--workspace-root", workspace_root, "--json", "evolution_summary"]);
  },

  async lifecycle_status(params) {
    const { workspace_root } = params;

    // Get all lifecycle metrics
    const [phaseResult, driftResult, stabilityResult] = await Promise.all([
      executeCliCommand(["--workspace-root", workspace_root, "--json", "lifecycle_phase"]),
      executeCliCommand(["--workspace-root", workspace_root, "--json", "lifecycle_drift"]),
      executeCliCommand(["--workspace-root", workspace_root, "--json", "lifecycle_stability"])
    ]);

    return {
      lifecycle_phase: phaseResult,
      lifecycle_drift: driftResult,
      lifecycle_stability: stabilityResult
    };
  },

  async list_playbooks(params) {
    const { workspace_root } = params;

    return await executeCliCommand(["--workspace-root", workspace_root, "--json", "list_playbooks"]);
  },

  async run_playbook(params) {
    const { workspace_root, playbook_id } = params;

    return await executeCliCommand(["--workspace-root", workspace_root, "--json", "run_playbook", "--playbook_id", playbook_id]);
  }
};

// Handle incoming JSON-RPC requests
let ongoingOperations = 0;

async function handleRequest(request) {
  ongoingOperations++;

  try {
    if (request.method === 'mcp.tools.list') {
      const toolList = Object.keys(tools).map(toolName => ({
        name: toolName,
        description: tools[toolName].description
      }));

      return {
        jsonrpc: '2.0',
        id: request.id,
        result: { tools: toolList }
      };
    } else if (request.method === 'mcp.tool.execute') {
      const { name: toolName, arguments: params } = request.params;

      // Validate input
      const validation = validateInput(toolName, params);
      if (!validation.valid) {
        return {
          jsonrpc: '2.0',
          id: request.id,
          error: { code: -32602, message: validation.error }
        };
      }

      // Execute tool
      if (!toolHandlers[toolName]) {
        return {
          jsonrpc: '2.0',
          id: request.id,
          error: { code: -32601, message: `Method not found: ${toolName}` }
        };
      }

      try {
        const result = await toolHandlers[toolName](params);
        return {
          jsonrpc: '2.0',
          id: request.id,
          result: result
        };
      } catch (error) {
        return {
          jsonrpc: '2.0',
          id: request.id,
          error: { code: -32603, message: error.message }
        };
      }
    } else {
      return {
        jsonrpc: '2.0',
        id: request.id,
        error: { code: -32601, message: `Method not found: ${request.method}` }
      };
    }
  } catch (error) {
    return {
      jsonrpc: '2.0',
      id: request.id,
      error: { code: -32603, message: error.message }
    };
  } finally {
    ongoingOperations--;
  }
}

// Buffer to hold partial JSON data if needed
let buffer = '';

// Process stdin input
process.stdin.setEncoding('utf8');

process.stdin.on('data', async (chunk) => {
  buffer += chunk;

  // Split by newlines to handle multiple JSON objects
  let lines = buffer.split('\n');

  // Process all complete lines
  const completeLines = lines.slice(0, -1);
  buffer = lines[lines.length - 1]; // Keep last (potentially incomplete) line

  for (const line of completeLines) {
    if (line.trim()) {  // Only process non-empty lines
      try {
        const request = JSON.parse(line.trim());
        const response = await handleRequest(request);
        process.stdout.write(JSON.stringify(response) + '\n');
      } catch (error) {
        // Write error response to stdout
        const errorResponse = {
          jsonrpc: '2.0',
          id: request?.id || null,
          error: { code: -32700, message: `Parse error: ${error.message}` }
        };
        process.stdout.write(JSON.stringify(errorResponse) + '\n');
      }
    }
  }
});

process.stdin.on('end', () => {
  // Don't exit immediately if there are ongoing operations
  const checkAndExit = () => {
    if (ongoingOperations === 0) {
      process.exit(0);
    } else {
      // Check again in a bit
      setTimeout(checkAndExit, 100);
    }
  };

  // Check if we can exit now
  checkAndExit();
});