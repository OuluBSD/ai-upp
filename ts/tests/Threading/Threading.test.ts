/**
 * Comprehensive tests for the Threading module
 * Tests all aspects of the threading functionality
 */

import { Semaphore, SyncEvent } from '../../src/Threading/Semaphore';
import { Async, Delay, WaitForAll, WaitForAny } from '../../src/Threading/Async';
import { AsyncCoWork, AsyncParallel, AsyncFirst, AsyncFor, AsyncForEach, AsyncMap, AsyncFilter } from '../../src/Threading/AsyncHelpers';
import { ThreadPool, ParallelExecute } from '../../src/Threading/ThreadPool';
import { SynchronizedVector, SynchronizedArray, SynchronizedIndex, SynchronizedMap, Synchronized } from '../../src/Threading/SynchronizedContainers';

describe('Semaphore', () => {
    test('should allow limited concurrent access', async () => {
        const semaphore = new Semaphore(2); // Allow 2 threads at a time
        const accessOrder: number[] = [];

        // Create 5 tasks that require the semaphore
        const tasks = [];
        for (let i = 0; i < 5; i++) {
            tasks.push((async (id: number) => {
                await semaphore.Wait();
                try {
                    // Record when this task got access
                    accessOrder.push(id);
                    // Hold the semaphore for a bit
                    await Delay(10);
                } finally {
                    semaphore.Signal();
                }
            })(i));
        }

        // Execute all tasks in parallel
        await Promise.all(tasks);

        // All tasks should have completed
        expect(accessOrder.length).toBe(5);
        // The order might vary but all IDs should be there
        expect(accessOrder.sort()).toEqual([0, 1, 2, 3, 4]);
    });

    test('should correctly report available permits', async () => {
        const semaphore = new Semaphore(3);
        
        expect(semaphore.GetAvailablePermits()).toBe(3);
        expect(semaphore.GetTotalPermits()).toBe(3);
        
        await semaphore.Wait();
        expect(semaphore.GetAvailablePermits()).toBe(2);
        
        semaphore.Signal();
        expect(semaphore.GetAvailablePermits()).toBe(3);
    });

    test('should support TryWait', () => {
        const semaphore = new Semaphore(1);
        
        // Should succeed when permits are available
        expect(semaphore.TryWait()).toBe(true);
        expect(semaphore.GetAvailablePermits()).toBe(0);
        
        // Should fail when no permits available
        expect(semaphore.TryWait()).toBe(false);
        expect(semaphore.GetAvailablePermits()).toBe(0);
    });
});

describe('SyncEvent', () => {
    test('should block until signaled', async () => {
        const event = new SyncEvent(false);

        // Track the order of execution
        const order: string[] = [];

        // Start waiting for the event
        const waitPromise = (async () => {
            await event.WaitFor();
            order.push('waited');
        })();

        // Wait a bit
        await Delay(10);
        order.push('before_set');

        // Signal the event
        event.Set();
        order.push('after_set');

        // Wait for the waiting task to complete
        await waitPromise;

        // Verify the order
        expect(order).toEqual(['before_set', 'after_set', 'waited']);
    });

    test('should start in correct initial state', () => {
        const event1 = new SyncEvent(true);
        const event2 = new SyncEvent(false);

        expect(event1.IsSignaled()).toBe(true);
        expect(event2.IsSignaled()).toBe(false);
    });
});

describe('Async utilities', () => {
    test('Delay should delay execution', async () => {
        const start = Date.now();
        await Delay(10);
        const end = Date.now();

        // Should have waited at least 5ms (allowing for timing precision)
        expect(end - start).toBeGreaterThanOrEqual(5);
    });

    test('WaitForAll should wait for all promises', async () => {
        const results = await WaitForAll([
            Promise.resolve(1),
            Promise.resolve(2),
            Promise.resolve(3)
        ]);
        
        expect(results).toEqual([1, 2, 3]);
    });

    test('WaitForAny should wait for first promise', async () => {
        const result = await WaitForAny([
            Delay(50).then(() => 'slow'),
            Delay(10).then(() => 'fast'),  // This should resolve first
            Delay(100).then(() => 'slower')
        ]);
        
        expect(result).toBe('fast');
    });

    test('Async.Delay should delay execution', async () => {
        const start = Date.now();
        await Async.Delay(10);
        const end = Date.now();

        // Should have waited at least 5ms (allowing for timing precision)
        expect(end - start).toBeGreaterThanOrEqual(5);
    });

    test('Async.Retry should retry failed operations', async () => {
        let attempts = 0;
        const result = await Async.Retry(() => {
            attempts++;
            if (attempts < 3) {
                throw new Error('Attempt failed');
            }
            return Promise.resolve('success');
        }, 3, 10);
        
        expect(result).toBe('success');
        expect(attempts).toBe(3);
    });

    test('Async.Timeout should time out operations', async () => {
        await expect(Async.Timeout(Delay(100), 10)).rejects.toThrow('timed out');
    });
});

describe('AsyncCoWork', () => {
    test('should execute async work items in parallel', async () => {
        const coWork = new AsyncCoWork<number, number>(2);
        coWork.Add(1, 2, 3, 4, 5);
        
        const results = await coWork.Execute(async (n) => {
            // Simulate async work
            await Delay(10);
            return n * 2;
        });
        
        expect(results).toEqual([2, 4, 6, 8, 10]);
    });
});

describe('Async helpers', () => {
    test('AsyncParallel should run functions in parallel', async () => {
        const results = await AsyncParallel(
            () => Delay(10).then(() => 1),
            () => Delay(5).then(() => 2),
            () => Delay(15).then(() => 3)
        );
        
        expect(results).toEqual([1, 2, 3]);
    });

    test('AsyncFirst should return first completed function', async () => {
        const result = await AsyncFirst(
            () => Delay(50).then(() => 'slow'),
            () => Delay(5).then(() => 'fast'),  // This should complete first
            () => Delay(100).then(() => 'slower')
        );
        
        expect(result).toBe('fast');
    });

    test('AsyncFor should iterate with periodic yields', async () => {
        const results: number[] = [];
        
        await AsyncFor(5, async (i) => {
            results.push(i);
            // Add a small delay to make the async nature more apparent
            await Delay(1);
        });
        
        expect(results).toEqual([0, 1, 2, 3, 4]);
    });

    test('AsyncMap should transform array elements asynchronously', async () => {
        const input = [1, 2, 3, 4, 5];
        const result = await AsyncMap(input, async (x) => {
            await Delay(1);  // Simulate async work
            return x * 2;
        });
        
        expect(result).toEqual([2, 4, 6, 8, 10]);
    });

    test('AsyncFilter should filter array elements asynchronously', async () => {
        const input = [1, 2, 3, 4, 5, 6];
        const result = await AsyncFilter(input, async (x) => {
            await Delay(1);  // Simulate async work
            return x % 2 === 0;
        });
        
        expect(result).toEqual([2, 4, 6]);
    });
});

describe('ThreadPool', () => {
    test('should execute tasks in parallel', async () => {
        const pool = new ThreadPool(3); // 3 threads
        
        // Submit tasks that return their input doubled
        const promises = [];
        for (let i = 1; i <= 5; i++) {
            // Pass the value as data to avoid closure issues
            promises.push(pool.Submit((value) => {
                // Simulate some work
                return value * 2;
            }, i));
        }
        
        const results = await Promise.all(promises);
        expect(results.sort((a, b) => a - b)).toEqual([2, 4, 6, 8, 10]);
        
        expect(pool.GetMaxThreadCount()).toBe(3);
        expect(pool.GetQueuedTaskCount()).toBe(0);
        
        pool.Shutdown();
    });

    test('should handle tasks with data', async () => {
        const pool = new ThreadPool(2);
        
        const promises = [];
        for (let i = 1; i <= 3; i++) {
            promises.push(pool.Submit((x) => x * 3, i));
        }
        
        const results = await Promise.all(promises);
        expect(results.sort()).toEqual([3, 6, 9]);
        
        pool.Shutdown();
    });

    test('should wait for all tasks to complete', async () => {
        const pool = new ThreadPool(2);
        
        // Add some tasks
        for (let i = 1; i <= 10; i++) {
            pool.Submit(async () => {
                await Delay(5);
                return i * 2;
            });
        }
        
        await pool.WaitForAllTasks();
        expect(pool.GetQueuedTaskCount()).toBe(0);
        
        pool.Shutdown();
    });
});

describe('ParallelExecute', () => {
    test('should execute multiple tasks in parallel', async () => {
        const tasks = [
            () => Promise.resolve(1),
            () => Promise.resolve(2),
            () => Promise.resolve(3),
            () => Promise.resolve(4),
            () => Promise.resolve(5)
        ];
        
        const results = await ParallelExecute(tasks, 2); // Use 2 threads
        expect(results).toEqual([1, 2, 3, 4, 5]);
    });
});

describe('Synchronized containers', () => {
    test('SynchronizedVector should provide thread-safe access', async () => {
        const vector = new SynchronizedVector<number>();
        
        // Add some elements
        await vector.Add(1);
        await vector.Add(2);
        await vector.Add(3);
        
        expect(await vector.GetCount()).toBe(3);
        expect(await vector.At(0)).toBe(1);
        expect(await vector.At(1)).toBe(2);
        expect(await vector.At(2)).toBe(3);
        
        // Remove an element
        await vector.Remove(0);
        expect(await vector.GetCount()).toBe(2);
        expect(await vector.At(0)).toBe(2);
    });

    test('SynchronizedArray should provide thread-safe access', async () => {
        const array = new SynchronizedArray<string>();
        
        await array.Add('hello');
        await array.Add('world');
        await array.Add('uppts');
        
        expect(await array.GetCount()).toBe(3);
        expect(await array.At(0)).toBe('hello');
        expect(await array.At(1)).toBe('world');
        expect(await array.At(2)).toBe('uppts');
    });

    test('SynchronizedIndex should provide thread-safe access', async () => {
        const index = new SynchronizedIndex<number>();
        
        await index.Add(10);
        await index.Add(20);
        await index.Add(30);
        
        expect(await index.GetCount()).toBe(3);
        expect(await index.Contains(20)).toBe(true);
        expect(await index.Contains(40)).toBe(false);
        
        await index.Remove(20);
        expect(await index.GetCount()).toBe(2);
        expect(await index.Contains(20)).toBe(false);
    });

    test('SynchronizedMap should provide thread-safe access', async () => {
        const map = new SynchronizedMap<string, number>();
        
        await map.Set('a', 1);
        await map.Set('b', 2);
        await map.Set('c', 3);
        
        expect(await map.GetCount()).toBe(3);
        expect(await map.Get('a')).toBe(1);
        expect(await map.Get('b')).toBe(2);
        expect(await map.Get('c')).toBe(3);
        expect(await map.Has('a')).toBe(true);
        expect(await map.Has('d')).toBe(false);
        
        await map.Remove('b');
        expect(await map.GetCount()).toBe(2);
        expect(await map.Has('b')).toBe(false);
    });

    test('Synchronized wrapper should provide generic thread-safe access', async () => {
        const data = { value: 0 };
        const sync = new Synchronized(data);
        
        const results = await sync.WithSync(obj => {
            obj.value++;
            return obj.value;
        });
        
        expect(results).toBe(1);
        expect(sync.Get().value).toBe(1);
    });
});