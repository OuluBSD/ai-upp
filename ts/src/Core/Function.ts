/**
 * A function wrapper class similar to U++ Function, wrapping a function with specified signature.
 * Provides a consistent interface for function objects with type checking.
 * 
 * @template TArgs The types of arguments the function accepts
 * @template TReturn The return type of the function
 * 
 * @example
 * ```typescript
 * const add = (a: number, b: number): number => a + b;
 * const func = new Function<(a: number, b: number) => number>(add);
 * console.log(func.Execute(5, 3)); // 8
 * 
 * const greet = (name: string): string => `Hello, ${name}!`;
 * const func2 = new Function<(name: string) => string>(greet);
 * console.log(func2.Execute("World")); // "Hello, World!"
 * ```
 */
export class Function<T extends (...args: any[]) => any> {
    private func: T | null;

    /**
     * Creates a new Function wrapper.
     * @param func The function to wrap, or null to create an empty Function
     */
    constructor(func?: T | null) {
        this.func = func || null;
    }

    /**
     * Checks if this Function has a valid function to execute.
     * @returns True if this Function has a function to execute, false otherwise
     */
    IsNull(): boolean {
        return this.func === null;
    }

    /**
     * Executes the wrapped function with the provided arguments.
     * @param args The arguments to pass to the function
     * @returns The result of the function execution
     */
    Execute(...args: Parameters<T>): ReturnType<T> {
        if (this.func === null) {
            throw new Error("Cannot execute null function");
        }
        return this.func(...args);
    }

    /**
     * Sets a new function to be wrapped.
     * @param func The new function to wrap
     */
    Set(func: T | null): void {
        this.func = func;
    }

    /**
     * Clears the function, making this Function null.
     */
    Clear(): void {
        this.func = null;
    }

    /**
     * Creates a Function from the provided function.
     * @param func The function to wrap
     * @returns A new Function instance
     */
    static Create<T extends (...args: any[]) => any>(func: T): Function<T> {
        return new Function(func);
    }

    /**
     * Transfers ownership of the wrapped function from this Function to the caller.
     * This Function becomes empty after the call.
     * @returns The wrapped function
     */
    Pick(): T {
        if (this.func === null) {
            throw new Error("Cannot pick from null function");
        }
        const result = this.func;
        this.Clear();
        return result;
    }
}