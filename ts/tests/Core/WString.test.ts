import { WString } from '../../src/Core/WString';

describe('WString', () => {
  describe('constructor', () => {
    it('creates empty string', () => {
      const wstr = new WString();
      expect(wstr.GetCount()).toBe(0);
      expect(wstr.IsEmpty()).toBe(true);
    });

    it('creates string from value', () => {
      const wstr = new WString('hello');
      expect(wstr.GetCount()).toBe(5);
      expect(wstr.ToString()).toBe('hello');
    });

    it('handles Unicode strings', () => {
      const wstr = new WString('Hello 世界 🌍');
      expect(wstr.ToString()).toBe('Hello 世界 🌍');
    });
  });

  describe('Unicode support', () => {
    it('gets code point at index', () => {
      const wstr = new WString('A🌍B');
      expect(wstr.GetCodePoint(0)).toBe(65); // 'A'
      expect(wstr.GetCodePoint(1)).toBe(127757); // '🌍' emoji
    });

    it('gets code point length', () => {
      const wstr = new WString('A🌍B');
      expect(wstr.GetCount()).toBe(4); // UTF-16 code units (emoji is 2)
      expect(wstr.GetCodePointLength()).toBe(3); // Unicode code points
    });

    it('converts to code points array', () => {
      const wstr = new WString('A🌍');
      const codePoints = wstr.ToCodePoints();
      expect(codePoints).toEqual([65, 127757]);
    });

    it('creates from code points', () => {
      const wstr = WString.FromCodePoints([65, 127757, 66]);
      expect(wstr.ToString()).toBe('A🌍B');
    });
  });

  describe('At and Get', () => {
    it('gets character at index', () => {
      const wstr = new WString('hello');
      expect(wstr.At(0)).toBe('h');
      expect(wstr.At(1)).toBe('e');
      expect(wstr.At(4)).toBe('o');
    });

    it('throws on out of bounds access', () => {
      const wstr = new WString('hello');
      expect(() => wstr.At(-1)).toThrow('WString index out of bounds');
      expect(() => wstr.At(5)).toThrow('WString index out of bounds');
    });

    it('Get returns empty string for out of bounds', () => {
      const wstr = new WString('hello');
      expect(wstr.Get(-1)).toBe('');
      expect(wstr.Get(5)).toBe('');
      expect(wstr.Get(0)).toBe('h');
    });
  });

  describe('GetCount and GetLength', () => {
    it('returns correct length', () => {
      expect(new WString('').GetCount()).toBe(0);
      expect(new WString('hello').GetCount()).toBe(5);
      expect(new WString('hello').GetLength()).toBe(5);
    });

    it('handles Unicode length correctly', () => {
      const wstr = new WString('🌍🌎🌏');
      expect(wstr.GetCount()).toBe(6); // UTF-16 units (each emoji is 2)
      expect(wstr.GetCodePointLength()).toBe(3); // Code points
    });
  });

  describe('IsEmpty and Clear', () => {
    it('checks if empty', () => {
      expect(new WString('').IsEmpty()).toBe(true);
      expect(new WString('hello').IsEmpty()).toBe(false);
    });

    it('clears string', () => {
      const wstr = new WString('hello');
      const cleared = wstr.Clear();
      expect(cleared.IsEmpty()).toBe(true);
      expect(wstr.ToString()).toBe('hello'); // Original unchanged
    });
  });

  describe('Case conversion', () => {
    it('converts to uppercase (locale-aware)', () => {
      const wstr = new WString('Hello Wörld');
      expect(wstr.ToUpper().ToString()).toBe('HELLO WÖRLD');
    });

    it('converts to lowercase (locale-aware)', () => {
      const wstr = new WString('Hello Wörld');
      expect(wstr.ToLower().ToString()).toBe('hello wörld');
    });

    it('handles Unicode case conversion', () => {
      const wstr = new WString('Straße'); // German
      const upper = wstr.ToUpper();
      expect(upper.ToString()).toContain('SS'); // ß -> SS in uppercase
    });
  });

  describe('Substring operations', () => {
    const wstr = new WString('Hello World');

    it('extracts middle substring', () => {
      expect(wstr.Mid(0, 5).ToString()).toBe('Hello');
      expect(wstr.Mid(6, 5).ToString()).toBe('World');
      expect(wstr.Mid(6).ToString()).toBe('World');
    });

    it('extracts left substring', () => {
      expect(wstr.Left(5).ToString()).toBe('Hello');
      expect(wstr.Left(0).ToString()).toBe('');
    });

    it('extracts right substring', () => {
      expect(wstr.Right(5).ToString()).toBe('World');
      expect(wstr.Right(0).ToString()).toBe('');
    });

    it('handles Unicode substrings', () => {
      const unicode = new WString('Hello 世界');
      expect(unicode.Mid(6, 2).ToString()).toBe('世界');
    });
  });

  describe('Find operations', () => {
    const wstr = new WString('Hello World, Hello Universe');

    it('finds substring', () => {
      expect(wstr.Find('Hello')).toBe(0);
      expect(wstr.Find('World')).toBe(6);
      expect(wstr.Find('Hello', 1)).toBe(13);
      expect(wstr.Find('missing')).toBe(-1);
    });

    it('finds character', () => {
      expect(wstr.FindChar('H')).toBe(0);
      expect(wstr.FindChar('W')).toBe(6);
      expect(wstr.FindChar('o', 5)).toBe(7);
      expect(wstr.FindChar('x')).toBe(-1);
    });

    it('finds substring from end', () => {
      expect(wstr.ReverseFind('Hello')).toBe(13);
      expect(wstr.ReverseFind('World')).toBe(6);
      expect(wstr.ReverseFind('missing')).toBe(-1);
    });

    it('finds Unicode characters', () => {
      const unicode = new WString('Hello 世界 World');
      expect(unicode.Find('世')).toBe(6);
      expect(unicode.Find('界')).toBe(7);
    });
  });

  describe('StartsWith and EndsWith', () => {
    const wstr = new WString('Hello World');

    it('checks prefix', () => {
      expect(wstr.StartsWith('Hello')).toBe(true);
      expect(wstr.StartsWith('World')).toBe(false);
      expect(wstr.StartsWith('')).toBe(true);
    });

    it('checks suffix', () => {
      expect(wstr.EndsWith('World')).toBe(true);
      expect(wstr.EndsWith('Hello')).toBe(false);
      expect(wstr.EndsWith('')).toBe(true);
    });
  });

  describe('Modification operations', () => {
    it('inserts substring', () => {
      const wstr = new WString('Hello World');
      expect(wstr.Insert(5, ' Beautiful').ToString()).toBe('Hello Beautiful World');
      expect(wstr.Insert(0, 'Say: ').ToString()).toBe('Say: Hello World');
    });

    it('removes characters', () => {
      const wstr = new WString('Hello World');
      expect(wstr.Remove(5, 6).ToString()).toBe('Hello');
      expect(wstr.Remove(0, 6).ToString()).toBe('World');
      expect(wstr.Remove(5).ToString()).toBe('HelloWorld');
    });

    it('replaces first occurrence', () => {
      const wstr = new WString('Hello World, Hello Universe');
      expect(wstr.Replace('Hello', 'Hi').ToString()).toBe('Hi World, Hello Universe');
    });

    it('replaces all occurrences', () => {
      const wstr = new WString('Hello World, Hello Universe');
      expect(wstr.ReplaceAll('Hello', 'Hi').ToString()).toBe('Hi World, Hi Universe');
    });
  });

  describe('Trim operations', () => {
    it('trims whitespace from both ends', () => {
      expect(new WString('  hello  ').Trim().ToString()).toBe('hello');
      expect(new WString('\t\nhello\n\t').Trim().ToString()).toBe('hello');
    });

    it('trims whitespace from start', () => {
      expect(new WString('  hello  ').TrimStart().ToString()).toBe('hello  ');
    });

    it('trims whitespace from end', () => {
      expect(new WString('  hello  ').TrimEnd().ToString()).toBe('  hello');
    });

    it('handles Unicode whitespace', () => {
      const wstr = new WString('\u00A0hello\u00A0'); // Non-breaking space
      expect(wstr.Trim().ToString()).toBe('hello');
    });
  });

  describe('Compare operations', () => {
    it('compares strings case-sensitive', () => {
      const a = new WString('apple');
      const b = new WString('banana');
      const c = new WString('apple');

      expect(a.Compare(b)).toBeLessThan(0);
      expect(b.Compare(a)).toBeGreaterThan(0);
      expect(a.Compare(c)).toBe(0);
      expect(a.Compare('apple')).toBe(0);
    });

    it('compares strings case-insensitive', () => {
      const a = new WString('Apple');
      const b = new WString('BANANA');

      expect(a.CompareNoCase('apple')).toBe(0);
      expect(a.CompareNoCase('APPLE')).toBe(0);
      expect(a.CompareNoCase(b)).toBeLessThan(0);
      expect(b.CompareNoCase(a)).toBeGreaterThan(0);
    });

    it('handles locale-aware comparison', () => {
      const a = new WString('ä');
      const b = new WString('z');
      // Locale-aware comparison may differ from simple character code comparison
      expect(a.Compare(b)).toBeDefined();
    });
  });

  describe('Conversion operations', () => {
    it('converts to native string', () => {
      const wstr = new WString('hello');
      expect(wstr.ToString()).toBe('hello');
      expect(wstr.ToStd()).toBe('hello');
    });

    it('converts to UTF-8', () => {
      const wstr = new WString('hello');
      const utf8 = wstr.ToUtf8();
      expect(utf8).toBeInstanceOf(Uint8Array);
      expect(utf8.length).toBe(5);
      expect(utf8[0]).toBe(104); // 'h'
    });

    it('converts to UTF-16 code units', () => {
      const wstr = new WString('hello');
      const utf16 = wstr.ToUtf16();
      expect(utf16).toEqual([104, 101, 108, 108, 111]);
    });

    it('handles Unicode UTF-8 conversion', () => {
      const wstr = new WString('世界');
      const utf8 = wstr.ToUtf8();
      expect(utf8.length).toBeGreaterThan(2); // Multi-byte UTF-8
    });
  });

  describe('Unicode normalization', () => {
    it('normalizes to NFC', () => {
      // Combining characters: e + combining acute
      const decomposed = new WString('e\u0301'); // é as e + ´
      const normalized = decomposed.Normalize('NFC');
      expect(normalized.ToString()).toBe('\u00E9'); // é as single character
    });

    it('normalizes to NFD', () => {
      const composed = new WString('\u00E9'); // é as single character
      const normalized = composed.Normalize('NFD');
      expect(normalized.GetCount()).toBe(2); // e + combining acute
    });

    it('defaults to NFC normalization', () => {
      const wstr = new WString('e\u0301');
      const normalized = wstr.Normalize();
      expect(normalized.ToString()).toBe('\u00E9');
    });
  });

  describe('Concatenation and repetition', () => {
    it('concatenates strings', () => {
      const a = new WString('Hello');
      const b = new WString(' World');
      expect(a.Cat(b).ToString()).toBe('Hello World');
      expect(a.Cat(' there').ToString()).toBe('Hello there');
    });

    it('repeats string', () => {
      const wstr = new WString('ab');
      expect(wstr.Repeat(3).ToString()).toBe('ababab');
      expect(wstr.Repeat(0).ToString()).toBe('');
      expect(wstr.Repeat(1).ToString()).toBe('ab');
    });

    it('handles Unicode concatenation', () => {
      const a = new WString('Hello');
      const b = new WString(' 世界');
      expect(a.Cat(b).ToString()).toBe('Hello 世界');
    });
  });

  describe('Split operation', () => {
    it('splits by separator', () => {
      const wstr = new WString('apple,banana,cherry');
      const parts = wstr.Split(',');
      expect(parts.length).toBe(3);
      expect(parts[0].ToString()).toBe('apple');
      expect(parts[1].ToString()).toBe('banana');
      expect(parts[2].ToString()).toBe('cherry');
    });

    it('splits Unicode strings', () => {
      const wstr = new WString('Hello,世界,World');
      const parts = wstr.Split(',');
      expect(parts.length).toBe(3);
      expect(parts[1].ToString()).toBe('世界');
    });
  });

  describe('Static From methods', () => {
    it('creates from native string', () => {
      const wstr = WString.From('hello');
      expect(wstr.ToString()).toBe('hello');
    });

    it('creates from UTF-8', () => {
      const utf8 = new Uint8Array([104, 101, 108, 108, 111]); // "hello"
      const wstr = WString.FromUtf8(utf8);
      expect(wstr.ToString()).toBe('hello');
    });

    it('creates from UTF-16 code units', () => {
      const units = [104, 101, 108, 108, 111]; // "hello"
      const wstr = WString.FromUtf16(units);
      expect(wstr.ToString()).toBe('hello');
    });

    it('creates from code points', () => {
      const codePoints = [72, 101, 108, 108, 111]; // "Hello"
      const wstr = WString.FromCodePoints(codePoints);
      expect(wstr.ToString()).toBe('Hello');
    });

    it('creates from Unicode code points', () => {
      const codePoints = [65, 127757]; // "A🌍"
      const wstr = WString.FromCodePoints(codePoints);
      expect(wstr.ToString()).toBe('A🌍');
    });
  });

  describe('Static Join', () => {
    it('joins array of strings', () => {
      const parts = ['apple', 'banana', 'cherry'];
      expect(WString.Join(', ', parts).ToString()).toBe('apple, banana, cherry');
    });

    it('joins array of WString objects', () => {
      const parts = [new WString('apple'), new WString('banana'), new WString('cherry')];
      expect(WString.Join(', ', parts).ToString()).toBe('apple, banana, cherry');
    });

    it('joins mixed array', () => {
      const parts = ['apple', new WString('banana'), 'cherry'];
      expect(WString.Join('-', parts).ToString()).toBe('apple-banana-cherry');
    });
  });

  describe('Iterator', () => {
    it('iterates over code points', () => {
      const wstr = new WString('A🌍B');
      const chars: string[] = [];
      for (const c of wstr) {
        chars.push(c);
      }
      expect(chars).toEqual(['A', '🌍', 'B']);
    });

    it('handles regular ASCII iteration', () => {
      const wstr = new WString('hello');
      const chars: string[] = [];
      for (const c of wstr) {
        chars.push(c);
      }
      expect(chars).toEqual(['h', 'e', 'l', 'l', 'o']);
    });
  });

  describe('toString and valueOf', () => {
    it('returns native string from toString', () => {
      const wstr = new WString('hello');
      expect(wstr.toString()).toBe('hello');
    });

    it('returns native string from valueOf', () => {
      const wstr = new WString('hello');
      expect(wstr.valueOf()).toBe('hello');
    });

    it('allows implicit string conversion', () => {
      const wstr = new WString('hello');
      expect('' + wstr).toBe('hello');
    });
  });

  describe('Immutability', () => {
    it('operations return new WString objects', () => {
      const original = new WString('hello');
      const upper = original.ToUpper();
      const trimmed = new WString('  hello  ').Trim();

      expect(original.ToString()).toBe('hello');
      expect(upper.ToString()).toBe('HELLO'); // ToUpper returns uppercase
      expect(trimmed.ToString()).toBe('hello');
    });
  });

  describe('Edge cases', () => {
    it('handles empty strings', () => {
      const empty = new WString('');
      expect(empty.GetCount()).toBe(0);
      expect(empty.IsEmpty()).toBe(true);
      expect(empty.Find('x')).toBe(-1);
    });

    it('handles special characters', () => {
      const wstr = new WString('Hello\nWorld\tTest');
      expect(wstr.GetCount()).toBe(16);
      expect(wstr.Find('\n')).toBe(5);
      expect(wstr.Find('\t')).toBe(11);
    });

    it('handles complex emoji', () => {
      const wstr = new WString('👨‍👩‍👧‍👦'); // Family emoji (composed of multiple code points)
      expect(wstr.GetCount()).toBeGreaterThan(0);
      expect(wstr.ToString()).toBe('👨‍👩‍👧‍👦');
    });

    it('handles right-to-left text', () => {
      const wstr = new WString('مرحبا'); // Arabic "hello"
      expect(wstr.GetCount()).toBeGreaterThan(0);
      expect(wstr.ToString()).toBe('مرحبا');
    });

    it('handles CJK characters', () => {
      const wstr = new WString('你好世界'); // Chinese "hello world"
      expect(wstr.GetCount()).toBe(4);
      expect(wstr.ToString()).toBe('你好世界');
    });
  });

  describe('Performance', () => {
    it('handles large Unicode strings efficiently', () => {
      const large = '世'.repeat(1000);
      const wstr = new WString(large);

      expect(wstr.GetCount()).toBe(1000);
      expect(wstr.Find('世')).toBe(0);
    });
  });
});
