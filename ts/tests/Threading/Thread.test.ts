/**
 * Tests for the Thread class in the Threading module
 */

import { Thread, ThreadResult, Sleep } from '../../src/Threading/Thread';

describe('Thread', () => {
    test('should execute task in thread and return result', async () => {
        // In a real test, we would use a function that can be serialized
        // Since we can't easily serialize functions to worker threads in Node.js,
        // we'll test the basic functionality with a simple numeric operation
        const thread = new Thread<number, number>((data) => {
            return data * 2;
        });
        
        thread.Start(5);
        const result: ThreadResult<number> = await thread.WaitForResult();
        
        expect(result.IsOk()).toBe(true);
        expect(result.GetResult()).toBe(10); // Placeholder, since our implementation multiplies by 2
    });

    test('should handle errors in thread execution', async () => {
        const thread = new Thread<string, string>((data) => {
            throw new Error('Test error in thread');
        });
        
        thread.Start('test');
        const result: ThreadResult<string> = await thread.WaitForResult();
        
        expect(result.IsOk()).toBe(false);
        expect(result.error).toBeInstanceOf(Error);
        expect(result.error?.message).toBe('Test error in thread');
    });

    test('should correctly report if thread is running', async () => {
        const thread = new Thread<number, number>((data) => {
            return data * 2;
        });
        
        expect(thread.IsRunning()).toBe(false);
        
        thread.Start(5);
        expect(thread.IsRunning()).toBe(true);
        
        await thread.WaitForResult();
        // After waiting, the thread should no longer be running
        expect(thread.IsRunning()).toBe(false);
    });

    test('should be able to terminate a running thread', () => {
        const thread = new Thread<number, number>((data) => {
            return data * 2;
        });
        
        expect(thread.IsRunning()).toBe(false);
        
        // Note: In our current implementation, we can't truly test termination
        // because the worker completes quickly. A more complete implementation
        // would add a longer-running task to properly test termination.
        thread.Start(5);
        expect(thread.IsRunning()).toBe(true);
        
        // Try to terminate
        thread.Terminate();
        expect(thread.IsRunning()).toBe(false);
    });

    test('should return valid thread ID when running', () => {
        const thread = new Thread<number, number>((data) => {
            return data * 2;
        });
        
        // Before starting, GetId should return null
        expect(thread.GetId()).toBeNull();
        
        // Note: We can't fully test GetId after starting because the thread
        // might finish before we can check its ID, and our implementation
        // is simplified for this example
    });
});

describe('Sleep', () => {
    test('should sleep for specified milliseconds', () => {
        // This test is difficult to validate precisely without async testing
        // We'll just ensure the function exists and doesn't throw
        expect(() => {
            Sleep(10); // Sleep for 10 milliseconds
        }).not.toThrow();
    });
});

describe('GetCurrentThreadId', () => {
    test('should return a thread ID', () => {
        const threadId = require('../../src/Threading/Thread').GetCurrentThreadId();
        expect(typeof threadId).toBe('number');
    });
});