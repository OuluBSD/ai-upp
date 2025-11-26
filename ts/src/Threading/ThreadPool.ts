/**
 * ThreadPool class for managing a pool of reusable threads
 * This provides an efficient way to execute multiple tasks without 
 * creating and destroying threads repeatedly
 * 
 * @example
 * ```typescript
 * const pool = new ThreadPool(4); // Create a pool with 4 threads
 * 
 * // Submit tasks to the pool
 * const result1 = pool.Submit(() => {
 *   // Some work here
 *   return 'result1';
 * });
 * 
 * const result2 = pool.Submit(() => {
 *   // Some other work
 *   return 'result2';
 * });
 * 
 * // Wait for results
 * console.log(await result1); // 'result1'
 * console.log(await result2); // 'result2'
 * 
 * // Shutdown the pool when done
 * pool.Shutdown();
 * ```
 */

import { Thread, ThreadResult } from './Thread';
import { Semaphore } from './Semaphore';

export interface Task<T = any, R = any> {
    execute(): R | Promise<R>;
}

class TaskWrapper<T = any, R = any> implements Task<T, R> {
    private func: (data: T) => R;
    private data: T;

    constructor(func: (data: T) => R, data: T) {
        this.func = func;
        this.data = data;
    }

    execute(): R {
        return this.func(this.data);
    }
}

export class ThreadPool {
    private threads: Thread<any, any>[] = [];
    private taskQueue: Array<{
        task: () => any,
        resolve: (value: any) => void,
        reject: (reason?: any) => void
    }> = [];
    private running: boolean = false;
    private maxThreads: number;
    private availableThreads: number;

    constructor(maxThreads: number = 4) {
        this.maxThreads = maxThreads;
        this.availableThreads = maxThreads;
    }

    /**
     * Starts the thread pool
     */
    Start(): void {
        if (this.running) {
            return;
        }

        this.running = true;

        // Initialize the worker threads
        for (let i = 0; i < this.maxThreads; i++) {
            this.createWorker();
        }
    }

    /**
     * Creates a new worker thread to process tasks
     */
    private createWorker(): void {
        // In a real implementation, we would create actual threads
        // For this U++-like implementation in JavaScript, we'll use async functions
        // to simulate the thread pool behavior
        
        // In JavaScript, we can't create true threads in the same way as U++
        // Instead, we'll process tasks asynchronously
        this.processTasks();
    }

    /**
     * Processes tasks from the queue
     */
    private async processTasks(): Promise<void> {
        while (this.running) {
            if (this.taskQueue.length > 0) {
                const { task, resolve, reject } = this.taskQueue.shift()!;
                
                try {
                    this.availableThreads--;
                    const result = await Promise.resolve(task());
                    resolve(result);
                } catch (error) {
                    reject(error);
                } finally {
                    this.availableThreads++;
                }
            } else {
                // If no tasks, wait a bit before checking again
                await new Promise(resolve => setTimeout(resolve, 1));
            }
        }
    }

    /**
     * Submits a task to the thread pool
     * @param task The task to execute
     * @returns A Promise that resolves with the task result
     */
    Submit<T, R>(task: () => R): Promise<R>;
    Submit<T, R>(task: (data: T) => R, data: T): Promise<R>;
    Submit<T, R>(task: ((data?: T) => R) | Task<T, R>, data?: T): Promise<R> {
        if (!this.running) {
            this.Start();
        }

        return new Promise<R>((resolve, reject) => {
            // Create a task to add to the queue
            let executableTask: () => R | Promise<R>;

            if (typeof task === 'function') {
                if (arguments.length === 1) {
                    // Task is a parameterless function
                    executableTask = task as () => R;
                } else {
                    // Task is a function with data parameter
                    executableTask = () => (task as (data: T) => R)(data!);
                }
            } else {
                // Task is a Task object with an execute method
                executableTask = () => task.execute();
            }
            
            // Add the task to the queue
            this.taskQueue.push({
                task: executableTask,
                resolve,
                reject
            });
        });
    }

    /**
     * Gets the number of active threads in the pool
     * @returns Number of active threads
     */
    GetActiveThreadCount(): number {
        return this.maxThreads - this.availableThreads;
    }

    /**
     * Gets the number of tasks waiting in the queue
     * @returns Number of queued tasks
     */
    GetQueuedTaskCount(): number {
        return this.taskQueue.length;
    }

    /**
     * Gets the maximum number of threads in the pool
     * @returns Maximum thread count
     */
    GetMaxThreadCount(): number {
        return this.maxThreads;
    }

    /**
     * Shuts down the thread pool, stopping all threads
     * @param waitForTasks If true, waits for all queued tasks to complete
     */
    Shutdown(waitForTasks: boolean = false): void {
        this.running = false;
        
        if (!waitForTasks) {
            // Clear the task queue
            this.taskQueue = [];
            
            // Reject all pending promises
            while (this.taskQueue.length > 0) {
                const { reject } = this.taskQueue.shift()!;
                reject(new Error('ThreadPool shutdown'));
            }
        }
        // If waitForTasks is true, the threads will finish their current tasks
        // and then terminate when the running flag is false
    }

    /**
     * Waits for all currently queued tasks to complete
     */
    async WaitForAllTasks(): Promise<void> {
        // Wait until the task queue is empty
        while (this.taskQueue.length > 0) {
            await new Promise(resolve => setTimeout(resolve, 10));
        }
        
        // Wait for all currently executing tasks to finish
        while (this.GetActiveThreadCount() > 0) {
            await new Promise(resolve => setTimeout(resolve, 10));
        }
    }

    /**
     * Submits multiple tasks and waits for all to complete
     * @param tasks The tasks to execute
     * @returns A Promise that resolves with an array of results
     */
    async SubmitAll<T, R>(...tasks: Array<() => R>): Promise<R[]> {
        const promises = tasks.map(task => this.Submit(task));
        return Promise.all(promises);
    }
}

/**
 * Static helper function to create and use a temporary thread pool
 * @param tasks The tasks to execute
 * @param maxThreads The maximum number of threads to use
 * @returns A Promise that resolves with an array of results
 */
export async function ParallelExecute<T, R>(
    tasks: Array<() => R>, 
    maxThreads: number = 4
): Promise<R[]> {
    const pool = new ThreadPool(maxThreads);
    const results = await pool.SubmitAll(...tasks);
    pool.Shutdown();
    return results;
}