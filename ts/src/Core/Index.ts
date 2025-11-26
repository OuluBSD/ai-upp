/**
 * Index - Sorted container with fast lookup
 *
 * U++ Index maintains elements in sorted order and provides O(log n) lookup.
 * Uses binary search for efficient Find operations.
 * Elements must be comparable (provide comparison function or use default).
 */

export class Index<T> {
  private data: T[];
  private compareFn: (a: T, b: T) => number;

  /**
   * Create new Index with optional comparison function
   * @param compareFn Comparison function (returns <0, 0, >0 for a<b, a==b, a>b)
   */
  constructor(compareFn?: (a: T, b: T) => number) {
    this.data = [];
    this.compareFn = compareFn || this.defaultCompare;
  }

  private defaultCompare(a: T, b: T): number {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
  }

  /**
   * Binary search to find insertion position
   * @param value Value to search for
   * @returns Index where value should be inserted
   */
  private binarySearch(value: T): number {
    let left = 0;
    let right = this.data.length;

    while (left < right) {
      const mid = Math.floor((left + right) / 2);
      if (this.compareFn(this.data[mid], value) < 0) {
        left = mid + 1;
      } else {
        right = mid;
      }
    }

    return left;
  }

  /**
   * Add element maintaining sorted order
   * @param value Element to add
   * @returns Index where element was inserted
   */
  Add(value: T): number {
    const pos = this.binarySearch(value);
    this.data.splice(pos, 0, value);
    return pos;
  }

  /**
   * Find element by value
   * @param value Value to find
   * @returns Index of element, or -1 if not found
   */
  Find(value: T): number {
    const pos = this.binarySearch(value);
    if (pos < this.data.length && this.compareFn(this.data[pos], value) === 0) {
      return pos;
    }
    return -1;
  }

  /**
   * Find element using custom predicate
   * @param predicate Function to test elements
   * @returns Index of first matching element, or -1 if not found
   */
  FindMatch(predicate: (value: T) => boolean): number {
    return this.data.findIndex(predicate);
  }

  /**
   * Check if value exists in index
   * @param value Value to check
   * @returns true if value exists
   */
  Contains(value: T): boolean {
    return this.Find(value) !== -1;
  }

  /**
   * Get element at index
   * @param i Index
   * @returns Element at index
   * @throws Error if index out of bounds
   */
  At(i: number): T {
    if (i < 0 || i >= this.data.length) {
      throw new Error(`Index out of bounds: ${i}`);
    }
    return this.data[i];
  }

  /**
   * Get number of elements
   * @returns Element count
   */
  GetCount(): number {
    return this.data.length;
  }

  /**
   * Check if index is empty
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
   * Remove element at index
   * @param i Index to remove
   */
  RemoveAt(i: number): void {
    if (i < 0 || i >= this.data.length) {
      throw new Error(`Index out of bounds: ${i}`);
    }
    this.data.splice(i, 1);
  }

  /**
   * Remove element by value
   * @param value Value to remove
   * @returns true if element was found and removed
   */
  RemoveKey(value: T): boolean {
    const idx = this.Find(value);
    if (idx !== -1) {
      this.data.splice(idx, 1);
      return true;
    }
    return false;
  }

  /**
   * Get all elements as array (sorted)
   * @returns Copy of internal array
   */
  GetKeys(): T[] {
    return [...this.data];
  }

  /**
   * Insert or find existing element
   * Returns tuple of [index, wasInserted]
   * @param value Value to insert
   * @returns [index, true] if inserted, [index, false] if already existed
   */
  FindAdd(value: T): [number, boolean] {
    const pos = this.binarySearch(value);
    if (pos < this.data.length && this.compareFn(this.data[pos], value) === 0) {
      return [pos, false];
    }
    this.data.splice(pos, 0, value);
    return [pos, true];
  }

  /**
   * Get element at index or return default value
   * @param i Index
   * @param defaultValue Default value if index out of bounds
   * @returns Element at index or default
   */
  Get(i: number, defaultValue: T): T {
    if (i < 0 || i >= this.data.length) {
      return defaultValue;
    }
    return this.data[i];
  }

  /**
   * Detach element at index (remove and return)
   * @param i Index
   * @returns Removed element
   */
  Detach(i: number): T {
    if (i < 0 || i >= this.data.length) {
      throw new Error(`Index out of bounds: ${i}`);
    }
    const element = this.data[i];
    this.data.splice(i, 1);
    return element;
  }

  /**
   * Get last element
   * @returns Last element
   * @throws Error if index is empty
   */
  Top(): T {
    if (this.data.length === 0) {
      throw new Error('Index is empty');
    }
    return this.data[this.data.length - 1];
  }

  /**
   * Get first element
   * @returns First element
   * @throws Error if index is empty
   */
  Head(): T {
    if (this.data.length === 0) {
      throw new Error('Index is empty');
    }
    return this.data[0];
  }

  /**
   * Create Index from array
   * @param array Source array
   * @param compareFn Optional comparison function
   * @returns New Index containing sorted elements
   */
  static From<T>(array: T[], compareFn?: (a: T, b: T) => number): Index<T> {
    const idx = new Index<T>(compareFn);
    for (const item of array) {
      idx.Add(item);
    }
    return idx;
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
    return `Index[${this.data.length}](${this.data.join(', ')})`;
  }
}
