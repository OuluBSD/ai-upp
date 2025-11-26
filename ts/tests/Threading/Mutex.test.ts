/**
 * Tests for the Mutex class in the Threading module
 */

import { Mutex, RecursiveMutex } from '../../src/Threading/Mutex';

describe('Mutex', () => {
    test('should acquire and release lock', async () => {
        const mutex = new Mutex();
        
        // Initially, the mutex should not be locked
        expect(mutex.IsLocked()).toBe(false);
        
        // Acquire the lock
        const release = await mutex.Lock();
        
        // Now the mutex should be locked
        expect(mutex.IsLocked()).toBe(true);
        
        // Release the lock
        release();
        
        // Now the mutex should not be locked
        expect(mutex.IsLocked()).toBe(false);
    });

    test('should block when trying to acquire a locked mutex', async () => {
        const mutex = new Mutex();
        
        // Acquire the lock first
        const firstRelease = await mutex.Lock();
        expect(mutex.IsLocked()).toBe(true);
        
        // Try to acquire the lock again - this should wait
        let secondAcquired = false;
        let secondRelease: (() => void) | undefined;
        const lockPromise = mutex.Lock().then((releaseFn) => {
            secondRelease = releaseFn;
            secondAcquired = true;
        });

        // Give a small timeout to ensure the second lock attempt is waiting
        await new Promise(resolve => setTimeout(resolve, 10));

        // The second lock should not have been acquired yet
        expect(secondAcquired).toBe(false);

        // Release the first lock
        firstRelease();

        // Wait for the second lock to be acquired
        await lockPromise;

        // Now the second lock should be acquired
        expect(secondAcquired).toBe(true);
        expect(mutex.IsLocked()).toBe(true);

        // Release the second lock
        if (secondRelease) {
            secondRelease();
        }
    });

    test('should successfully use TryLock when mutex is not locked', () => {
        const mutex = new Mutex();
        
        // Try to lock when unlocked should succeed
        const release = mutex.TryLock();
        expect(release).not.toBeNull();
        expect(mutex.IsLocked()).toBe(true);
        
        // Release the lock
        if (release) {
            release();
        }
        
        expect(mutex.IsLocked()).toBe(false);
    });

    test('should return null from TryLock when mutex is locked', async () => {
        const mutex = new Mutex();
        
        // Lock the mutex
        const firstRelease = await mutex.Lock();
        expect(mutex.IsLocked()).toBe(true);
        
        // TryLock should return null when mutex is locked
        const secondTry = mutex.TryLock();
        expect(secondTry).toBeNull();
        
        // Release the first lock
        firstRelease();
    });
});

describe('RecursiveMutex', () => {
    test('should allow same thread to lock multiple times', async () => {
        const mutex = new RecursiveMutex();
        
        // Initially, the mutex should not be locked
        expect(mutex.IsLocked()).toBe(false);
        
        // Acquire the lock first time
        const release1 = await mutex.Lock();
        expect(mutex.IsLocked()).toBe(true);
        
        // Acquire the lock second time (should succeed for recursive mutex)
        const release2 = await mutex.Lock();
        expect(mutex.IsLocked()).toBe(true);
        
        // Release once - should still be locked
        release1();
        expect(mutex.IsLocked()).toBe(true);
        
        // Release again - should now be unlocked
        release2();
        expect(mutex.IsLocked()).toBe(false);
    });

    test('should successfully use TryLock when recursive mutex is not locked', async () => {
        const mutex = new RecursiveMutex();
        
        // Try to lock when unlocked should succeed
        const release = mutex.TryLock();
        expect(release).not.toBeNull();
        expect(mutex.IsLocked()).toBe(true);
        
        // Release the lock
        if (release) {
            release();
        }
        
        expect(mutex.IsLocked()).toBe(false);
    });
});