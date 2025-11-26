/**
 * String - U++ String class with TypeScript implementation
 *
 * Provides familiar U++ string API wrapping JavaScript native strings.
 * Immutable by default like JavaScript strings, but provides U++ method names.
 *
 * Key features:
 * - U++ method naming (GetCount, ToUpper, Find, etc.)
 * - String manipulation and search operations
 * - Conversion utilities
 * - Format support
 * - Efficient implementation using native JavaScript strings
 */

export class String {
  private data: string;

  /**
   * Create new String
   * @param value Initial value (default: empty string)
   */
  constructor(value: string = '') {
    this.data = value;
  }

  /**
   * Get character at index
   * @param i Index
   * @returns Character at index
   * @throws Error if index out of bounds
   */
  At(i: number): string {
    if (i < 0 || i >= this.data.length) {
      throw new Error(`String index out of bounds: ${i}`);
    }
    return this.data[i];
  }

  /**
   * Get character at index (operator[] equivalent)
   * @param i Index
   * @returns Character at index
   */
  Get(i: number): string {
    return this.data[i] || '';
  }

  /**
   * Get length of string
   * @returns Character count
   */
  GetCount(): number {
    return this.data.length;
  }

  /**
   * Get length (alias for GetCount)
   */
  GetLength(): number {
    return this.data.length;
  }

  /**
   * Check if string is empty
   * @returns true if empty
   */
  IsEmpty(): boolean {
    return this.data.length === 0;
  }

  /**
   * Clear string (return empty string)
   * @returns New empty String
   */
  Clear(): String {
    return new String('');
  }

  /**
   * Convert to uppercase
   * @returns New uppercase String
   */
  ToUpper(): String {
    return new String(this.data.toUpperCase());
  }

  /**
   * Convert to lowercase
   * @returns New lowercase String
   */
  ToLower(): String {
    return new String(this.data.toLowerCase());
  }

  /**
   * Convert to ASCII uppercase (locale-independent)
   * @returns New uppercase String
   */
  ToAsciiUpper(): String {
    return new String(this.data.toUpperCase());
  }

  /**
   * Convert to ASCII lowercase (locale-independent)
   * @returns New lowercase String
   */
  ToAsciiLower(): String {
    return new String(this.data.toLowerCase());
  }

  /**
   * Get substring starting at position
   * @param pos Start position
   * @param count Number of characters (default: to end)
   * @returns New String with substring
   */
  Mid(pos: number, count?: number): String {
    if (count === undefined) {
      return new String(this.data.substring(pos));
    }
    return new String(this.data.substr(pos, count));
  }

  /**
   * Get leftmost characters
   * @param count Number of characters
   * @returns New String with leftmost characters
   */
  Left(count: number): String {
    return new String(this.data.substring(0, count));
  }

  /**
   * Get rightmost characters
   * @param count Number of characters
   * @returns New String with rightmost characters
   */
  Right(count: number): String {
    return new String(this.data.substring(this.data.length - count));
  }

  /**
   * Find substring
   * @param substr Substring to find
   * @param from Start position (default: 0)
   * @returns Index of first occurrence, or -1 if not found
   */
  Find(substr: string, from: number = 0): number {
    return this.data.indexOf(substr, from);
  }

  /**
   * Find character
   * @param c Character to find
   * @param from Start position (default: 0)
   * @returns Index of first occurrence, or -1 if not found
   */
  FindChar(c: string, from: number = 0): number {
    return this.data.indexOf(c, from);
  }

  /**
   * Find substring from end
   * @param substr Substring to find
   * @returns Index of last occurrence, or -1 if not found
   */
  ReverseFind(substr: string): number {
    return this.data.lastIndexOf(substr);
  }

  /**
   * Find character from end
   * @param c Character to find
   * @returns Index of last occurrence, or -1 if not found
   */
  ReverseFindChar(c: string): number {
    return this.data.lastIndexOf(c);
  }

  /**
   * Find first occurrence of any character in set
   * @param chars Character set
   * @param from Start position (default: 0)
   * @returns Index of first match, or -1 if not found
   */
  FindFirstOf(chars: string, from: number = 0): number {
    for (let i = from; i < this.data.length; i++) {
      if (chars.includes(this.data[i])) {
        return i;
      }
    }
    return -1;
  }

  /**
   * Find last occurrence of any character in set
   * @param chars Character set
   * @returns Index of last match, or -1 if not found
   */
  FindLastOf(chars: string): number {
    for (let i = this.data.length - 1; i >= 0; i--) {
      if (chars.includes(this.data[i])) {
        return i;
      }
    }
    return -1;
  }

  /**
   * Check if string starts with prefix
   * @param prefix Prefix to check
   * @returns true if starts with prefix
   */
  StartsWith(prefix: string): boolean {
    return this.data.startsWith(prefix);
  }

  /**
   * Check if string ends with suffix
   * @param suffix Suffix to check
   * @returns true if ends with suffix
   */
  EndsWith(suffix: string): boolean {
    return this.data.endsWith(suffix);
  }

  /**
   * Insert substring at position
   * @param pos Position to insert
   * @param str String to insert
   * @returns New String with insertion
   */
  Insert(pos: number, str: string): String {
    const before = this.data.substring(0, pos);
    const after = this.data.substring(pos);
    return new String(before + str + after);
  }

  /**
   * Remove characters
   * @param pos Start position
   * @param count Number of characters to remove (default: 1)
   * @returns New String with characters removed
   */
  Remove(pos: number, count: number = 1): String {
    const before = this.data.substring(0, pos);
    const after = this.data.substring(pos + count);
    return new String(before + after);
  }

  /**
   * Replace first occurrence
   * @param oldStr String to replace
   * @param newStr Replacement string
   * @returns New String with replacement
   */
  Replace(oldStr: string, newStr: string): String {
    return new String(this.data.replace(oldStr, newStr));
  }

  /**
   * Replace all occurrences
   * @param oldStr String to replace
   * @param newStr Replacement string
   * @returns New String with all replacements
   */
  ReplaceAll(oldStr: string, newStr: string): String {
    return new String(this.data.split(oldStr).join(newStr));
  }

  /**
   * Trim whitespace from both ends
   * @returns New trimmed String
   */
  Trim(): String {
    return new String(this.data.trim());
  }

  /**
   * Trim whitespace from start
   * @returns New trimmed String
   */
  TrimStart(): String {
    return new String(this.data.trimStart());
  }

  /**
   * Trim whitespace from end
   * @returns New trimmed String
   */
  TrimEnd(): String {
    return new String(this.data.trimEnd());
  }

  /**
   * Compare strings (case-sensitive)
   * @param other String to compare
   * @returns 0 if equal, <0 if this < other, >0 if this > other
   */
  Compare(other: string | String): number {
    const otherStr = other instanceof String ? other.ToString() : other;
    if (this.data === otherStr) return 0;
    return this.data < otherStr ? -1 : 1;
  }

  /**
   * Compare strings (case-insensitive)
   * @param other String to compare
   * @returns 0 if equal, <0 if this < other, >0 if this > other
   */
  CompareNoCase(other: string | String): number {
    const thisLower = this.data.toLowerCase();
    const otherStr = other instanceof String ? other.ToString() : other;
    const otherLower = otherStr.toLowerCase();
    if (thisLower === otherLower) return 0;
    return thisLower < otherLower ? -1 : 1;
  }

  /**
   * Convert to native JavaScript string
   * @returns Native string
   */
  ToString(): string {
    return this.data;
  }

  /**
   * Get native string (alias for ToString)
   */
  ToStd(): string {
    return this.data;
  }

  /**
   * Convert to integer
   * @param defaultValue Default value if parse fails (default: 0)
   * @returns Parsed integer
   */
  ToInt(defaultValue: number = 0): number {
    const parsed = parseInt(this.data, 10);
    return isNaN(parsed) ? defaultValue : parsed;
  }

  /**
   * Convert to double/float
   * @param defaultValue Default value if parse fails (default: 0.0)
   * @returns Parsed number
   */
  ToDouble(defaultValue: number = 0.0): number {
    const parsed = parseFloat(this.data);
    return isNaN(parsed) ? defaultValue : parsed;
  }

  /**
   * Check if string is a number
   * @returns true if string represents a valid number
   */
  IsNumber(): boolean {
    return !isNaN(parseFloat(this.data)) && isFinite(parseFloat(this.data));
  }

  /**
   * Concatenate strings
   * @param other String to append
   * @returns New concatenated String
   */
  Cat(other: string | String): String {
    const otherStr = other instanceof String ? other.ToString() : other;
    return new String(this.data + otherStr);
  }

  /**
   * Repeat string n times
   * @param count Number of repetitions
   * @returns New repeated String
   */
  Repeat(count: number): String {
    return new String(this.data.repeat(count));
  }

  /**
   * Split string by separator
   * @param separator Separator string
   * @returns Array of String parts
   */
  Split(separator: string): String[] {
    return this.data.split(separator).map(s => new String(s));
  }

  /**
   * Create String from native string
   * @param str Native string
   * @returns New String
   */
  static From(str: string): String {
    return new String(str);
  }

  /**
   * Create String from number
   * @param n Number to convert
   * @returns String representation
   */
  static FromInt(n: number): String {
    return new String(n.toString());
  }

  /**
   * Create String from double
   * @param n Number to convert
   * @param precision Decimal places (optional)
   * @returns String representation
   */
  static FromDouble(n: number, precision?: number): String {
    if (precision !== undefined) {
      return new String(n.toFixed(precision));
    }
    return new String(n.toString());
  }

  /**
   * Format string (simple sprintf-like)
   * @param format Format string with %s, %d, %f placeholders
   * @param args Arguments to format
   * @returns Formatted String
   */
  static Format(format: string, ...args: any[]): String {
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

    return new String(result);
  }

  /**
   * Join array of strings
   * @param separator Separator string
   * @param strings Array of strings to join
   * @returns Joined String
   */
  static Join(separator: string, strings: (string | String)[]): String {
    const parts = strings.map(s => s instanceof String ? s.ToString() : s);
    return new String(parts.join(separator));
  }

  /**
   * Iterator support for for-of loops (iterates over characters)
   */
  [Symbol.iterator](): Iterator<string> {
    return this.data[Symbol.iterator]();
  }

  /**
   * String representation
   */
  toString(): string {
    return this.data;
  }

  /**
   * Value of (for implicit conversions)
   */
  valueOf(): string {
    return this.data;
  }
}
