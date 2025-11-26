/**
 * One<T> - Unique ownership smart pointer
 *
 * Provides unique ownership semantics similar to std::unique_ptr in C++.
 * In U++, One<T> represents single ownership - the object is owned by
 * exactly one container/variable and cannot be copied, only moved.
 *
 * In TypeScript, we simulate this with:
 * - Move semantics via Detach/Attach
 * - Optional ownership tracking
 * - Automatic cleanup on Clear
 *
 * Key features:
 * - Single ownership (no copying, only moving)
 * - Automatic memory management
 * - Null-safe access
 * - Pick semantics for transfer
 */

/**
 * One<T> - Unique ownership container
 *
 * Holds a single value with unique ownership.
 * The value can be moved but not copied.
 *
 * @template T Type of the owned value
 *
 * @example
 * ```typescript
 * const one = new One<number>(42);
 * console.log(one.Get()); // 42
 *
 * const moved = one.Pick(); // Transfer ownership
 * console.log(one.IsEmpty()); // true
 * console.log(moved.Get()); // 42
 * ```
 */
export class One<T> {
  private value: T | null;

  /**
   * Create One with optional initial value
   * @param value Initial value (optional)
   */
  constructor(value?: T) {
    this.value = value !== undefined ? value : null;
  }

  /**
   * Create One with value
   * @param value Value to own
   * @returns New One instance
   */
  static Create<T>(value: T): One<T> {
    return new One(value);
  }

  /**
   * Check if One is empty (no value)
   * @returns true if empty
   */
  IsEmpty(): boolean {
    return this.value === null;
  }

  /**
   * Check if One has a value
   * @returns true if has value
   */
  HasValue(): boolean {
    return this.value !== null;
  }

  /**
   * Get the value
   * @returns Value
   * @throws Error if empty
   */
  Get(): T {
    if (this.value === null) {
      throw new Error('One<T> is empty');
    }
    return this.value;
  }

  /**
   * Get the value or default
   * @param defaultValue Default value if empty
   * @returns Value or default
   */
  GetOr(defaultValue: T): T {
    return this.value !== null ? this.value : defaultValue;
  }

  /**
   * Get the value or null
   * @returns Value or null
   */
  GetOrNull(): T | null {
    return this.value;
  }

  /**
   * Set the value (replaces existing)
   * @param value New value
   */
  Set(value: T): void {
    this.value = value;
  }

  /**
   * Clear the value (set to empty)
   */
  Clear(): void {
    this.value = null;
  }

  /**
   * Detach the value (move out, leaving empty)
   * This is the "Pick" operation in U++
   * @returns The detached value
   * @throws Error if empty
   */
  Detach(): T {
    if (this.value === null) {
      throw new Error('One<T> is empty, cannot detach');
    }
    const val = this.value;
    this.value = null;
    return val;
  }

  /**
   * Pick the value (U++ naming, same as Detach)
   * Transfers ownership out of this One
   * @returns The picked value
   */
  Pick(): T {
    return this.Detach();
  }

  /**
   * Attach a value (move in)
   * @param value Value to attach
   */
  Attach(value: T): void {
    this.value = value;
  }

  /**
   * Swap values with another One
   * @param other Other One instance
   */
  Swap(other: One<T>): void {
    const temp = this.value;
    this.value = other.value;
    other.value = temp;
  }

  /**
   * Create a new One by picking from this one (transfer)
   * Leaves this One empty
   * @returns New One with transferred value
   */
  PickToOne(): One<T> {
    const val = this.Detach();
    return new One(val);
  }

  /**
   * Map the value to a new type
   * @param fn Mapping function
   * @returns New One with mapped value
   */
  Map<U>(fn: (value: T) => U): One<U> {
    if (this.value === null) {
      return new One<U>();
    }
    return new One(fn(this.value));
  }

  /**
   * FlatMap the value (chain operations)
   * @param fn Function that returns One<U>
   * @returns Result of fn or empty One
   */
  FlatMap<U>(fn: (value: T) => One<U>): One<U> {
    if (this.value === null) {
      return new One<U>();
    }
    return fn(this.value);
  }

  /**
   * Execute function if value exists
   * @param fn Function to execute with value
   */
  IfPresent(fn: (value: T) => void): void {
    if (this.value !== null) {
      fn(this.value);
    }
  }

  /**
   * Filter value based on predicate
   * @param predicate Test function
   * @returns This One if predicate true, empty One otherwise
   */
  Filter(predicate: (value: T) => boolean): One<T> {
    if (this.value !== null && predicate(this.value)) {
      return this;
    }
    return new One<T>();
  }

  /**
   * String representation
   */
  toString(): string {
    if (this.value === null) {
      return 'One(empty)';
    }
    return `One(${this.value})`;
  }

  /**
   * Compare with another One
   * @param other Other One
   * @returns true if both empty or both have equal values
   */
  Equals(other: One<T>): boolean {
    if (this.value === null && other.value === null) {
      return true;
    }
    if (this.value === null || other.value === null) {
      return false;
    }
    return this.value === other.value;
  }

  /**
   * Clone the value (creates new One with copy)
   * Only works for primitive types or objects that support cloning
   * @returns New One with cloned value
   */
  Clone(): One<T> {
    if (this.value === null) {
      return new One<T>();
    }

    // For primitive types, just copy
    if (typeof this.value !== 'object') {
      return new One(this.value);
    }

    // For objects, try to clone
    // Check if object has a Clone method
    if ('Clone' in this.value && typeof (this.value as any).Clone === 'function') {
      return new One((this.value as any).Clone());
    }

    // Try structured clone for plain objects
    try {
      return new One(structuredClone(this.value));
    } catch {
      throw new Error('One<T>: Cannot clone value, no Clone method and structuredClone failed');
    }
  }
}

/**
 * Pick helper function (U++ style)
 * Extracts value from One, leaving it empty
 *
 * @param one One to pick from
 * @returns Picked value
 *
 * @example
 * ```typescript
 * const one = new One(42);
 * const value = Pick(one);
 * console.log(one.IsEmpty()); // true
 * ```
 */
export function Pick<T>(one: One<T>): T {
  return one.Pick();
}
