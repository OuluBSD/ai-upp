/**
 * A variant class similar to U++ Value, representing a value that can hold different types.
 * This implementation stores type information alongside the value to enable type checking.
 *
 * @example
 * ```typescript
 * let value = new Value(42);
 * console.log(value.Is("number")); // true
 * console.log(value.Get()); // 42
 *
 * value.Set("hello");
 * console.log(value.Is("string")); // true
 * console.log(value.Get()); // "hello"
 * ```
 */
export class Value<T> {
    private data: T;
    private dataType: string;

    /**
     * Creates a new Value instance with the provided initial value.
     * @param value The initial value to store
     */
    constructor(value?: T) {
        if (value !== undefined) {
            this.data = value;
            this.dataType = typeof value;
        } else {
            this.data = undefined as unknown as T;
            this.dataType = 'undefined';
        }
    }

    /**
     * Checks if this Value contains a value of the specified type.
     * @param expectedType The type to check for (e.g., "number", "string", "boolean", "object", "function")
     * @returns True if the stored value is of the specified type, false otherwise
     */
    Is(expectedType: string): boolean {
        return this.dataType === expectedType;
    }

    /**
     * Gets the value contained in this Value.
     * @returns The stored value (type safety is the responsibility of the caller)
     */
    Get(): T {
        return this.data;
    }

    /**
     * Sets a new value in this Value.
     * @param value The value to store
     */
    Set<U>(value: U): void {
        this.data = value as unknown as T;
        this.dataType = typeof value;
    }

    /**
     * Gets the type of the stored value as a string.
     * @returns The string representation of the stored value's type
     */
    GetType(): string {
        return this.dataType;
    }

    /**
     * Checks if this Value is empty or contains undefined.
     * @returns True if the Value is undefined, false otherwise
     */
    IsVoid(): boolean {
        return this.dataType === 'undefined';
    }

    /**
     * Clears the value, setting it to undefined.
     */
    Clear(): void {
        this.data = undefined as unknown as T;
        this.dataType = 'undefined';
    }

    /**
     * Creates a Value from the provided data.
     * @param data The data to store in the Value
     * @returns A new Value instance
     */
    static Create<T>(data: T): Value<T> {
        return new Value(data);
    }

    /**
     * Transfers ownership of the value from this Value to the caller.
     * This Value becomes empty after the call.
     * @returns The stored value
     */
    Pick(): T {
        const result = this.data;
        this.Clear();
        return result;
    }
}

/**
 * A more flexible variant class that can hold any of a set of types.
 * This implementation stores type information alongside the value to enable type checking.
 *
 * @example
 * ```typescript
 * let variant = new Variant(42);
 * console.log(variant.Is("number")); // true
 * console.log(variant.Get()); // 42
 *
 * variant.Set("hello");
 * console.log(variant.Is("string")); // true
 * console.log(variant.Get()); // "hello"
 * ```
 */
export class Variant<T> {
    private data: T;
    private dataType: string;

    /**
     * Creates a new Variant instance with the provided initial value.
     * @param value The initial value to store
     */
    constructor(value: T) {
        this.data = value;
        this.dataType = typeof value;
    }

    /**
     * Checks if this Variant contains a value of the specified type.
     * @param expectedType The type to check for (e.g., "number", "string", "boolean", "object", "function")
     * @returns True if the stored value is of the specified type, false otherwise
     */
    Is(expectedType: string): boolean {
        return this.dataType === expectedType;
    }

    /**
     * Gets the value contained in this Variant.
     * @returns The stored value (type safety is the responsibility of the caller)
     */
    Get(): T {
        return this.data;
    }

    /**
     * Sets a new value in this Variant.
     * @param value The value to store
     */
    Set<U extends T>(value: U): void {
        this.data = value;
        this.dataType = typeof value;
    }

    /**
     * Gets the type of the stored value as a string.
     * @returns The string representation of the stored value's type
     */
    GetType(): string {
        return this.dataType;
    }

    /**
     * Transfers ownership of the value from this Variant to the caller.
     * This Variant becomes empty after the call.
     * @returns The stored value
     */
    Pick(): T {
        const result = this.data;
        this.data = undefined as unknown as T;
        this.dataType = 'undefined';
        return result;
    }
}