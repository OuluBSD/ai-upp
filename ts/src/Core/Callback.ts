/**
 * A callback class similar to U++ Callback, representing a function that can be called later.
 * Provides a consistent interface for callback functions with type checking.
 * 
 * @template TArgs The types of arguments the callback accepts
 * @template TReturn The return type of the callback
 * 
 * @example
 * ```typescript
 * const callback = new Callback<(name: string) => void>((name: string) => {
 *     console.log(`Hello, ${name}!`);
 * });
 * callback.Execute("World"); // "Hello, World!"
 * 
 * const add = new Callback<(a: number, b: number) => number>((a: number, b: number) => a + b);
 * console.log(add.Execute(5, 3)); // 8
 * ```
 */
export class Callback<T extends (...args: any[]) => any> {
    private callback: T | null;
    private context: any | null;

    /**
     * Creates a new Callback instance.
     * @param callback The callback function to store, or null to create an empty Callback
     * @param context Optional context to bind the callback to
     */
    constructor(callback?: T | null, context?: any) {
        this.callback = callback || null;
        this.context = context || null;
    }

    /**
     * Checks if this Callback has a valid function to execute.
     * @returns True if this Callback has a function to execute, false otherwise
     */
    IsNull(): boolean {
        return this.callback === null;
    }

    /**
     * Executes the stored callback with the provided arguments.
     * @param args The arguments to pass to the callback
     * @returns The result of the callback execution
     */
    Execute(...args: Parameters<T>): ReturnType<T> {
        if (this.callback === null) {
            throw new Error("Cannot execute null callback");
        }
        
        if (this.context !== null) {
            return this.callback.apply(this.context, args);
        }
        
        return this.callback(...args);
    }

    /**
     * Sets a new callback function.
     * @param callback The new callback function
     * @param context Optional context to bind the callback to
     */
    Set(callback: T | null, context?: any): void {
        this.callback = callback;
        if (context !== undefined) {
            this.context = context;
        }
    }

    /**
     * Clears the callback, making this Callback null.
     */
    Clear(): void {
        this.callback = null;
        this.context = null;
    }

    /**
     * Creates a Callback from the provided function.
     * @param callback The function to wrap as a callback
     * @param context Optional context to bind the callback to
     * @returns A new Callback instance
     */
    static Create<T extends (...args: any[]) => any>(callback: T, context?: any): Callback<T> {
        return new Callback(callback, context);
    }

    /**
     * Transfers ownership of the callback function from this Callback to the caller.
     * This Callback becomes empty after the call.
     * @returns The stored callback function
     */
    Pick(): T {
        if (this.callback === null) {
            throw new Error("Cannot pick from null callback");
        }
        const result = this.callback;
        this.Clear();
        return result;
    }

    /**
     * Sets the context for this callback. This is useful when the callback is a method
     * of an object and needs to maintain proper 'this' binding.
     * @param context The context to bind to
     * @returns This Callback instance for method chaining
     */
    operator_scope(context: any): this {
        this.context = context;
        return this;
    }
}