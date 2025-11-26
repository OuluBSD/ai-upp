/**
 * Ptr<T> - Shared reference smart pointer
 *
 * Provides shared ownership semantics similar to references in JavaScript.
 * In U++, Ptr<T> is a nullable reference that can share ownership of objects.
 *
 * In TypeScript/JavaScript, references are already shared by default,
 * but Ptr<T> provides:
 * - Explicit null-safety
 * - U++ compatible API
 * - Weak reference support
 * - Reference counting semantics
 *
 * Key features:
 * - Shared ownership (multiple Ptrs can reference same object)
 * - Null-safe access
 * - Weak reference support
 * - Clear U++ API
 */

/**
 * Reference counter for shared ownership tracking
 * Used internally by Ptr to implement reference counting
 */
class RefCount<T> {
  public refCount: number = 1;
  public weakCount: number = 0;

  constructor(public value: T | null) {}
}

/**
 * Ptr<T> - Shared pointer with reference counting
 *
 * Holds a reference to a value that can be shared among multiple Ptrs.
 * Uses reference counting to track shared ownership.
 *
 * @template T Type of the referenced value
 *
 * @example
 * ```typescript
 * const ptr1 = new Ptr({ x: 10, y: 20 });
 * const ptr2 = ptr1; // Share reference
 *
 * console.log(ptr1.Get().x); // 10
 * console.log(ptr2.Get().x); // 10
 *
 * ptr1.Get().x = 30;
 * console.log(ptr2.Get().x); // 30 (shared)
 * ```
 */
export class Ptr<T> {
  private ref: RefCount<T> | null;

  /**
   * Create Ptr with optional initial value
   * @param value Initial value (optional)
   */
  constructor(value?: T) {
    if (value !== undefined) {
      this.ref = new RefCount(value);
    } else {
      this.ref = null;
    }
  }

  /**
   * Create Ptr with value
   * @param value Value to reference
   * @returns New Ptr instance
   */
  static Create<T>(value: T): Ptr<T> {
    return new Ptr(value);
  }

  /**
   * Create null Ptr
   * @returns Null Ptr
   */
  static Null<T>(): Ptr<T> {
    return new Ptr<T>();
  }

  /**
   * Check if Ptr is null (no value)
   * @returns true if null
   */
  IsNull(): boolean {
    return this.ref === null || this.ref.value === null;
  }

  /**
   * Check if Ptr has a value
   * @returns true if has value
   */
  HasValue(): boolean {
    return this.ref !== null && this.ref.value !== null;
  }

  /**
   * Get the referenced value
   * @returns Value
   * @throws Error if null
   */
  Get(): T {
    if (this.ref === null || this.ref.value === null) {
      throw new Error('Ptr<T> is null');
    }
    return this.ref.value;
  }

  /**
   * Get the value or default
   * @param defaultValue Default value if null
   * @returns Value or default
   */
  GetOr(defaultValue: T): T {
    if (this.ref !== null && this.ref.value !== null) {
      return this.ref.value;
    }
    return defaultValue;
  }

  /**
   * Get the value or null
   * @returns Value or null
   */
  GetOrNull(): T | null {
    return this.ref !== null ? this.ref.value : null;
  }

  /**
   * Set the value (creates new reference)
   * @param value New value
   */
  Set(value: T): void {
    this.Clear();
    this.ref = new RefCount(value);
  }

  /**
   * Clear the reference (set to null)
   */
  Clear(): void {
    if (this.ref !== null) {
      this.ref.refCount--;
      if (this.ref.refCount === 0 && this.ref.weakCount === 0) {
        this.ref.value = null;
      }
    }
    this.ref = null;
  }

  /**
   * Attach to existing reference (share ownership)
   * @param other Other Ptr to share with
   */
  Attach(other: Ptr<T>): void {
    if (other.ref === this.ref) {
      return; // Already attached
    }

    this.Clear();

    if (other.ref !== null) {
      this.ref = other.ref;
      this.ref.refCount++;
    }
  }

  /**
   * Detach from current reference
   * @returns The detached value
   */
  Detach(): T {
    const val = this.Get();
    this.Clear();
    return val;
  }

  /**
   * Get reference count
   * @returns Number of Ptrs sharing this value
   */
  GetRefCount(): number {
    return this.ref !== null ? this.ref.refCount : 0;
  }

  /**
   * Get weak reference count
   * @returns Number of WeakPtrs referencing this value
   */
  GetWeakCount(): number {
    return this.ref !== null ? this.ref.weakCount : 0;
  }

  /**
   * Create a copy that shares the reference
   * @returns New Ptr sharing the same reference
   */
  Share(): Ptr<T> {
    const newPtr = new Ptr<T>();
    newPtr.Attach(this);
    return newPtr;
  }

  /**
   * Compare with another Ptr (reference equality)
   * @param other Other Ptr
   * @returns true if referencing same object
   */
  Equals(other: Ptr<T>): boolean {
    return this.ref === other.ref;
  }

  /**
   * Compare values (value equality)
   * @param other Other Ptr
   * @returns true if values are equal
   */
  ValueEquals(other: Ptr<T>): boolean {
    if (this.IsNull() && other.IsNull()) {
      return true;
    }
    if (this.IsNull() || other.IsNull()) {
      return false;
    }
    return this.Get() === other.Get();
  }

  /**
   * Operator -> equivalent (access members)
   * Just returns Get() for member access
   * @returns Value
   */
  Arrow(): T {
    return this.Get();
  }

  /**
   * String representation
   */
  toString(): string {
    if (this.IsNull()) {
      return 'Ptr(null)';
    }
    const refCount = this.GetRefCount();
    return `Ptr(${this.Get()}, refs: ${refCount})`;
  }

  /**
   * Create weak reference to this Ptr
   * @returns New WeakPtr
   */
  ToWeak(): WeakPtr<T> {
    return new WeakPtr(this);
  }
}

/**
 * WeakPtr<T> - Weak reference that doesn't affect reference counting
 *
 * Holds a weak reference to a value. The value may be deleted
 * even if WeakPtr exists. Must be upgraded to Ptr to access.
 *
 * @template T Type of the referenced value
 *
 * @example
 * ```typescript
 * const ptr = new Ptr({ x: 10 });
 * const weak = ptr.ToWeak();
 *
 * const locked = weak.Lock();
 * if (locked.HasValue()) {
 *   console.log(locked.Get().x); // 10
 * }
 * ```
 */
export class WeakPtr<T> {
  private ref: RefCount<T> | null;

  /**
   * Create WeakPtr from Ptr
   * @param ptr Ptr to reference weakly
   */
  constructor(ptr?: Ptr<T>) {
    if (ptr && ptr['ref']) {
      this.ref = ptr['ref'];
      this.ref.weakCount++;
    } else {
      this.ref = null;
    }
  }

  /**
   * Check if weak reference is expired
   * @returns true if referenced object is gone
   */
  IsExpired(): boolean {
    return this.ref === null || this.ref.value === null || this.ref.refCount === 0;
  }

  /**
   * Lock the weak reference (upgrade to Ptr)
   * @returns Ptr if still valid, null Ptr if expired
   */
  Lock(): Ptr<T> {
    if (this.IsExpired()) {
      return new Ptr<T>();
    }

    const ptr = new Ptr<T>();
    ptr['ref'] = this.ref;
    if (this.ref) {
      this.ref.refCount++;
    }
    return ptr;
  }

  /**
   * Clear the weak reference
   */
  Clear(): void {
    if (this.ref !== null) {
      this.ref.weakCount--;
      if (this.ref.refCount === 0 && this.ref.weakCount === 0) {
        this.ref.value = null;
      }
    }
    this.ref = null;
  }

  /**
   * Get use count (strong references)
   * @returns Number of strong references
   */
  UseCount(): number {
    return this.ref !== null ? this.ref.refCount : 0;
  }

  /**
   * String representation
   */
  toString(): string {
    if (this.IsExpired()) {
      return 'WeakPtr(expired)';
    }
    return `WeakPtr(refs: ${this.UseCount()})`;
  }
}
