#!/usr/bin/env node

import { spawn } from 'child_process';

// Wait function
function wait(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

console.log('Starting comprehensive MCP Server test...\n');

// Spawn the server process
const server = spawn('node', ['server.js'], {
  cwd: './'
});

server.on('error', (err) => {
  console.error('Server error:', err);
});

// Wait a moment for server to start
await wait(500);

// Test 1: List tools
console.log('Test 1: Listing tools...');
const listRequest = {
  jsonrpc: '2.0',
  method: 'mcp.tools.list',
  id: 1
};

server.stdin.write(JSON.stringify(listRequest) + '\n');

// Wait for response
server.stdout.once('data', (data) => {
  console.log('List tools response:', data.toString().trim());
  
  // Test 2: Call workspace_overview
  console.log('\nTest 2: Calling workspace_overview...');
  const toolRequest = {
    jsonrpc: '2.0',
    method: 'mcp.tool.execute',
    id: 2,
    params: {
      name: 'workspace_overview',
      arguments: {
        workspace_root: '/tmp/test-workspace'
      }
    }
  };
  
  server.stdin.write(JSON.stringify(toolRequest) + '\n');
});

// Listen for the second response
server.stdout.once('data', async (data) => {
  console.log('Workspace overview response:', data.toString().trim());
  
  // Test 3: Call optimization_proposal
  console.log('\nTest 3: Calling optimization_proposal...');
  const optRequest = {
    jsonrpc: '2.0',
    method: 'mcp.tool.execute',
    id: 3,
    params: {
      name: 'optimization_proposal',
      arguments: {
        workspace_root: '/tmp/test-workspace',
        package: 'Core',
        max_actions: 5,
        with_futures: true
      }
    }
  };
  
  server.stdin.write(JSON.stringify(optRequest) + '\n');
});

// Listen for the third response
server.stdout.once('data', (data) => {
  console.log('Optimization proposal response:', data.toString().trim());
  
  // Test 4: Call invalid tool to test error handling
  console.log('\nTest 4: Calling non-existent tool...');
  const invalidRequest = {
    jsonrpc: '2.0',
    method: 'mcp.tool.execute',
    id: 4,
    params: {
      name: 'nonexistent_tool',
      arguments: {
        workspace_root: '/tmp/test-workspace'
      }
    }
  };
  
  server.stdin.write(JSON.stringify(invalidRequest) + '\n');
});

// Listen for error response
server.stdout.once('data', (data) => {
  console.log('Non-existent tool response:', data.toString().trim());
  
  // Finish test
  console.log('\nComprehensive test completed.');
  server.kill(); // Terminate the server process
});

// Also handle server close event to ensure cleanup
server.on('close', (code) => {
  console.log(`Server exited with code ${code}`);
});