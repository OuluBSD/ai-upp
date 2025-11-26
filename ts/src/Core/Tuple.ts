/**
 * A tuple class similar to U++ Tuple, implementing a fixed-size collection of heterogeneous values.
 *
 * @template T A variadic type tuple containing the types of elements in the tuple
 *
 * @example
 * ```typescript
 * const tuple = new Tuple(1, "hello", true);
 * console.log(tuple.Get(0)); // 1
 * console.log(tuple.Get(1)); // "hello"
 * console.log(tuple.Get(2)); // true
 * console.log(tuple.GetCount()); // 3
 * ```
 */
export class Tuple<T extends readonly any[]> {
    private elements: T;

    /**
     * Creates a new Tuple with the provided values.
     * @param elements The elements to store in the tuple
     */
    constructor(...elements: T) {
        if (elements.length < 1) {
            throw new Error("Tuple must have at least one element");
        }
        this.elements = elements as T;
    }

    /**
     * Accesses the element at the specified index in the tuple.
     * @param index The index of the element to access
     * @returns The element at the specified index
     */
    Get<K extends keyof T & number>(index: K): T[K] {
        if (index < 0 || index >= this.elements.length) {
            throw new Error(`Index ${index} out of bounds for tuple of length ${this.elements.length}`);
        }
        return this.elements[index];
    }

    /**
     * Gets the number of elements in the tuple.
     * @returns The count of elements in the tuple
     */
    GetCount(): number {
        return this.elements.length;
    }

    /**
     * Checks if the tuple is empty.
     * @returns True if the tuple has no elements, false otherwise
     */
    IsEmpty(): boolean {
        return this.elements.length === 0;
    }

    /**
     * Returns the element at the specified index using array-like access.
     * @param index The index of the element to access
     * @returns The element at the specified index
     */
    At<K extends keyof T & number>(index: K): T[K] {
        return this.Get(index);
    }

    /**
     * Converts the tuple to an array.
     * @returns An array containing all elements of the tuple
     */
    ToArray(): readonly [...T] {
        return [...this.elements] as readonly [...T];
    }

    /**
     * Gets the underlying elements array.
     * @returns The elements array
     */
    Elements(): T {
        return this.elements;
    }

    /**
     * Sets the element at the specified index.
     * @param index The index of the element to set
     * @param value The new value to set
     */
    Set<K extends keyof T & number>(index: K, value: T[K]): void {
        if (index < 0 || index >= this.elements.length) {
            throw new Error(`Index ${index} out of bounds for tuple of length ${this.elements.length}`);
        }
        // Using tuple as mutable array for this assignment
        (this.elements as unknown as any[])[index] = value;
    }

    /**
     * Creates a new tuple with the provided elements.
     * @param elements The elements for the new tuple
     * @returns A new Tuple instance
     */
    static Make<T extends [any, ...any[]]>(...elements: T): Tuple<T> {
        return new Tuple(...elements);
    }
}