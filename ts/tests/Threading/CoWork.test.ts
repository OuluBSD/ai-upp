/**
 * Tests for the CoWork class in the Threading module
 */

import { CoWork, WorkItem, ParallelMap, ParallelReduce } from '../../src/Threading/CoWork';

describe('CoWork', () => {
    test('should execute work items in parallel', async () => {
        const coWork = new CoWork<number, number>(2); // Use 2 threads
        
        // Add some work items
        coWork.Add(1, 2, 3, 4, 5);
        
        // Execute a simple function on each work item
        const results = await coWork.Execute((n) => n * 2);
        
        // Check that all work items completed
        expect(results.length).toBe(5);
        
        // Check that all work items completed successfully
        for (let i = 0; i < results.length; i++) {
            expect(results[i].isCompleted()).toBe(true);
            expect(results[i].getResult()).toBe([2, 4, 6, 8, 10][i]);
        }
    });

    test('should be able to get results after execution', async () => {
        const coWork = new CoWork<string, string>(2);
        
        // Add work items
        coWork.Add('hello', 'world', 'uppts');
        
        // Execute a function
        await coWork.Execute((s) => s.toUpperCase());
        
        // Get results
        const results = coWork.GetResults();
        
        // Check results
        expect(results.length).toBe(3);
        expect(results[0].getResult()).toBe('HELLO');
        expect(results[1].getResult()).toBe('WORLD');
        expect(results[2].getResult()).toBe('UPPTS');
    });

    test('should handle errors in work items', async () => {
        const coWork = new CoWork<number, number>(2);
        
        // Add work items including one that will cause an error
        coWork.Add(1, 2, -1); // Use -1 to trigger an error condition in our function
        
        // Execute a function that throws an error for negative numbers
        const results = await coWork.Execute((n) => {
            if (n < 0) {
                throw new Error(`Negative number not allowed: ${n}`);
            }
            return n * 2;
        });
        
        // Check that the first two completed successfully
        expect(results[0].isCompleted()).toBe(true);
        expect(results[0].getResult()).toBe(2);
        
        expect(results[1].isCompleted()).toBe(true);
        expect(results[1].getResult()).toBe(4);
        
        // Check that the third had an error
        expect(results[2].isCompleted()).toBe(true);
        expect(results[2].error).toBeInstanceOf(Error);
        expect(results[2].error?.message).toBe('Negative number not allowed: -1');
    });

    test('should return correct counts', async () => {
        const coWork = new CoWork<number, number>(2);
        
        // Add work items
        coWork.Add(1, 2, 3);
        
        // Check initial count
        expect(coWork.GetCount()).toBe(3);
        expect(coWork.GetCompletedCount()).toBe(0);
        expect(coWork.IsCompleted()).toBe(false);
        
        // Execute
        await coWork.Execute((n) => n * 2);
        
        // Check final counts
        expect(coWork.GetCount()).toBe(3);
        expect(coWork.GetCompletedCount()).toBe(3);
        expect(coWork.IsCompleted()).toBe(true);
    });

    test('should clear all work items and threads', async () => {
        const coWork = new CoWork<number, number>(2);
        
        // Add work items
        coWork.Add(1, 2, 3);
        
        // Execute
        await coWork.Execute((n) => n * 2);
        
        // Check initial state
        expect(coWork.GetCount()).toBe(3);
        
        // Clear
        coWork.Clear();
        
        // Check cleared state
        expect(coWork.GetCount()).toBe(0);
        expect(coWork.GetCompletedCount()).toBe(0);
    });
});

describe('WorkItem', () => {
    test('should store data and result correctly', () => {
        const workItem = new WorkItem<number, number>(5);
        
        // Initially not completed
        expect(workItem.isCompleted()).toBe(false);
        expect(workItem.data).toBe(5);
        
        // Set result
        workItem.result = 10;
        workItem.completed = true;
        
        // Check result
        expect(workItem.isCompleted()).toBe(true);
        expect(workItem.getResult()).toBe(10);
    });

    test('should throw error when getting result of failed work item', () => {
        const workItem = new WorkItem<number, number>(5);
        
        // Set error state
        workItem.error = new Error('Test error');
        workItem.completed = true;
        
        // Getting result should throw
        expect(() => workItem.getResult()).toThrow('Test error');
    });
});

describe('ParallelMap', () => {
    test('should process array elements in parallel', async () => {
        const inputs = [1, 2, 3, 4, 5];
        const results = await ParallelMap(inputs, (n) => n * 2, 2);
        
        expect(results).toEqual([2, 4, 6, 8, 10]);
    });

    test('should handle errors in parallel processing', async () => {
        const inputs = [1, 2, -1, 4, 5];
        
        // This should throw an error when processing -1
        await expect(
            ParallelMap(inputs, (n) => {
                if (n < 0) {
                    throw new Error(`Negative number not allowed: ${n}`);
                }
                return n * 2;
            }, 2)
        ).rejects.toThrow('Negative number not allowed: -1');
    });
});

describe('ParallelReduce', () => {
    test('should process and reduce array elements in parallel', async () => {
        const inputs = [1, 2, 3, 4, 5];
        const result = await ParallelReduce(
            inputs,
            (n) => n * 2,  // Process: double each number
            (acc, value) => acc + value,  // Reduce: sum all values
            0,  // Initial value
            2   // Use 2 threads
        );
        
        // Expected: (1*2) + (2*2) + (3*2) + (4*2) + (5*2) = 2 + 4 + 6 + 8 + 10 = 30
        expect(result).toBe(30);
    });
});