/**
 * BiVector - Bi-directional vector (deque) with U++ interface
 *
 * U++ BiVector provides efficient insertion and removal at both ends.
 * Implements double-ended queue (deque) operations with O(1) push/pop
 * at both front and back.
 *
 * Uses JavaScript Array as backing store. While JavaScript arrays have
 * O(n) unshift/shift operations, they're optimized by modern engines
 * and acceptable for most use cases. For truly high-performance deque,
 * consider a circular buffer implementation.
 */

export class BiVector<T> {
  private data: T[];

  /**
   * Create new BiVector
   */
  constructor() {
    this.data = [];
  }

  /**
   * Add element to back (end)
   * @param value Element to add
   * @returns Added element
   */
  AddTail(value: T): T {
    this.data.push(value);
    return value;
  }

  /**
   * Add element to front (beginning)
   * @param value Element to add
   * @returns Added element
   */
  AddHead(value: T): T {
    this.data.unshift(value);
    return value;
  }

  /**
   * Alias for AddTail (U++ compatibility)
   */
  Add(value: T): T {
    return this.AddTail(value);
  }

  /**
   * Remove and return element from back
   * @returns Removed element
   * @throws Error if empty
   */
  PopTail(): T {
    if (this.data.length === 0) {
      throw new Error('BiVector is empty');
    }
    return this.data.pop()!;
  }

  /**
   * Remove and return element from front
   * @returns Removed element
   * @throws Error if empty
   */
  PopHead(): T {
    if (this.data.length === 0) {
      throw new Error('BiVector is empty');
    }
    return this.data.shift()!;
  }

  /**
   * Get element from back without removing
   * @returns Last element
   * @throws Error if empty
   */
  Tail(): T {
    if (this.data.length === 0) {
      throw new Error('BiVector is empty');
    }
    return this.data[this.data.length - 1];
  }

  /**
   * Get element from front without removing
   * @returns First element
   * @throws Error if empty
   */
  Head(): T {
    if (this.data.length === 0) {
      throw new Error('BiVector is empty');
    }
    return this.data[0];
  }

  /**
   * Alias for Tail (U++ compatibility)
   */
  Top(): T {
    return this.Tail();
  }

  /**
   * Remove last element (no return)
   */
  DropTail(): void {
    if (this.data.length > 0) {
      this.data.pop();
    }
  }

  /**
   * Remove first element (no return)
   */
  DropHead(): void {
    if (this.data.length > 0) {
      this.data.shift();
    }
  }

  /**
   * Alias for DropTail
   */
  Drop(): void {
    this.DropTail();
  }

  /**
   * Get element at index
   * @param i Index (0 = first, negative values not supported)
   * @returns Element at index
   * @throws Error if index out of bounds
   */
  At(i: number): T {
    if (i < 0 || i >= this.data.length) {
      throw new Error(`BiVector index out of bounds: ${i}`);
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
      throw new Error(`BiVector index out of bounds: ${i}`);
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
   * Check if empty
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
   * Insert element at index
   * @param i Index
   * @param value Element to insert
   */
  Insert(i: number, value: T): void {
    if (i < 0 || i > this.data.length) {
      throw new Error(`BiVector index out of bounds: ${i}`);
    }
    this.data.splice(i, 0, value);
  }

  /**
   * Remove element at index
   * @param i Index
   */
  Remove(i: number): void {
    if (i < 0 || i >= this.data.length) {
      throw new Error(`BiVector index out of bounds: ${i}`);
    }
    this.data.splice(i, 1);
  }

  /**
   * Remove range of elements
   * @param start Start index (inclusive)
   * @param count Number of elements to remove
   */
  RemoveRange(start: number, count: number): void {
    if (start < 0 || start >= this.data.length) {
      throw new Error(`BiVector index out of bounds: ${start}`);
    }
    this.data.splice(start, count);
  }

  /**
   * Shrink to size (keep first n elements)
   * @param size New size
   */
  Shrink(size: number): void {
    if (size < this.data.length) {
      this.data.length = size;
    }
  }

  /**
   * Set size (shrink or grow)
   * @param size New size
   */
  SetCount(size: number): void {
    this.data.length = size;
  }

  /**
   * Find element by predicate
   * @param predicate Test function
   * @returns Index of first matching element, or -1
   */
  Find(predicate: (value: T) => boolean): number {
    return this.data.findIndex(predicate);
  }

  /**
   * Find element by value
   * @param value Value to find
   * @returns Index of first occurrence, or -1
   */
  FindValue(value: T): number {
    return this.data.indexOf(value);
  }

  /**
   * Remove and return element at index
   * @param i Index
   * @returns Removed element
   */
  Detach(i: number): T {
    if (i < 0 || i >= this.data.length) {
      throw new Error(`BiVector index out of bounds: ${i}`);
    }
    const element = this.data[i];
    this.data.splice(i, 1);
    return element;
  }

  /**
   * Get internal array (copy)
   * @returns Copy of internal array
   */
  GetData(): T[] {
    return [...this.data];
  }

  /**
   * Append elements from array
   * @param values Array of values to append
   */
  Append(values: T[]): void {
    this.data.push(...values);
  }

  /**
   * Prepend elements from array
   * @param values Array of values to prepend
   */
  Prepend(values: T[]): void {
    this.data.unshift(...values);
  }

  /**
   * Create BiVector from array
   * @param array Source array
   * @returns New BiVector containing array elements
   */
  static From<T>(array: T[]): BiVector<T> {
    const bv = new BiVector<T>();
    bv.data = [...array];
    return bv;
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
    return `BiVector[${this.data.length}](${this.data.join(', ')})`;
  }
}
