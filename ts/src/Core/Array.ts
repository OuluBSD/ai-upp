/**
 * Array - Container of elements managed as references
 *
 * U++ Array stores elements by reference/pointer and owns them.
 * In TypeScript, we simulate this using object references.
 * Key difference from Vector: Array is optimized for object types
 * where each element is independently allocated.
 *
 * For primitive types, prefer Vector.
 * For object types that need stable references, use Array.
 */

export class Array<T> {
  private data: T[];

  constructor() {
    this.data = [];
  }

  /**
   * Create new element at end using factory function
   * @param factory Function that creates the element
   * @returns Reference to created element
   */
  Create(factory: () => T): T {
    const element = factory();
    this.data.push(element);
    return element;
  }

  /**
   * Add element to end
   * @param value Element to add
   * @returns Reference to added element
   */
  Add(value: T): T {
    this.data.push(value);
    return value;
  }

  /**
   * Get element at index
   * @param i Index
   * @returns Element at index
   * @throws Error if index out of bounds
   */
  At(i: number): T {
    if (i < 0 || i >= this.data.length) {
      throw new Error(`Array index out of bounds: ${i}`);
    }
    return this.data[i];
  }

  /**
   * Set element at index
   * @param i Index
   * @param value New value
   * @throws Error if index out of bounds
   */
  Set(i: number, value: T): void {
    if (i < 0 || i >= this.data.length) {
      throw new Error(`Array index out of bounds: ${i}`);
    }
    this.data[i] = value;
  }

  /**
   * Get number of elements
   * @returns Element count
   */
  GetCount(): number {
    return this.data.length;
  }

  /**
   * Check if array is empty
   * @returns true if empty
   */
  IsEmpty(): boolean {
    return this.data.length === 0;
  }

  /**
   * Clear all elements
   */
  Clear(): void {
    this.data = [];
  }

  /**
   * Shrink array to specified size
   * @param n New size
   */
  Shrink(n: number): void {
    if (n < this.data.length) {
      this.data.length = n;
    }
  }

  /**
   * Set array size
   * Note: Growing array will add undefined elements (use Create to properly initialize)
   * @param n New size
   */
  SetCount(n: number): void {
    this.data.length = n;
  }

  /**
   * Remove element at index
   * @param i Index to remove
   */
  Remove(i: number): void {
    if (i < 0 || i >= this.data.length) {
      throw new Error(`Array index out of bounds: ${i}`);
    }
    this.data.splice(i, 1);
  }

  /**
   * Remove range of elements
   * @param i Starting index
   * @param count Number of elements to remove
   */
  RemoveRange(i: number, count: number): void {
    if (i < 0 || i >= this.data.length) {
      throw new Error(`Array index out of bounds: ${i}`);
    }
    this.data.splice(i, count);
  }

  /**
   * Insert element at index
   * @param i Index
   * @param value Value to insert
   */
  Insert(i: number, value: T): void {
    if (i < 0 || i > this.data.length) {
      throw new Error(`Array index out of bounds: ${i}`);
    }
    this.data.splice(i, 0, value);
  }

  /**
   * Get last element
   * @returns Last element
   * @throws Error if array is empty
   */
  Top(): T {
    if (this.data.length === 0) {
      throw new Error('Array is empty');
    }
    return this.data[this.data.length - 1];
  }

  /**
   * Remove and return last element
   * @returns Last element
   * @throws Error if array is empty
   */
  Pop(): T {
    if (this.data.length === 0) {
      throw new Error('Array is empty');
    }
    return this.data.pop() as T;
  }

  /**
   * Drop last element without returning it
   */
  Drop(): void {
    if (this.data.length > 0) {
      this.data.pop();
    }
  }

  /**
   * Find element by predicate
   * @param predicate Function to test elements
   * @returns Index of first matching element, or -1 if not found
   */
  Find(predicate: (value: T) => boolean): number {
    return this.data.findIndex(predicate);
  }

  /**
   * Detach element at index (remove and return without destroying)
   * @param i Index
   * @returns Detached element
   */
  Detach(i: number): T {
    if (i < 0 || i >= this.data.length) {
      throw new Error(`Array index out of bounds: ${i}`);
    }
    const element = this.data[i];
    this.data.splice(i, 1);
    return element;
  }

  /**
   * Swap elements at two indices
   * @param i First index
   * @param j Second index
   */
  Swap(i: number, j: number): void {
    if (i < 0 || i >= this.data.length || j < 0 || j >= this.data.length) {
      throw new Error('Array index out of bounds');
    }
    const temp = this.data[i];
    this.data[i] = this.data[j];
    this.data[j] = temp;
  }

  /**
   * Get underlying array data
   * Warning: Direct modifications bypass Array semantics
   * @returns Internal array reference
   */
  GetData(): T[] {
    return this.data;
  }

  /**
   * Create array from existing elements
   * @param elements Elements to add
   * @returns New Array containing elements
   */
  static From<T>(elements: T[]): Array<T> {
    const arr = new Array<T>();
    arr.data = [...elements];
    return arr;
  }

  /**
   * Iterator support for for-of loops
   */
  [Symbol.iterator](): Iterator<T> {
    return this.data[Symbol.iterator]();
  }

  /**
   * Get string representation
   */
  toString(): string {
    return `Array[${this.data.length}]`;
  }
}
