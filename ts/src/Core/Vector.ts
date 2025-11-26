/**
 * Vector - Dynamic array container matching U++ Vector API
 *
 * Provides U++ Vector semantics using TypeScript/JavaScript Array as backing store.
 * Key differences from native Array:
 * - Uses U++ naming conventions (Add, GetCount, At, etc.)
 * - Provides pick/transfer semantics for efficient ownership transfer
 * - Includes U++ iteration patterns
 */

export class Vector<T> {
  private data: T[];

  constructor() {
    this.data = [];
  }

  /**
   * Add element to end of vector
   * @param value Element to add
   * @returns Reference to the added element
   */
  Add(value: T): T {
    this.data.push(value);
    return value;
  }

  /**
   * Add element and return its index
   * @param value Element to add
   * @returns Index of the added element
   */
  AddPick(value: T): number {
    this.data.push(value);
    return this.data.length - 1;
  }

  /**
   * Get element at index (const version)
   * @param i Index
   * @returns Element at index
   * @throws Error if index out of bounds
   */
  At(i: number): T {
    if (i < 0 || i >= this.data.length) {
      throw new Error(`Vector index out of bounds: ${i}`);
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
      throw new Error(`Vector index out of bounds: ${i}`);
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
   * Check if vector is empty
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
   * Shrink vector to specified size (removing elements from end)
   * @param n New size
   */
  Shrink(n: number): void {
    if (n < this.data.length) {
      this.data.length = n;
    }
  }

  /**
   * Set vector size with optional fill value
   * @param n New size
   * @param fillValue Optional value for new elements
   */
  SetCount(n: number, fillValue?: T): void {
    if (n < this.data.length) {
      this.data.length = n;
    } else if (n > this.data.length) {
      const toAdd = n - this.data.length;
      for (let i = 0; i < toAdd; i++) {
        this.data.push(fillValue as T);
      }
    }
  }

  /**
   * Remove element at index
   * @param i Index to remove
   */
  Remove(i: number): void {
    if (i < 0 || i >= this.data.length) {
      throw new Error(`Vector index out of bounds: ${i}`);
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
      throw new Error(`Vector index out of bounds: ${i}`);
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
      throw new Error(`Vector index out of bounds: ${i}`);
    }
    this.data.splice(i, 0, value);
  }

  /**
   * Insert element at index with pick semantics
   * @param i Index
   * @param value Value to insert
   * @returns Index where inserted
   */
  InsertPick(i: number, value: T): number {
    this.Insert(i, value);
    return i;
  }

  /**
   * Get first element
   * @returns First element
   * @throws Error if vector is empty
   */
  Top(): T {
    if (this.data.length === 0) {
      throw new Error('Vector is empty');
    }
    return this.data[this.data.length - 1];
  }

  /**
   * Remove and return last element
   * @returns Last element
   * @throws Error if vector is empty
   */
  Pop(): T {
    if (this.data.length === 0) {
      throw new Error('Vector is empty');
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
   * Find element by value (using strict equality)
   * @param value Value to find
   * @returns Index of first matching element, or -1 if not found
   */
  FindValue(value: T): number {
    return this.data.indexOf(value);
  }

  /**
   * Append another vector to this one
   * @param other Vector to append
   */
  Append(other: Vector<T>): void {
    this.data.push(...other.data);
  }

  /**
   * Get underlying array (for iteration, advanced use)
   * Warning: Direct modifications bypass Vector semantics
   * @returns Internal array reference
   */
  GetData(): T[] {
    return this.data;
  }

  /**
   * Create vector from array
   * @param array Source array
   * @returns New Vector containing array elements
   */
  static From<T>(array: T[]): Vector<T> {
    const v = new Vector<T>();
    v.data = [...array];
    return v;
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
    return `Vector[${this.data.length}](${this.data.join(', ')})`;
  }
}
