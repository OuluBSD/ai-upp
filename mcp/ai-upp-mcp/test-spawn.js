#!/usr/bin/env node

import { spawn } from 'child_process';

// Test the child process functionality directly
async function testSpawn() {
    return new Promise((resolve, reject) => {
        const child = spawn('./theide-cli', ['--workspace-root', '/tmp/test', '--json', 'workspace_stats'], { cwd: process.cwd() });
        
        let stdout = '';
        let stderr = '';
        
        child.stdout.on('data', (data) => {
            console.log('STDOUT:', data.toString());
            stdout += data.toString();
        });
        
        child.stderr.on('data', (data) => {
            console.log('STDERR:', data.toString());
            stderr += data.toString();
        });
        
        child.on('error', (err) => {
            console.log('SPAWN ERROR:', err.message);
            reject(new Error(`Failed to start theide-cli: ${err.message}`));
        });
        
        child.on('close', (code) => {
            console.log('CHILD PROCESS CLOSED WITH CODE:', code);
            if (code !== 0) {
                reject(new Error(`Command failed with code ${code}: ${stderr}`));
            } else {
                try {
                    const result = JSON.parse(stdout);
                    console.log('PARSED RESULT:', result);
                    resolve(result);
                } catch (parseErr) {
                    reject(new Error(`Failed to parse JSON response: ${parseErr.message}\n${stdout}`));
                }
            }
        });
    });
}

console.log('Testing child process execution...');
testSpawn().then(result => {
    console.log('SUCCESS:', result);
}).catch(err => {
    console.log('ERROR:', err.message);
});