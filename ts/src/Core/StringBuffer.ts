/**
 * StringBuffer - Efficient string building with U++ interface
 *
 * Provides efficient string concatenation using array-based approach.
 * U++ StringBuffer is designed for building strings incrementally
 * without creating many intermediate string objects.
 *
 * In TypeScript, we use an array to accumulate parts and join them
 * only when the final string is needed.
 *
 * Key features:
 * - Efficient append operations (O(1) amortized)
 * - U++ method naming (Cat, Clear, GetCount, etc.)
 * - Conversion to String or native string
 * - Supports various data types
 */

import { String } from './String';

export class StringBuffer {
  private parts: string[];
  private cachedString: string | null;

  /**
   * Create new StringBuffer
   * @param initialValue Initial string (optional)
   */
  constructor(initialValue?: string) {
    this.parts = initialValue !== undefined ? [initialValue] : [];
    this.cachedString = null;
  }

  /**
   * Append string to buffer
   * @param str String to append
   * @returns This buffer (for chaining)
   */
  Cat(str: string | String): StringBuffer {
    const s = str instanceof String ? str.ToString() : str;
    this.parts.push(s);
    this.cachedString = null;
    return this;
  }

  /**
   * Append character to buffer
   * @param c Character to append
   * @returns This buffer (for chaining)
   */
  CatChar(c: string): StringBuffer {
    this.parts.push(c);
    this.cachedString = null;
    return this;
  }

  /**
   * Append number to buffer
   * @param n Number to append
   * @returns This buffer (for chaining)
   */
  CatInt(n: number): StringBuffer {
    this.parts.push(globalThis.String(Math.floor(n)));
    this.cachedString = null;
    return this;
  }

  /**
   * Append double/float to buffer
   * @param n Number to append
   * @param precision Decimal places (optional)
   * @returns This buffer (for chaining)
   */
  CatDouble(n: number, precision?: number): StringBuffer {
    if (precision !== undefined) {
      this.parts.push(n.toFixed(precision));
    } else {
      this.parts.push(globalThis.String(n));
    }
    this.cachedString = null;
    return this;
  }

  /**
   * Clear buffer
   */
  Clear(): void {
    this.parts = [];
    this.cachedString = null;
  }

  /**
   * Get estimated length (may not be exact until ToString called)
   * @returns Approximate length
   */
  GetLength(): number {
    if (this.cachedString !== null) {
      return this.cachedString.length;
    }
    // Estimate based on parts
    return this.parts.reduce((sum, part) => sum + part.length, 0);
  }

  /**
   * Get number of parts in buffer
   * @returns Part count
   */
  GetPartCount(): number {
    return this.parts.length;
  }

  /**
   * Check if buffer is empty
   * @returns true if empty
   */
  IsEmpty(): boolean {
    return this.parts.length === 0;
  }

  /**
   * Convert to native string
   * @returns Concatenated string
   */
  ToString(): string {
    if (this.cachedString === null) {
      this.cachedString = this.parts.join('');
    }
    return this.cachedString;
  }

  /**
   * Convert to String object
   * @returns String object with concatenated value
   */
  ToStringObject(): String {
    return new String(this.ToString());
  }

  /**
   * Set buffer content (replaces existing)
   * @param str New content
   */
  Set(str: string | String): void {
    const s = str instanceof String ? str.ToString() : str;
    this.parts = [s];
    this.cachedString = null;
  }

  /**
   * Reserve space (no-op in JavaScript, for U++ compatibility)
   * @param size Size to reserve
   */
  Reserve(size: number): void {
    // No-op: JavaScript arrays grow dynamically
    // Included for U++ API compatibility
  }

  /**
   * Insert string at position
   * Note: This is less efficient than appending
   * @param pos Position to insert (in characters, not parts)
   * @param str String to insert
   */
  Insert(pos: number, str: string | String): void {
    const current = this.ToString();
    const s = str instanceof String ? str.ToString() : str;
    const before = current.substring(0, pos);
    const after = current.substring(pos);
    this.parts = [before + s + after];
    this.cachedString = null;
  }

  /**
   * Remove characters from buffer
   * @param pos Start position
   * @param count Number of characters to remove
   */
  Remove(pos: number, count: number): void {
    const current = this.ToString();
    const before = current.substring(0, pos);
    const after = current.substring(pos + count);
    this.parts = [before + after];
    this.cachedString = null;
  }

  /**
   * Get character at position
   * @param i Index
   * @returns Character at index
   */
  At(i: number): string {
    const str = this.ToString();
    if (i < 0 || i >= str.length) {
      throw new Error(`StringBuffer index out of bounds: ${i}`);
    }
    return str[i];
  }

  /**
   * Get character at position (operator[] equivalent)
   * @param i Index
   * @returns Character at index, or empty string if out of bounds
   */
  Get(i: number): string {
    const str = this.ToString();
    return str[i] || '';
  }

  /**
   * Append formatted string
   * @param format Format string with %s, %d, %f placeholders
   * @param args Arguments to format
   * @returns This buffer (for chaining)
   */
  CatFormat(format: string, ...args: any[]): StringBuffer {
    let result = format;
    let argIndex = 0;

    result = result.replace(/%([sdf])/g, (match, type) => {
      if (argIndex >= args.length) {
        return match;
      }

      const arg = args[argIndex++];
      switch (type) {
        case 's':
          return globalThis.String(arg);
        case 'd':
          return globalThis.String(Math.floor(Number(arg)));
        case 'f':
          return globalThis.String(Number(arg));
        default:
          return match;
      }
    });

    this.parts.push(result);
    this.cachedString = null;
    return this;
  }

  /**
   * Append line (string + newline)
   * @param str String to append (optional)
   * @returns This buffer (for chaining)
   */
  CatLine(str?: string | String): StringBuffer {
    if (str !== undefined) {
      const s = str instanceof String ? str.ToString() : str;
      this.parts.push(s);
    }
    this.parts.push('\n');
    this.cachedString = null;
    return this;
  }

  /**
   * String representation
   */
  toString(): string {
    return this.ToString();
  }

  /**
   * Value of (for implicit conversions)
   */
  valueOf(): string {
    return this.ToString();
  }

  /**
   * Create StringBuffer from array of strings
   * @param parts Array of strings
   * @returns New StringBuffer
   */
  static From(parts: (string | String)[]): StringBuffer {
    const sb = new StringBuffer();
    for (const part of parts) {
      sb.Cat(part);
    }
    return sb;
  }

  /**
   * Join strings with separator
   * @param separator Separator string
   * @param parts Array of strings
   * @returns StringBuffer with joined strings
   */
  static Join(separator: string, parts: (string | String)[]): StringBuffer {
    const sb = new StringBuffer();
    for (let i = 0; i < parts.length; i++) {
      if (i > 0) {
        sb.Cat(separator);
      }
      sb.Cat(parts[i]);
    }
    return sb;
  }
}
