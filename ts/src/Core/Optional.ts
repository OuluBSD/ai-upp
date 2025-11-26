/**
 * A nullable wrapper class similar to U++ Optional, representing a value that may or may not be present.
 * 
 * @template T The type of the value that may be contained in the Optional
 * 
 * @example
 * ```typescript
 * let opt = new Optional<string>();
 * console.log(opt.IsNull()); // true
 * 
 * opt = Optional.Of("hello");
 * console.log(opt.IsNull()); // false
 * console.log(opt.Get()); // "hello"
 * 
 * opt.Clear();
 * console.log(opt.IsNull()); // true
 * ```
 */
export class Optional<T> {
    private value: T | null;
    private hasValue: boolean;

    /**
     * Constructs an empty Optional with no value.
     */
    constructor() {
        this.value = null;
        this.hasValue = false;
    }

    /**
     * Checks if this Optional contains a value.
     * @returns True if this Optional contains a value, false otherwise
     */
    IsNull(): boolean {
        return !this.hasValue;
    }

    /**
     * Gets the value contained in this Optional.
     * @returns The value if present, otherwise throws an error
     */
    Get(): T {
        if (!this.hasValue) {
            throw new Error("Optional is empty, no value available");
        }
        return this.value!;
    }

    /**
     * Gets the value contained in this Optional or a default value if empty.
     * @param defaultValue The default value to return if this Optional is empty
     * @returns The contained value or the default value
     */
    GetWithDefault(defaultValue: T): T {
        return this.hasValue ? this.value! : defaultValue;
    }

    /**
     * Sets the value in this Optional.
     * @param value The value to store in this Optional
     */
    Set(value: T): void {
        this.value = value;
        this.hasValue = true;
    }

    /**
     * Clears the value from this Optional.
     */
    Clear(): void {
        this.value = null;
        this.hasValue = false;
    }

    /**
     * Creates an Optional containing the specified non-null value.
     * @param value The value to be contained in the Optional
     * @returns An Optional with the specified value
     */
    static Of<T>(value: T | null | undefined): Optional<T> {
        if (value == null) {
            return new Optional<T>();
        }
        const opt = new Optional<T>();
        opt.value = value;
        opt.hasValue = true;
        return opt;
    }

    /**
     * Creates an empty Optional.
     * @returns An empty Optional
     */
    static Null<T>(): Optional<T> {
        return new Optional<T>();
    }

    /**
     * Transfers ownership of the value from this Optional to the caller.
     * This Optional becomes empty after the call.
     * @returns The value if present, otherwise throws an error
     */
    Pick(): T {
        if (!this.hasValue) {
            throw new Error("Optional is empty, no value available to pick");
        }
        const result = this.value!;
        this.Clear();
        return result;
    }

    /**
     * Checks if this Optional contains a value that is equal to the specified value.
     * @param other The value to compare with
     * @returns True if this Optional contains a value equal to the specified value
     */
    IsEqual(other: T | null): boolean {
        if (other === null || other === undefined) {
            return this.IsNull();
        }
        return !this.IsNull() && this.value === other;
    }
}