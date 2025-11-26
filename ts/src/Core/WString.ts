/**
 * WString - Wide String for Unicode support
 *
 * In U++, WString handles wide characters (UTF-16/32).
 * In TypeScript, all strings are already Unicode-aware (UTF-16),
 * so WString is essentially an alias/wrapper emphasizing Unicode handling.
 *
 * Key features:
 * - Full Unicode support (UTF-16 internally like JavaScript)
 * - Code point aware operations
 * - U++ method naming conventions
 * - Interoperability with String class
 */

export class WString {
  private data: string;

  /**
   * Create new WString
   * @param value Initial value (default: empty string)
   */
  constructor(value: string = '') {
    this.data = value;
  }

  /**
   * Get code point at index (Unicode-aware)
   * @param i Index
   * @returns Code point value
   * @throws Error if index out of bounds
   */
  GetCodePoint(i: number): number {
    const codePoint = this.data.codePointAt(i);
    if (codePoint === undefined) {
      throw new Error(`WString index out of bounds: ${i}`);
    }
    return codePoint;
  }

  /**
   * Get character at index (may be surrogate pair)
   * @param i Index
   * @returns Character at index
   * @throws Error if index out of bounds
   */
  At(i: number): string {
    if (i < 0 || i >= this.data.length) {
      throw new Error(`WString index out of bounds: ${i}`);
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
   * Get length (UTF-16 code units)
   * @returns Length in code units
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
   * Get length in Unicode code points
   * @returns Length in code points
   */
  GetCodePointLength(): number {
    return Array.from(this.data).length;
  }

  /**
   * Check if string is empty
   * @returns true if empty
   */
  IsEmpty(): boolean {
    return this.data.length === 0;
  }

  /**
   * Clear string (return empty WString)
   * @returns New empty WString
   */
  Clear(): WString {
    return new WString('');
  }

  /**
   * Convert to uppercase (Unicode-aware)
   * @returns New uppercase WString
   */
  ToUpper(): WString {
    return new WString(this.data.toLocaleUpperCase());
  }

  /**
   * Convert to lowercase (Unicode-aware)
   * @returns New lowercase WString
   */
  ToLower(): WString {
    return new WString(this.data.toLocaleLowerCase());
  }

  /**
   * Get substring starting at position
   * @param pos Start position
   * @param count Number of characters (default: to end)
   * @returns New WString with substring
   */
  Mid(pos: number, count?: number): WString {
    if (count === undefined) {
      return new WString(this.data.substring(pos));
    }
    return new WString(this.data.substr(pos, count));
  }

  /**
   * Get leftmost characters
   * @param count Number of characters
   * @returns New WString with leftmost characters
   */
  Left(count: number): WString {
    return new WString(this.data.substring(0, count));
  }

  /**
   * Get rightmost characters
   * @param count Number of characters
   * @returns New WString with rightmost characters
   */
  Right(count: number): WString {
    return new WString(this.data.substring(this.data.length - count));
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
   * @returns New WString with insertion
   */
  Insert(pos: number, str: string): WString {
    const before = this.data.substring(0, pos);
    const after = this.data.substring(pos);
    return new WString(before + str + after);
  }

  /**
   * Remove characters
   * @param pos Start position
   * @param count Number of characters to remove (default: 1)
   * @returns New WString with characters removed
   */
  Remove(pos: number, count: number = 1): WString {
    const before = this.data.substring(0, pos);
    const after = this.data.substring(pos + count);
    return new WString(before + after);
  }

  /**
   * Replace first occurrence
   * @param oldStr String to replace
   * @param newStr Replacement string
   * @returns New WString with replacement
   */
  Replace(oldStr: string, newStr: string): WString {
    return new WString(this.data.replace(oldStr, newStr));
  }

  /**
   * Replace all occurrences
   * @param oldStr String to replace
   * @param newStr Replacement string
   * @returns New WString with all replacements
   */
  ReplaceAll(oldStr: string, newStr: string): WString {
    return new WString(this.data.split(oldStr).join(newStr));
  }

  /**
   * Trim whitespace from both ends
   * @returns New trimmed WString
   */
  Trim(): WString {
    return new WString(this.data.trim());
  }

  /**
   * Trim whitespace from start
   * @returns New trimmed WString
   */
  TrimStart(): WString {
    return new WString(this.data.trimStart());
  }

  /**
   * Trim whitespace from end
   * @returns New trimmed WString
   */
  TrimEnd(): WString {
    return new WString(this.data.trimEnd());
  }

  /**
   * Compare strings (case-sensitive, locale-aware)
   * @param other String to compare
   * @returns 0 if equal, <0 if this < other, >0 if this > other
   */
  Compare(other: string | WString): number {
    const otherStr = other instanceof WString ? other.ToString() : other;
    return this.data.localeCompare(otherStr);
  }

  /**
   * Compare strings (case-insensitive, locale-aware)
   * @param other String to compare
   * @returns 0 if equal, <0 if this < other, >0 if this > other
   */
  CompareNoCase(other: string | WString): number {
    const otherStr = other instanceof WString ? other.ToString() : other;
    return this.data.localeCompare(otherStr, undefined, { sensitivity: 'base' });
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
   * Convert to UTF-8 byte array
   * @returns Uint8Array with UTF-8 encoded bytes
   */
  ToUtf8(): Uint8Array {
    const encoder = new TextEncoder();
    return encoder.encode(this.data);
  }

  /**
   * Convert to UTF-16 code units array
   * @returns Array of UTF-16 code units
   */
  ToUtf16(): number[] {
    const units: number[] = [];
    for (let i = 0; i < this.data.length; i++) {
      units.push(this.data.charCodeAt(i));
    }
    return units;
  }

  /**
   * Convert to code points array (full Unicode)
   * @returns Array of Unicode code points
   */
  ToCodePoints(): number[] {
    return Array.from(this.data).map(char => char.codePointAt(0)!);
  }

  /**
   * Concatenate strings
   * @param other String to append
   * @returns New concatenated WString
   */
  Cat(other: string | WString): WString {
    const otherStr = other instanceof WString ? other.ToString() : other;
    return new WString(this.data + otherStr);
  }

  /**
   * Repeat string n times
   * @param count Number of repetitions
   * @returns New repeated WString
   */
  Repeat(count: number): WString {
    return new WString(this.data.repeat(count));
  }

  /**
   * Normalize Unicode string (NFC, NFD, NFKC, NFKD)
   * @param form Normalization form (default: 'NFC')
   * @returns Normalized WString
   */
  Normalize(form: 'NFC' | 'NFD' | 'NFKC' | 'NFKD' = 'NFC'): WString {
    return new WString(this.data.normalize(form));
  }

  /**
   * Split string by separator
   * @param separator Separator string
   * @returns Array of WString parts
   */
  Split(separator: string): WString[] {
    return this.data.split(separator).map(s => new WString(s));
  }

  /**
   * Create WString from native string
   * @param str Native string
   * @returns New WString
   */
  static From(str: string): WString {
    return new WString(str);
  }

  /**
   * Create WString from UTF-8 byte array
   * @param bytes UTF-8 encoded byte array
   * @returns New WString
   */
  static FromUtf8(bytes: Uint8Array): WString {
    const decoder = new TextDecoder('utf-8');
    return new WString(decoder.decode(bytes));
  }

  /**
   * Create WString from UTF-16 code units
   * @param units Array of UTF-16 code units
   * @returns New WString
   */
  static FromUtf16(units: number[]): WString {
    return new WString(globalThis.String.fromCharCode(...units));
  }

  /**
   * Create WString from code points
   * @param codePoints Array of Unicode code points
   * @returns New WString
   */
  static FromCodePoints(codePoints: number[]): WString {
    return new WString(globalThis.String.fromCodePoint(...codePoints));
  }

  /**
   * Join array of wide strings
   * @param separator Separator string
   * @param strings Array of strings to join
   * @returns Joined WString
   */
  static Join(separator: string, strings: (string | WString)[]): WString {
    const parts = strings.map(s => s instanceof WString ? s.ToString() : s);
    return new WString(parts.join(separator));
  }

  /**
   * Iterator support for for-of loops (iterates over code points)
   */
  [Symbol.iterator](): Iterator<string> {
    return Array.from(this.data)[Symbol.iterator]();
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
