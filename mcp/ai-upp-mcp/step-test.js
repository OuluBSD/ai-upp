#!/usr/bin/env node

import { spawn } from 'child_process';

function wait(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

async function testServer() {
    console.log('Starting step-by-step MCP Server test...\n');

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
        console.log('List tools response:', data.toString().trim(), '\n');
    });

    // Wait before next test
    await wait(1000);

    // Test 2: Call workspace_overview
    console.log('Test 2: Calling workspace_overview...');
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
    
    // Wait for response
    server.stdout.once('data', (data) => {
        console.log('Workspace overview response:', data.toString().trim(), '\n');
    });

    // Wait before next test
    await wait(1000);

    // Test 3: Call optimization_proposal
    console.log('Test 3: Calling optimization_proposal...');
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
    
    // Wait for response
    server.stdout.once('data', (data) => {
        console.log('Optimization proposal response:', data.toString().trim(), '\n');
        
        // Close the server
        server.kill();
        console.log('Step-by-step test completed.');
    });
}

testServer();