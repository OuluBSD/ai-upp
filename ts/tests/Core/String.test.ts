import { String } from '../../src/Core/String';

describe('String', () => {
  describe('constructor', () => {
    it('creates empty string', () => {
      const str = new String();
      expect(str.GetCount()).toBe(0);
      expect(str.IsEmpty()).toBe(true);
    });

    it('creates string from value', () => {
      const str = new String('hello');
      expect(str.GetCount()).toBe(5);
      expect(str.ToString()).toBe('hello');
    });
  });

  describe('At and Get', () => {
    it('gets character at index', () => {
      const str = new String('hello');
      expect(str.At(0)).toBe('h');
      expect(str.At(1)).toBe('e');
      expect(str.At(4)).toBe('o');
    });

    it('throws on out of bounds access', () => {
      const str = new String('hello');
      expect(() => str.At(-1)).toThrow('String index out of bounds');
      expect(() => str.At(5)).toThrow('String index out of bounds');
    });

    it('Get returns empty string for out of bounds', () => {
      const str = new String('hello');
      expect(str.Get(-1)).toBe('');
      expect(str.Get(5)).toBe('');
      expect(str.Get(0)).toBe('h');
    });
  });

  describe('GetCount and GetLength', () => {
    it('returns correct length', () => {
      expect(new String('').GetCount()).toBe(0);
      expect(new String('hello').GetCount()).toBe(5);
      expect(new String('hello').GetLength()).toBe(5);
    });
  });

  describe('IsEmpty and Clear', () => {
    it('checks if empty', () => {
      expect(new String('').IsEmpty()).toBe(true);
      expect(new String('hello').IsEmpty()).toBe(false);
    });

    it('clears string', () => {
      const str = new String('hello');
      const cleared = str.Clear();
      expect(cleared.IsEmpty()).toBe(true);
      expect(str.ToString()).toBe('hello'); // Original unchanged
    });
  });

  describe('Case conversion', () => {
    it('converts to uppercase', () => {
      const str = new String('Hello World');
      expect(str.ToUpper().ToString()).toBe('HELLO WORLD');
    });

    it('converts to lowercase', () => {
      const str = new String('Hello World');
      expect(str.ToLower().ToString()).toBe('hello world');
    });

    it('converts to ASCII upper', () => {
      const str = new String('hello');
      expect(str.ToAsciiUpper().ToString()).toBe('HELLO');
    });

    it('converts to ASCII lower', () => {
      const str = new String('HELLO');
      expect(str.ToAsciiLower().ToString()).toBe('hello');
    });
  });

  describe('Substring operations', () => {
    const str = new String('Hello World');

    it('extracts middle substring', () => {
      expect(str.Mid(0, 5).ToString()).toBe('Hello');
      expect(str.Mid(6, 5).ToString()).toBe('World');
      expect(str.Mid(6).ToString()).toBe('World');
    });

    it('extracts left substring', () => {
      expect(str.Left(5).ToString()).toBe('Hello');
      expect(str.Left(0).ToString()).toBe('');
    });

    it('extracts right substring', () => {
      expect(str.Right(5).ToString()).toBe('World');
      expect(str.Right(0).ToString()).toBe('');
    });
  });

  describe('Find operations', () => {
    const str = new String('Hello World, Hello Universe');

    it('finds substring', () => {
      expect(str.Find('Hello')).toBe(0);
      expect(str.Find('World')).toBe(6);
      expect(str.Find('Hello', 1)).toBe(13);
      expect(str.Find('missing')).toBe(-1);
    });

    it('finds character', () => {
      expect(str.FindChar('H')).toBe(0);
      expect(str.FindChar('W')).toBe(6);
      expect(str.FindChar('o', 5)).toBe(7);
      expect(str.FindChar('x')).toBe(-1);
    });

    it('finds substring from end', () => {
      expect(str.ReverseFind('Hello')).toBe(13);
      expect(str.ReverseFind('World')).toBe(6);
      expect(str.ReverseFind('missing')).toBe(-1);
    });

    it('finds character from end', () => {
      expect(str.ReverseFindChar('H')).toBe(13);
      expect(str.ReverseFindChar('e')).toBe(26); // Last 'e' in "Universe"
      expect(str.ReverseFindChar('x')).toBe(-1);
    });

    it('finds first of character set', () => {
      expect(str.FindFirstOf('aeiou')).toBe(1); // 'e' in Hello
      expect(str.FindFirstOf('xyz')).toBe(-1);
      expect(str.FindFirstOf('W')).toBe(6);
    });

    it('finds last of character set', () => {
      const s = new String('Hello');
      expect(s.FindLastOf('aeiou')).toBe(4); // 'o'
      expect(s.FindLastOf('xyz')).toBe(-1);
    });
  });

  describe('StartsWith and EndsWith', () => {
    const str = new String('Hello World');

    it('checks prefix', () => {
      expect(str.StartsWith('Hello')).toBe(true);
      expect(str.StartsWith('World')).toBe(false);
      expect(str.StartsWith('')).toBe(true);
    });

    it('checks suffix', () => {
      expect(str.EndsWith('World')).toBe(true);
      expect(str.EndsWith('Hello')).toBe(false);
      expect(str.EndsWith('')).toBe(true);
    });
  });

  describe('Modification operations', () => {
    it('inserts substring', () => {
      const str = new String('Hello World');
      expect(str.Insert(5, ' Beautiful').ToString()).toBe('Hello Beautiful World');
      expect(str.Insert(0, 'Say: ').ToString()).toBe('Say: Hello World');
    });

    it('removes characters', () => {
      const str = new String('Hello World');
      expect(str.Remove(5, 6).ToString()).toBe('Hello');
      expect(str.Remove(0, 6).ToString()).toBe('World');
      expect(str.Remove(5).ToString()).toBe('HelloWorld');
    });

    it('replaces first occurrence', () => {
      const str = new String('Hello World, Hello Universe');
      expect(str.Replace('Hello', 'Hi').ToString()).toBe('Hi World, Hello Universe');
    });

    it('replaces all occurrences', () => {
      const str = new String('Hello World, Hello Universe');
      expect(str.ReplaceAll('Hello', 'Hi').ToString()).toBe('Hi World, Hi Universe');
      expect(str.ReplaceAll('l', 'L').ToString()).toBe('HeLLo WorLd, HeLLo Universe');
    });
  });

  describe('Trim operations', () => {
    it('trims whitespace from both ends', () => {
      expect(new String('  hello  ').Trim().ToString()).toBe('hello');
      expect(new String('\t\nhello\n\t').Trim().ToString()).toBe('hello');
    });

    it('trims whitespace from start', () => {
      expect(new String('  hello  ').TrimStart().ToString()).toBe('hello  ');
    });

    it('trims whitespace from end', () => {
      expect(new String('  hello  ').TrimEnd().ToString()).toBe('  hello');
    });

    it('handles strings with no whitespace', () => {
      expect(new String('hello').Trim().ToString()).toBe('hello');
    });
  });

  describe('Compare operations', () => {
    it('compares strings case-sensitive', () => {
      const a = new String('apple');
      const b = new String('banana');
      const c = new String('apple');

      expect(a.Compare(b)).toBeLessThan(0);
      expect(b.Compare(a)).toBeGreaterThan(0);
      expect(a.Compare(c)).toBe(0);
      expect(a.Compare('apple')).toBe(0);
    });

    it('compares strings case-insensitive', () => {
      const a = new String('Apple');
      const b = new String('BANANA');

      expect(a.CompareNoCase('apple')).toBe(0);
      expect(a.CompareNoCase('APPLE')).toBe(0);
      expect(a.CompareNoCase(b)).toBeLessThan(0);
      expect(b.CompareNoCase(a)).toBeGreaterThan(0);
    });
  });

  describe('Conversion operations', () => {
    it('converts to native string', () => {
      const str = new String('hello');
      expect(str.ToString()).toBe('hello');
      expect(str.ToStd()).toBe('hello');
    });

    it('converts to integer', () => {
      expect(new String('123').ToInt()).toBe(123);
      expect(new String('-456').ToInt()).toBe(-456);
      expect(new String('12.34').ToInt()).toBe(12);
      expect(new String('invalid').ToInt()).toBe(0);
      expect(new String('invalid').ToInt(99)).toBe(99);
    });

    it('converts to double', () => {
      expect(new String('123.45').ToDouble()).toBe(123.45);
      expect(new String('-67.89').ToDouble()).toBe(-67.89);
      expect(new String('invalid').ToDouble()).toBe(0.0);
      expect(new String('invalid').ToDouble(99.9)).toBe(99.9);
    });

    it('checks if string is number', () => {
      expect(new String('123').IsNumber()).toBe(true);
      expect(new String('123.45').IsNumber()).toBe(true);
      expect(new String('-67.89').IsNumber()).toBe(true);
      expect(new String('invalid').IsNumber()).toBe(false);
      expect(new String('').IsNumber()).toBe(false);
    });
  });

  describe('Concatenation and repetition', () => {
    it('concatenates strings', () => {
      const a = new String('Hello');
      const b = new String(' World');
      expect(a.Cat(b).ToString()).toBe('Hello World');
      expect(a.Cat(' there').ToString()).toBe('Hello there');
    });

    it('repeats string', () => {
      const str = new String('ab');
      expect(str.Repeat(3).ToString()).toBe('ababab');
      expect(str.Repeat(0).ToString()).toBe('');
      expect(str.Repeat(1).ToString()).toBe('ab');
    });
  });

  describe('Split operation', () => {
    it('splits by separator', () => {
      const str = new String('apple,banana,cherry');
      const parts = str.Split(',');
      expect(parts.length).toBe(3);
      expect(parts[0].ToString()).toBe('apple');
      expect(parts[1].ToString()).toBe('banana');
      expect(parts[2].ToString()).toBe('cherry');
    });

    it('splits empty string', () => {
      const str = new String('');
      const parts = str.Split(',');
      expect(parts.length).toBe(1);
      expect(parts[0].ToString()).toBe('');
    });
  });

  describe('Static From methods', () => {
    it('creates from native string', () => {
      const str = String.From('hello');
      expect(str.ToString()).toBe('hello');
    });

    it('creates from integer', () => {
      expect(String.FromInt(123).ToString()).toBe('123');
      expect(String.FromInt(-456).ToString()).toBe('-456');
      expect(String.FromInt(0).ToString()).toBe('0');
    });

    it('creates from double', () => {
      expect(String.FromDouble(123.45).ToString()).toBe('123.45');
      expect(String.FromDouble(-67.89).ToString()).toBe('-67.89');
      expect(String.FromDouble(123.45, 1).ToString()).toBe('123.5');
      expect(String.FromDouble(123.456, 2).ToString()).toBe('123.46');
    });
  });

  describe('Static Format', () => {
    it('formats strings', () => {
      expect(String.Format('Hello %s', 'World').ToString()).toBe('Hello World');
      expect(String.Format('%s %s', 'Hello', 'World').ToString()).toBe('Hello World');
    });

    it('formats integers', () => {
      expect(String.Format('Count: %d', 42).ToString()).toBe('Count: 42');
      expect(String.Format('%d + %d = %d', 2, 3, 5).ToString()).toBe('2 + 3 = 5');
      expect(String.Format('Number: %d', 3.14).ToString()).toBe('Number: 3');
    });

    it('formats floats', () => {
      expect(String.Format('Pi: %f', 3.14159).ToString()).toBe('Pi: 3.14159');
      expect(String.Format('%f + %f', 1.5, 2.5).ToString()).toBe('1.5 + 2.5');
    });

    it('handles mixed formats', () => {
      const result = String.Format('Name: %s, Age: %d, Score: %f', 'Alice', 30, 95.5);
      expect(result.ToString()).toBe('Name: Alice, Age: 30, Score: 95.5');
    });

    it('handles insufficient arguments', () => {
      expect(String.Format('%s %s', 'Hello').ToString()).toBe('Hello %s');
    });
  });

  describe('Static Join', () => {
    it('joins array of strings', () => {
      const parts = ['apple', 'banana', 'cherry'];
      expect(String.Join(', ', parts).ToString()).toBe('apple, banana, cherry');
    });

    it('joins array of String objects', () => {
      const parts = [new String('apple'), new String('banana'), new String('cherry')];
      expect(String.Join(', ', parts).ToString()).toBe('apple, banana, cherry');
    });

    it('joins mixed array', () => {
      const parts = ['apple', new String('banana'), 'cherry'];
      expect(String.Join('-', parts).ToString()).toBe('apple-banana-cherry');
    });

    it('joins empty array', () => {
      expect(String.Join(', ', []).ToString()).toBe('');
    });
  });

  describe('Iterator', () => {
    it('iterates over characters', () => {
      const str = new String('hello');
      const chars: string[] = [];
      for (const c of str) {
        chars.push(c);
      }
      expect(chars).toEqual(['h', 'e', 'l', 'l', 'o']);
    });
  });

  describe('toString and valueOf', () => {
    it('returns native string from toString', () => {
      const str = new String('hello');
      expect(str.toString()).toBe('hello');
    });

    it('returns native string from valueOf', () => {
      const str = new String('hello');
      expect(str.valueOf()).toBe('hello');
    });

    it('allows implicit string conversion', () => {
      const str = new String('hello');
      expect('' + str).toBe('hello');
    });
  });

  describe('Immutability', () => {
    it('operations return new String objects', () => {
      const original = new String('hello');
      const upper = original.ToUpper();
      const trimmed = new String('  hello  ').Trim();

      expect(original.ToString()).toBe('hello');
      expect(upper.ToString()).toBe('HELLO');
      expect(trimmed.ToString()).toBe('hello');
    });
  });

  describe('Edge cases', () => {
    it('handles empty strings', () => {
      const empty = new String('');
      expect(empty.GetCount()).toBe(0);
      expect(empty.IsEmpty()).toBe(true);
      expect(empty.Find('x')).toBe(-1);
      expect(empty.Split(',')).toHaveLength(1);
    });

    it('handles special characters', () => {
      const str = new String('Hello\nWorld\tTest');
      expect(str.GetCount()).toBe(16);
      expect(str.Find('\n')).toBe(5);
      expect(str.Find('\t')).toBe(11);
    });

    it('handles unicode characters', () => {
      const str = new String('Hello 世界 🌍');
      expect(str.GetCount()).toBeGreaterThan(0);
      expect(str.ToString()).toBe('Hello 世界 🌍');
    });
  });

  describe('Performance', () => {
    it('handles large strings efficiently', () => {
      const large = 'a'.repeat(10000);
      const str = new String(large);

      expect(str.GetCount()).toBe(10000);
      expect(str.Find('a')).toBe(0);
      expect(str.FindChar('a', 9999)).toBe(9999);
    });

    it('handles many operations efficiently', () => {
      let str = new String('test');

      for (let i = 0; i < 100; i++) {
        str = str.Cat(' ').Cat(String.FromInt(i));
      }

      expect(str.GetCount()).toBeGreaterThan(100);
    });
  });
});
