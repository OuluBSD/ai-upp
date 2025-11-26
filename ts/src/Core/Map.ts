/**
 * Map - Key-value container with U++ interface
 *
 * U++ VectorMap/ArrayMap stores key-value pairs with fast lookup by key.
 * This implementation uses JavaScript Map as backing store for O(1) lookup.
 * Unlike Index, keys don't need to be comparable - uses hash-based lookup.
 */

export class Map<K, V> {
  private data: globalThis.Map<K, V>;

  /**
   * Create new Map
   */
  constructor() {
    this.data = new globalThis.Map<K, V>();
  }

  /**
   * Add key-value pair
   * Overwrites existing value if key exists
   * @param key Key
   * @param value Value
   * @returns Reference to the added value
   */
  Add(key: K, value: V): V {
    this.data.set(key, value);
    return value;
  }

  /**
   * Get value by key
   * @param key Key to find
   * @param defaultValue Default value if key not found
   * @returns Value associated with key, or default if not found
   */
  Get(key: K, defaultValue: V): V {
    if (this.data.has(key)) {
      return this.data.get(key)!;
    }
    return defaultValue;
  }

  /**
   * Get value by key, throws if not found
   * @param key Key to find
   * @returns Value associated with key
   * @throws Error if key not found
   */
  GetThrow(key: K): V {
    if (!this.data.has(key)) {
      throw new Error('Key not found in Map');
    }
    return this.data.get(key)!;
  }

  /**
   * Find key position in iteration order
   * @param key Key to find
   * @returns Index of key in iteration order, or -1 if not found
   */
  Find(key: K): number {
    let i = 0;
    for (const k of this.data.keys()) {
      if (k === key) {
        return i;
      }
      i++;
    }
    return -1;
  }

  /**
   * Check if key exists
   * @param key Key to check
   * @returns true if key exists
   */
  Contains(key: K): boolean {
    return this.data.has(key);
  }

  /**
   * Set value for existing key or add new key-value pair
   * @param key Key
   * @param value Value
   */
  Set(key: K, value: V): void {
    this.data.set(key, value);
  }

  /**
   * Get key at index in iteration order
   * @param i Index
   * @returns Key at index
   * @throws Error if index out of bounds
   */
  GetKey(i: number): K {
    if (i < 0 || i >= this.data.size) {
      throw new Error(`Map index out of bounds: ${i}`);
    }
    let idx = 0;
    for (const key of this.data.keys()) {
      if (idx === i) {
        return key;
      }
      idx++;
    }
    throw new Error(`Map index out of bounds: ${i}`);
  }

  /**
   * Get value at index in iteration order
   * @param i Index
   * @returns Value at index
   * @throws Error if index out of bounds
   */
  GetValue(i: number): V {
    if (i < 0 || i >= this.data.size) {
      throw new Error(`Map index out of bounds: ${i}`);
    }
    let idx = 0;
    for (const value of this.data.values()) {
      if (idx === i) {
        return value;
      }
      idx++;
    }
    throw new Error(`Map index out of bounds: ${i}`);
  }

  /**
   * Get number of entries
   * @returns Entry count
   */
  GetCount(): number {
    return this.data.size;
  }

  /**
   * Check if map is empty
   * @returns true if empty
   */
  IsEmpty(): boolean {
    return this.data.size === 0;
  }

  /**
   * Clear all entries
   */
  Clear(): void {
    this.data.clear();
  }

  /**
   * Remove entry by key
   * @param key Key to remove
   * @returns true if key was found and removed
   */
  RemoveKey(key: K): boolean {
    return this.data.delete(key);
  }

  /**
   * Remove entry at index
   * @param i Index to remove
   */
  RemoveAt(i: number): void {
    const key = this.GetKey(i);
    this.data.delete(key);
  }

  /**
   * Get all keys as array
   * @returns Array of keys
   */
  GetKeys(): K[] {
    return Array.from(this.data.keys());
  }

  /**
   * Get all values as array
   * @returns Array of values
   */
  GetValues(): V[] {
    return Array.from(this.data.values());
  }

  /**
   * Insert or update key-value pair
   * Returns tuple of [index, wasInserted]
   * @param key Key
   * @param value Value
   * @returns [index, true] if inserted, [index, false] if updated
   */
  FindAdd(key: K, value: V): [number, boolean] {
    const existed = this.data.has(key);
    this.data.set(key, value);
    const idx = this.Find(key);
    return [idx, !existed];
  }

  /**
   * Get or create value for key
   * If key exists, returns existing value
   * If key doesn't exist, creates using factory and returns it
   * @param key Key
   * @param factory Factory function to create value if key not found
   * @returns Existing or newly created value
   */
  GetAdd(key: K, factory: () => V): V {
    if (this.data.has(key)) {
      return this.data.get(key)!;
    }
    const value = factory();
    this.data.set(key, value);
    return value;
  }

  /**
   * Create Map from entries
   * @param entries Array of [key, value] pairs
   * @returns New Map containing the entries
   */
  static From<K, V>(entries: [K, V][]): Map<K, V> {
    const map = new Map<K, V>();
    for (const [key, value] of entries) {
      map.Add(key, value);
    }
    return map;
  }

  /**
   * Iterator support for for-of loops (iterates over [key, value] pairs)
   */
  [Symbol.iterator](): Iterator<[K, V]> {
    return this.data[Symbol.iterator]();
  }

  /**
   * Get entries iterator (for compatibility)
   */
  entries(): IterableIterator<[K, V]> {
    return this.data.entries();
  }

  /**
   * Get keys iterator
   */
  keys(): IterableIterator<K> {
    return this.data.keys();
  }

  /**
   * Get values iterator
   */
  values(): IterableIterator<V> {
    return this.data.values();
  }

  /**
   * Get string representation
   */
  toString(): string {
    const entries = Array.from(this.data.entries())
      .map(([k, v]) => `${k}: ${v}`)
      .join(', ');
    return `Map[${this.data.size}](${entries})`;
  }
}
