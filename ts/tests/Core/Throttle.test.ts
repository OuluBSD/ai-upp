import { Throttle, QueuedThrottle } from '../../src/Core/Throttle';

describe('Throttle', () => {
    beforeEach(() => {
        jest.useFakeTimers();
    });

    afterEach(() => {
        jest.useRealTimers();
    });

    test('Execute allows function call if enough time has passed', () => {
        const throttle = new Throttle(1000); // 1 second throttle
        const mockFn = jest.fn();

        // First call should execute
        const result1 = throttle.Execute(mockFn, 'arg1');
        expect(result1).toBe(true);
        expect(mockFn).toHaveBeenCalledWith('arg1');

        // Reset mock to check next call
        mockFn.mockClear();

        // Second call immediately should be throttled
        const result2 = throttle.Execute(mockFn, 'arg2');
        expect(result2).toBe(false);
        expect(mockFn).not.toHaveBeenCalled();

        // Advance time by throttle delay
        jest.advanceTimersByTime(1000);

        // Now call again, should execute
        const result3 = throttle.Execute(mockFn, 'arg3');
        expect(result3).toBe(true);
        expect(mockFn).toHaveBeenCalledWith('arg3');
    });

    test('Reset resets the throttle', () => {
        const throttle = new Throttle(1000);
        const mockFn = jest.fn();

        // First call should execute
        throttle.Execute(mockFn, 'arg1');
        expect(mockFn).toHaveBeenCalledWith('arg1');
        mockFn.mockClear();

        // Second call immediately should be throttled
        const result = throttle.Execute(mockFn, 'arg2');
        expect(result).toBe(false);
        expect(mockFn).not.toHaveBeenCalled();

        // Reset the throttle
        throttle.Reset();

        // Now call again, should execute because of reset
        const resultAfterReset = throttle.Execute(mockFn, 'arg3');
        expect(resultAfterReset).toBe(true);
        expect(mockFn).toHaveBeenCalledWith('arg3');
    });

    test('GetRemainingTime returns correct remaining time', () => {
        const throttle = new Throttle(1000);
        const mockFn = jest.fn();

        // Execute once to start the throttle timer
        throttle.Execute(mockFn, 'arg1');
        
        // After 500ms, 500ms should remain
        jest.advanceTimersByTime(500);
        expect(throttle.GetRemainingTime()).toBe(500);

        // After another 300ms, 200ms should remain
        jest.advanceTimersByTime(300);
        expect(throttle.GetRemainingTime()).toBe(200);

        // After 200 more ms, 0 should remain
        jest.advanceTimersByTime(200);
        expect(throttle.GetRemainingTime()).toBe(0);
    });

    test('GetDelay returns correct delay value', () => {
        const throttle = new Throttle(2500);
        expect(throttle.GetDelay()).toBe(2500);

        const throttle2 = new Throttle(500);
        expect(throttle2.GetDelay()).toBe(500);
    });
});

describe('QueuedThrottle', () => {
    beforeEach(() => {
        jest.useFakeTimers();
    });

    afterEach(() => {
        jest.useRealTimers();
    });

    test('Execute queues function calls if throttled', () => {
        const throttle = new QueuedThrottle(1000); // 1 second throttle
        const mockFn = jest.fn();

        // First call should execute immediately
        throttle.Execute(mockFn, 'first');
        expect(mockFn).toHaveBeenCalledWith('first');
        expect(mockFn).toHaveBeenCalledTimes(1);

        // Queue a few more calls while throttled
        throttle.Execute(mockFn, 'second');
        throttle.Execute(mockFn, 'third');
        throttle.Execute(mockFn, 'fourth');

        // Mock should still have been called only once
        expect(mockFn).toHaveBeenCalledTimes(1);

        // Advance time to allow queued execution (the timeout is set for 1000ms from first execution)
        jest.advanceTimersByTime(1000);

        // The last queued call should now execute
        expect(mockFn).toHaveBeenCalledTimes(2);
        expect(mockFn).toHaveBeenLastCalledWith('fourth');
    });

    test('Reset clears queued calls', () => {
        const throttle = new QueuedThrottle(1000);
        const mockFn = jest.fn();

        // Execute once to start the throttle
        throttle.Execute(mockFn, 'first');
        expect(mockFn).toHaveBeenCalledTimes(1);

        // Queue a call
        throttle.Execute(mockFn, 'second');
        expect(mockFn).toHaveBeenCalledTimes(1); // Still only 1

        // Reset before the queued call executes
        throttle.Reset();

        // Advance time - no additional execution should happen because queue was cleared
        jest.advanceTimersByTime(1000);
        expect(mockFn).toHaveBeenCalledTimes(1); // Still only 1
    });

    test('GetRemainingTime returns correct remaining time', () => {
        const throttle = new QueuedThrottle(1000);
        const mockFn = jest.fn();

        // Execute once to start the throttle timer
        throttle.Execute(mockFn, 'first');
        
        // After 300ms, 700ms should remain
        jest.advanceTimersByTime(300);
        expect(throttle.GetRemainingTime()).toBe(700);
    });
});