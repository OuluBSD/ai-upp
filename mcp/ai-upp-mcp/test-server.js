#!/usr/bin/env node

import { spawn } from 'child_process';

// Test function to send requests to the server
async function sendRequest(server, request) {
  return new Promise((resolve, reject) => {
    const responsePromise = new Promise((res, rej) => {
      const timer = setTimeout(() => {
        rej(new Error('Timeout waiting for response'));
      }, 5000);

      server.stdout.once('data', (data) => {
        clearTimeout(timer);
        try {
          const response = JSON.parse(data.toString().trim());
          res(response);
        } catch (e) {
          rej(e);
        }
      });
    });

    server.stdin.write(JSON.stringify(request) + '\n');
    resolve(responsePromise);
  });
}

console.log('Starting MCP Server test...\n');

// Spawn the server process
const server = spawn('node', ['server.js'], {
  cwd: './'
});

server.on('error', (err) => {
  console.error('Server error:', err);
});

// Wait a moment for server to start
await new Promise(resolve => setTimeout(resolve, 500));

try {
  // Test 1: List tools
  console.log('Test 1: Listing tools...');
  const listRequest = {
    jsonrpc: '2.0',
    method: 'mcp.tools.list',
    id: 1
  };

  const listResponse = await sendRequest(server, listRequest);
  console.log('Response:', JSON.stringify(listResponse, null, 2));

  // Test 2: Try calling a tool (this will likely fail due to missing theide-cli, but we can see how it responds)
  console.log('\nTest 2: Calling workspace_overview...');
  const toolRequest = {
    jsonrpc: '2.0',
    method: 'mcp.tool.execute',
    id: 2,
    params: {
      name: 'workspace_overview',
      arguments: {
        workspace_root: '/tmp/fake-path'
      }
    }
  };

  server.stdin.write(JSON.stringify(toolRequest) + '\n');

  // Wait for response
  server.stdout.once('data', (data) => {
    console.log('Tool response:', data.toString().trim());
  });

  // Give some time to receive response before closing
  await new Promise(resolve => setTimeout(resolve, 2000));
  
} catch (error) {
  console.error('Test error:', error);
} finally {
  server.kill(); // Terminate the server process
  console.log('\nTest completed.');
}