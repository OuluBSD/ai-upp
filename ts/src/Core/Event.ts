/**
 * An event class similar to U++ Event, providing an event system for callbacks.
 * Allows registering and unregistering callbacks, and triggering events with arguments.
 * 
 * @template TArgs The types of arguments the event handlers accept
 * 
 * @example
 * ```typescript
 * const event = new Event<(message: string) => void>();
 * 
 * const handler = (msg: string) => console.log(`Received: ${msg}`);
 * event.Add(handler);
 * 
 * event.Fire("Hello, World!"); // "Received: Hello, World!"
 * 
 * event.Remove(handler);
 * event.Fire("This won't be printed");
 * ```
 */
export class Event<T extends (...args: any[]) => any> {
    private handlers: T[] = [];
    private onceHandlers: T[] = [];

    /**
     * Adds a callback to be executed when this event is fired.
     * @param handler The callback to add
     * @returns This Event instance for method chaining
     */
    Add(handler: T): this {
        if (typeof handler === 'function') {
            this.handlers.push(handler);
        }
        return this;
    }

    /**
     * Adds a callback that will be executed only once when this event is fired.
     * @param handler The callback to add for one-time execution
     * @returns This Event instance for method chaining
     */
    AddOnce(handler: T): this {
        if (typeof handler === 'function') {
            this.onceHandlers.push(handler);
        }
        return this;
    }

    /**
     * Removes a callback from this event.
     * @param handler The callback to remove
     * @returns This Event instance for method chaining
     */
    Remove(handler: T): this {
        const index = this.handlers.indexOf(handler);
        if (index > -1) {
            this.handlers.splice(index, 1);
        }
        
        const onceIndex = this.onceHandlers.indexOf(handler);
        if (onceIndex > -1) {
            this.onceHandlers.splice(onceIndex, 1);
        }
        
        return this;
    }

    /**
     * Fires the event, executing all registered callbacks with the provided arguments.
     * @param args The arguments to pass to the callbacks
     * @returns This Event instance for method chaining
     */
    Fire(...args: Parameters<T>): this {
        // Execute regular handlers
        for (const handler of this.handlers) {
            handler(...args);
        }
        
        // Execute one-time handlers
        for (const handler of this.onceHandlers) {
            handler(...args);
        }
        
        // Clear one-time handlers after execution
        this.onceHandlers = [];
        
        return this;
    }

    /**
     * Checks if this event has any handlers registered.
     * @returns True if there are handlers registered, false otherwise
     */
    IsEmpty(): boolean {
        return this.handlers.length === 0 && this.onceHandlers.length === 0;
    }

    /**
     * Gets the number of registered handlers.
     * @returns The count of registered handlers
     */
    GetCount(): number {
        return this.handlers.length + this.onceHandlers.length;
    }

    /**
     * Clears all registered handlers.
     * @returns This Event instance for method chaining
     */
    Clear(): this {
        this.handlers = [];
        this.onceHandlers = [];
        return this;
    }

    /**
     * Checks if a specific handler is registered with this event.
     * @param handler The handler to check for
     * @returns True if the handler is registered, false otherwise
     */
    Has(handler: T): boolean {
        return this.handlers.includes(handler) || this.onceHandlers.includes(handler);
    }
}