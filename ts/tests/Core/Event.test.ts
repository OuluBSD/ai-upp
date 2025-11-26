import { Event } from '../../src/Core/Event';

describe('Event', () => {
    test('Add registers a callback', () => {
        const event = new Event<(message: string) => void>();
        const mockCallback = jest.fn();
        
        event.Add(mockCallback);
        event.Fire('Hello');
        
        expect(mockCallback).toHaveBeenCalledWith('Hello');
    });

    test('AddOnce registers a callback that executes only once', () => {
        const event = new Event<(value: number) => void>();
        const mockCallback = jest.fn();
        
        event.AddOnce(mockCallback);
        event.Fire(1);
        event.Fire(2);
        
        expect(mockCallback).toHaveBeenCalledTimes(1);
        expect(mockCallback).toHaveBeenCalledWith(1);
    });

    test('Remove unregisters a callback', () => {
        const event = new Event<() => void>();
        const mockCallback = jest.fn();
        
        event.Add(mockCallback);
        event.Fire();
        expect(mockCallback).toHaveBeenCalledTimes(1);
        
        event.Remove(mockCallback);
        event.Fire();
        expect(mockCallback).toHaveBeenCalledTimes(1); // Should still be 1
    });

    test('Fire executes all registered callbacks', () => {
        const event = new Event<(x: number, y: string) => void>();
        const mockCallback1 = jest.fn();
        const mockCallback2 = jest.fn();
        
        event.Add(mockCallback1);
        event.Add(mockCallback2);
        event.Fire(42, 'test');
        
        expect(mockCallback1).toHaveBeenCalledWith(42, 'test');
        expect(mockCallback2).toHaveBeenCalledWith(42, 'test');
    });

    test('IsEmpty returns correct value', () => {
        const event = new Event<() => void>();
        expect(event.IsEmpty()).toBe(true);

        event.Add(() => {});
        expect(event.IsEmpty()).toBe(false);

        event.Clear();
        expect(event.IsEmpty()).toBe(true);
    });

    test('GetCount returns correct count of registered handlers', () => {
        const event = new Event<() => void>();
        expect(event.GetCount()).toBe(0);

        const handler1 = () => {};
        const handler2 = () => {};
        const handler3 = () => {};
        
        event.Add(handler1);
        expect(event.GetCount()).toBe(1);

        event.Add(handler2);
        expect(event.GetCount()).toBe(2);

        event.AddOnce(handler3);
        expect(event.GetCount()).toBe(3);

        event.Fire(); // This should remove the once handler
        expect(event.GetCount()).toBe(2);
    });

    test('Clear removes all registered handlers', () => {
        const event = new Event<() => void>();
        event.Add(() => {});
        event.Add(() => {});
        event.AddOnce(() => {});
        
        expect(event.GetCount()).toBe(3);
        
        event.Clear();
        expect(event.GetCount()).toBe(0);
        expect(event.IsEmpty()).toBe(true);
    });

    test('Has checks if a handler is registered', () => {
        const event = new Event<() => void>();
        const handler1 = () => {};
        const handler2 = () => {};
        
        event.Add(handler1);
        expect(event.Has(handler1)).toBe(true);
        expect(event.Has(handler2)).toBe(false);

        event.AddOnce(handler2);
        expect(event.Has(handler2)).toBe(true);
    });

    test('Method chaining works correctly', () => {
        const event = new Event<() => void>();
        const handler = () => {};

        // All of these should return the event instance for chaining
        expect(event.Add(handler)).toBe(event);
        expect(event.Remove(handler)).toBe(event);
        expect(event.Fire()).toBe(event);
        expect(event.Clear()).toBe(event);
    });

    test('Multiple arguments work correctly', () => {
        const event = new Event<(a: number, b: string, c: boolean) => void>();
        const mockCallback = jest.fn();
        
        event.Add(mockCallback);
        event.Fire(42, 'hello', true);
        
        expect(mockCallback).toHaveBeenCalledWith(42, 'hello', true);
    });
});