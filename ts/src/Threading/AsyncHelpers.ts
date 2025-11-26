/**
 * Async/await helpers that match U++ patterns
 * These utilities provide familiar U++-like async functionality
 */

import { Thread } from './Thread';
import { CoWork } from './CoWork';

/**
 * Async version of the CoWork functionality
 * Allows for async processing of work items with parallel execution
 */
export class AsyncCoWork<T, R> {
    private workItems: Array<{ data: T, result?: R, error?: Error }> = [];
    private maxConcurrency: number;

    constructor(maxConcurrency: number = 4) {
        this.maxConcurrency = maxConcurrency;
    }

    /**
     * Adds work items to be processed
     * @param data The data to process
     * @returns This AsyncCoWork instance for chaining
     */
    Add(...data: T[]): AsyncCoWork<T, R> {
        for (const item of data) {
            this.workItems.push({ data: item });
        }
        return this;
    }

    /**
     * Executes the work items asynchronously with the provided async function
     * @param func The async function to execute for each work item
     * @returns Promise that resolves when all work items are completed
     */
    async Execute(func: (data: T) => Promise<R>): Promise<R[]> {
        // Process items with limited concurrency
        const results: R[] = [];
        
        // Process in batches based on maxConcurrency
        for (let i = 0; i < this.workItems.length; i += this.maxConcurrency) {
            const batch = this.workItems.slice(i, i + this.maxConcurrency);
            
            // Process batch in parallel
            const batchPromises = batch.map(async (workItem) => {
                try {
                    const result = await func(workItem.data);
                    workItem.result = result;
                    return result;
                } catch (error) {
                    workItem.error = error as Error;
                    throw error;
                }
            });
            
            const batchResults = await Promise.all(batchPromises);
            results.push(...batchResults);
        }
        
        return results;
    }

    /**
     * Gets the results of all work items
     * @returns Array of results
     */
    GetResults(): R[] {
        return this.workItems
            .filter(item => item.result !== undefined)
            .map(item => item.result as R);
    }

    /**
     * Gets any errors that occurred during processing
     * @returns Array of errors
     */
    GetErrors(): Error[] {
        return this.workItems
            .filter(item => item.error !== undefined)
            .map(item => item.error as Error);
    }

    /**
     * Clears all work items
     */
    Clear(): void {
        this.workItems = [];
    }
}

/**
 * Executes an async function in a new thread
 * @param func The async function to execute in a separate thread
 * @param data The data to pass to the function
 * @returns A Thread instance that can be awaited for results
 */
export function AsyncExecute<T, R>(func: (data: T) => Promise<R>, data: T): Thread<T, Promise<R>> {
    // Note: In JavaScript/Node.js, true parallel execution of async functions 
    // across different threads is complex. This is a placeholder implementation.
    return new Thread<T, Promise<R>>(async (input: T) => {
        return func(input);
    });
}

/**
 * Runs multiple async functions in parallel and waits for all to complete
 * @param funcs Array of async functions to run
 * @returns Promise that resolves to an array of results
 */
export async function AsyncParallel<T>(...funcs: Array<() => Promise<T>>): Promise<T[]> {
    const promises = funcs.map(func => func());
    return Promise.all(promises);
}

/**
 * Runs multiple async functions in parallel but waits for any to complete
 * @param funcs Array of async functions to run
 * @returns Promise that resolves to the result of the first to complete
 */
export async function AsyncFirst<T>(...funcs: Array<() => Promise<T>>): Promise<T> {
    const promises = funcs.map(func => func());
    return Promise.race(promises);
}

/**
 * Helper function to execute a function with a timeout
 * @param func The async function to execute
 * @param timeoutMs The timeout in milliseconds
 * @returns Promise that resolves with the function result or rejects with a timeout error
 */
export async function WithTimeout<T>(func: () => Promise<T>, timeoutMs: number): Promise<T> {
    // Create a timeout promise
    const timeoutPromise = new Promise<never>((_, reject) => {
        setTimeout(() => reject(new Error(`Operation timed out after ${timeoutMs}ms`)), timeoutMs);
    });

    // Race the original function against the timeout
    return Promise.race([func(), timeoutPromise]);
}

/**
 * Async loop helper that yields control periodically to avoid blocking the event loop
 * @param count Number of iterations
 * @param func Function to execute for each iteration
 */
export async function AsyncFor(
    count: number,
    func: (index: number) => Promise<void> | void
): Promise<void> {
    for (let i = 0; i < count; i++) {
        func(i);
        
        // Yield control to the event loop periodically to avoid blocking
        if (i % 1000 === 0) {
            await new Promise(resolve => setImmediate(resolve));
        }
    }
}

/**
 * Async version of ForEach for arrays
 * @param array The array to iterate over
 * @param callbackFn Function to execute for each element
 */
export async function AsyncForEach<T>(
    array: T[],
    callbackFn: (value: T, index: number, array: T[]) => Promise<void> | void
): Promise<void> {
    for (let i = 0; i < array.length; i++) {
        await callbackFn(array[i], i, array);
        
        // Yield control to the event loop periodically
        if (i % 1000 === 0) {
            await new Promise(resolve => setImmediate(resolve));
        }
    }
}

/**
 * Async version of Map for arrays
 * @param array The array to transform
 * @param callbackFn Function to transform each element
 */
export async function AsyncMap<T, U>(
    array: T[],
    callbackFn: (value: T, index: number, array: T[]) => Promise<U> | U
): Promise<U[]> {
    const results: U[] = [];
    
    for (let i = 0; i < array.length; i++) {
        const result = await callbackFn(array[i], i, array);
        results.push(result);
        
        // Yield control to the event loop periodically
        if (i % 1000 === 0) {
            await new Promise(resolve => setImmediate(resolve));
        }
    }
    
    return results;
}

/**
 * Async version of Filter for arrays
 * @param array The array to filter
 * @param predicate Function to test each element
 */
export async function AsyncFilter<T>(
    array: T[],
    predicate: (value: T, index: number, array: T[]) => Promise<boolean> | boolean
): Promise<T[]> {
    const results: T[] = [];
    
    for (let i = 0; i < array.length; i++) {
        const shouldInclude = await predicate(array[i], i, array);
        if (shouldInclude) {
            results.push(array[i]);
        }
        
        // Yield control to the event loop periodically
        if (i % 1000 === 0) {
            await new Promise(resolve => setImmediate(resolve));
        }
    }
    
    return results;
}