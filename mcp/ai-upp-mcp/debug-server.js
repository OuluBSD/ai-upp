#!/usr/bin/env node

// Debug version of the MCP server to troubleshoot execution
import { spawn } from 'child_process';

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
        console.error('RECEIVED LINE:', line.trim()); // Log to stderr to not interfere with stdout responses
        const request = JSON.parse(line.trim());
        console.error('PARSED REQUEST:', request); // Log to stderr
        
        // Handle the request
        let response;
        if (request.method === 'mcp.tools.list') {
          response = {
            jsonrpc: '2.0',
            id: request.id,
            result: {
              tools: [
                { name: 'evolution_summary', description: 'Debug test tool' }
              ]
            }
          };
        } else if (request.method === 'mcp.tool.execute') {
          console.error('EXECUTING TOOL:', request.params.name); // Log to stderr
          
          // Simulate the execution
          try {
            const result = await executeCliCommand(['--workspace-root', request.params.arguments.workspace_root, '--json', 'evolution_summary']);
            console.error('CLI RESULT:', JSON.stringify(result)); // Log to stderr
            response = {
              jsonrpc: '2.0',
              id: request.id,
              result: result
            };
          } catch (cliError) {
            console.error('CLI ERROR:', cliError.message); // Log to stderr
            response = {
              jsonrpc: '2.0',
              id: request.id,
              error: { code: -32603, message: cliError.message }
            };
          }
        } else {
          response = {
            jsonrpc: '2.0',
            id: request.id,
            error: { code: -32601, message: `Method not found: ${request.method}` }
          };
        }
        
        console.error('SENDING RESPONSE:', JSON.stringify(response)); // Log to stderr
        process.stdout.write(JSON.stringify(response) + '\n');
      } catch (error) {
        // Write error response to stdout
        const errorResponse = {
          jsonrpc: '2.0',
          id: null,
          error: { code: -32700, message: `Parse error: ${error.message}` }
        };
        console.error('PARSE ERROR:', error.message); // Log to stderr
        process.stdout.write(JSON.stringify(errorResponse) + '\n');
      }
    }
  }
});

// Handle end of input - process any remaining buffer
process.stdin.on('end', async () => {
  // Process any remaining data in buffer
  if (buffer.trim()) {
    try {
      const request = JSON.parse(buffer.trim());
      const response = createResponse(request);
      process.stdout.write(JSON.stringify(response) + '\n');
    } catch (error) {
      // Write error response to stdout
      const errorResponse = {
        jsonrpc: '2.0',
        id: null,
        error: { code: -32700, message: `Parse error: ${error.message}` }
      };
      process.stdout.write(JSON.stringify(errorResponse) + '\n');
    }
  }
  process.exit(0);
});

// Function to execute CLI command and return result (simplified for debugging)
function executeCliCommand(cmdArgs) {
  return new Promise((resolve, reject) => {
    console.error('SPAWNING:', './theide-cli', cmdArgs); // Log to stderr
    const child = spawn('./theide-cli', cmdArgs, { cwd: process.cwd() });
    
    let stdout = '';
    let stderr = '';
    
    child.stdout.on('data', (data) => {
      console.error('CLI STDOUT:', data.toString()); // Log to stderr
      stdout += data.toString();
    });
    
    child.stderr.on('data', (data) => {
      console.error('CLI STDERR:', data.toString()); // Log to stderr
      stderr += data.toString();
    });
    
    child.on('error', (err) => {
      console.error('CLI SPAWN ERROR:', err.message); // Log to stderr
      reject(new Error(`Failed to start theide-cli: ${err.message}`));
    });
    
    child.on('close', (code) => {
      console.error('CLI CLOSED WITH CODE:', code); // Log to stderr
      if (code !== 0) {
        reject(new Error(`Command failed with code ${code}: ${stderr}`));
      } else {
        try {
          console.error('CLI OUTPUT TO PARSE:', stdout); // Log to stderr
          const result = JSON.parse(stdout);
          console.error('CLI PARSED RESULT:', result); // Log to stderr
          resolve(result);
        } catch (parseErr) {
          reject(new Error(`Failed to parse JSON response: ${parseErr.message}\n${stdout}`));
        }
      }
    });
  });
}