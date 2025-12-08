/**
 * Example 3: Threading and Async operations
 * Demonstrates threading functionality: Thread, Mutex, Async operations
 */

import { Thread, Sleep } from '../src/Threading/Thread';
import { Mutex } from '../src/Threading/Mutex';
import { Async, Delay } from '../src/Threading/Async';
import { CoWork } from '../src/Threading/CoWork';
import { Vector } from '../src/Core/Vector';

async function threadingExample() {
    console.log('=== Threading Example ===');
    
    // Shared data that multiple threads will access
    let sharedCounter = 0;
    const mutex = new Mutex();
    
    // Create multiple threads that increment the shared counter
    const threads = new Vector<Thread>();
    
    for (let i = 0; i < 3; i++) {
        const thread = new Thread(() => {
            for (let j = 0; j < 5; j++) {
                // Lock the mutex before accessing shared data
                mutex.Enter();
                try {
                    const temp = sharedCounter;
                    // Simulate some work
                    Sleep(10); // Sleep for 10 milliseconds
                    sharedCounter = temp + 1;
                    console.log(`Thread ${Thread.GetCurrentThreadId()} incremented counter to: ${sharedCounter}`);
                } finally {
                    // Always release the mutex
                    mutex.Leave();
                }
            }
        });
        
        threads.Add(thread);
    }
    
    // Start all threads
    for (let i = 0; i < threads.GetCount(); i++) {
        threads.At(i).Start();
    }
    
    // Wait for all threads to complete
    for (let i = 0; i < threads.GetCount(); i++) {
        threads.At(i).Join();
    }
    
    console.log(`Final counter value: ${sharedCounter} (expected: 15)`);
    console.log('Threading example completed.\n');
}

async function asyncExample() {
    console.log('=== Async Operations Example ===');
    
    // Example 1: Parallel processing with CoWork
    console.log('Starting parallel computation example...');
    
    const workItems = new Vector<() => number>();
    
    // Add several work items that simulate computation
    for (let i = 1; i <= 5; i++) {
        const workFn = () => {
            // Simulate CPU-intensive work
            let result = 0;
            for (let j = 0; j < 1000000 * i; j++) {
                result += j % (i + 1);
            }
            console.log(`Work item ${i} completed with result: ${result % 1000}`);
            return result % 1000;
        };
        workItems.Add(workFn);
    }
    
    // Execute work items in parallel
    const results = await CoWork.ParallelMap(workItems);
    console.log(`Parallel computation results: [${results.join(', ')}]`);
    
    // Example 2: Async delays
    console.log('\nStarting async delay example...');
    console.log('Starting delay operations...');
    
    const delayPromise1 = Delay(1000); // 1 second delay
    const delayPromise2 = Delay(500);  // 0.5 second delay
    const delayPromise3 = Delay(1500); // 1.5 second delay
    
    await Promise.all([delayPromise1, delayPromise2, delayPromise3]);
    console.log('All delay operations completed.');
    
    // Example 3: Async with Future pattern
    console.log('\nStarting Future pattern example...');
    const future = Async.Execute(() => {
        // Simulate async work
        Sleep(500);
        return 'Future result';
    });
    
    const futureResult = await future;
    console.log(`Future completed with result: ${futureResult}`);
    
    console.log('Async operations example completed.\n');
}

async function runThreadingExamples() {
    console.log('Starting threading and async examples...\n');
    
    await threadingExample();
    await asyncExample();
    
    console.log('All threading and async examples completed!');
}

runThreadingExamples().catch(console.error);