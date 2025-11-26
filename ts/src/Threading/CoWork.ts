/**
 * CoWork class for parallel processing in a U++-like manner
 * This provides a way to execute tasks in parallel similar to U++'s CoWork functionality
 * 
 * @template T The type of the input to the processing function
 * @template R The type of the result from the processing function
 */

import { Thread, ThreadResult } from './Thread';

export class WorkItem<T, R> {
    data: T;
    result: R | null = null;
    error: Error | null = null;
    completed: boolean = false;

    constructor(data: T) {
        this.data = data;
    }

    isCompleted(): boolean {
        return this.completed;
    }

    getResult(): R {
        if (this.error) {
            throw this.error;
        }
        return this.result as R;
    }
}

export class CoWork<T, R> {
    private workItems: WorkItem<T, R>[] = [];
    private threads: Thread<T, R>[] = [];
    private maxThreads: number;

    /**
     * Creates a new CoWork instance
     * @param maxThreads Maximum number of threads to use for parallel processing
     */
    constructor(maxThreads: number = 4) { // Default to 4 threads
        this.maxThreads = maxThreads;
    }

    /**
     * Adds work items to be processed in parallel
     * @param data Array of input data to process
     * @returns This CoWork instance for chaining
     */
    Add(...data: T[]): CoWork<T, R> {
        for (const item of data) {
            this.workItems.push(new WorkItem(item));
        }
        return this;
    }

    /**
     * Executes the work items in parallel using the provided function
     * @param func The function to execute for each work item
     * @returns Promise that resolves when all work items are completed
     */
    async Execute(func: (data: T) => R): Promise<WorkItem<T, R>[]> {
        // Reset any previous results
        for (const workItem of this.workItems) {
            workItem.completed = false;
            workItem.result = null;
            workItem.error = null;
        }

        // Create threads pool
        const activeThreads: Thread<T, R>[] = [];
        const workQueue = [...this.workItems];
        const completedWorkItems: WorkItem<T, R>[] = [];

        // Process work items in parallel up to maxThreads
        while (workQueue.length > 0 || activeThreads.length > 0) {
            // Start new threads up to maxThreads limit
            while (workQueue.length > 0 && activeThreads.length < this.maxThreads) {
                const workItem = workQueue.shift()!;
                
                // Create a new thread to process this work item
                const thread = new Thread<T, R>(func);
                
                // Start the thread
                thread.Start(workItem.data);
                
                // Store thread and associated work item
                this.threads.push(thread);
                activeThreads.push(thread);
                
                // Handle completion of this thread
                thread.WaitForResult().then((result: ThreadResult<R>) => {
                    // Find the corresponding work item
                    const index = this.workItems.indexOf(workItem);
                    if (index !== -1) {
                        if (result.IsOk()) {
                            this.workItems[index].result = result.GetResult();
                            this.workItems[index].completed = true;
                        } else {
                            this.workItems[index].error = result.error;
                            this.workItems[index].completed = true;
                        }
                        
                        // Remove the completed thread from active threads
                        const threadIndex = activeThreads.indexOf(thread);
                        if (threadIndex !== -1) {
                            activeThreads.splice(threadIndex, 1);
                        }
                    }
                });
            }

            // Wait a bit to avoid busy waiting
            await new Promise(resolve => setTimeout(resolve, 1));
        }

        return this.workItems;
    }

    /**
     * Gets the results of all work items
     * @returns Array of WorkItem objects containing results
     */
    GetResults(): WorkItem<T, R>[] {
        return this.workItems;
    }

    /**
     * Clears all work items and threads
     */
    Clear(): void {
        this.workItems = [];
        this.threads = [];
    }

    /**
     * Gets the number of work items
     */
    GetCount(): number {
        return this.workItems.length;
    }

    /**
     * Gets the number of completed work items
     */
    GetCompletedCount(): number {
        return this.workItems.filter(item => item.completed).length;
    }

    /**
     * Checks if all work items have been completed
     */
    IsCompleted(): boolean {
        return this.workItems.every(item => item.completed);
    }
}

/**
 * Static utility function to run a function in parallel on an array of inputs
 * @param inputs Array of inputs to process
 * @param func Function to apply to each input element
 * @param maxThreads Maximum number of threads to use
 * @returns Promise that resolves to an array of results
 */
export async function ParallelMap<T, R>(
    inputs: T[],
    func: (input: T) => R,
    maxThreads: number = 4
): Promise<R[]> {
    const coWork = new CoWork<T, R>(maxThreads);
    coWork.Add(...inputs);
    
    const results = await coWork.Execute(func);
    
    // Extract just the results in the same order as the inputs
    return results.map(item => {
        if (item.error) {
            throw item.error;
        }
        return item.result as R;
    });
}

/**
 * Static utility function to run a function in parallel on an array of inputs
 * and reduce the results with a reducer function
 * @param inputs Array of inputs to process
 * @param processFunc Function to apply to each input element
 * @param reduceFunc Function to reduce the results
 * @param maxThreads Maximum number of threads to use
 * @returns Promise that resolves to the reduced result
 */
export async function ParallelReduce<T, P, R>(
    inputs: T[],
    processFunc: (input: T) => P,
    reduceFunc: (acc: R, value: P, index: number, array: P[]) => R,
    initialValue: R,
    maxThreads: number = 4
): Promise<R> {
    const processedResults = await ParallelMap(inputs, processFunc, maxThreads);
    return processedResults.reduce(reduceFunc, initialValue);
}