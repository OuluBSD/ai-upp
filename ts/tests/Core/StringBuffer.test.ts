import { StringBuffer } from '../../src/Core/StringBuffer';
import { String } from '../../src/Core/String';

describe('StringBuffer', () => {
  describe('constructor', () => {
    it('creates empty buffer', () => {
      const sb = new StringBuffer();
      expect(sb.IsEmpty()).toBe(true);
      expect(sb.GetLength()).toBe(0);
      expect(sb.ToString()).toBe('');
    });

    it('creates buffer with initial value', () => {
      const sb = new StringBuffer('hello');
      expect(sb.IsEmpty()).toBe(false);
      expect(sb.GetLength()).toBe(5);
      expect(sb.ToString()).toBe('hello');
    });
  });

  describe('Cat operations', () => {
    it('concatenates strings', () => {
      const sb = new StringBuffer();
      sb.Cat('hello');
      sb.Cat(' ');
      sb.Cat('world');
      expect(sb.ToString()).toBe('hello world');
    });

    it('concatenates String objects', () => {
      const sb = new StringBuffer();
      sb.Cat(new String('hello'));
      sb.Cat(new String(' '));
      sb.Cat(new String('world'));
      expect(sb.ToString()).toBe('hello world');
    });

    it('supports method chaining', () => {
      const sb = new StringBuffer();
      sb.Cat('hello').Cat(' ').Cat('world');
      expect(sb.ToString()).toBe('hello world');
    });

    it('concatenates empty strings', () => {
      const sb = new StringBuffer();
      sb.Cat('').Cat('hello').Cat('');
      expect(sb.ToString()).toBe('hello');
    });
  });

  describe('CatChar', () => {
    it('appends single characters', () => {
      const sb = new StringBuffer();
      sb.CatChar('a').CatChar('b').CatChar('c');
      expect(sb.ToString()).toBe('abc');
    });

    it('handles special characters', () => {
      const sb = new StringBuffer();
      sb.CatChar('\n').CatChar('\t');
      expect(sb.ToString()).toBe('\n\t');
    });
  });

  describe('CatInt', () => {
    it('appends integers', () => {
      const sb = new StringBuffer();
      sb.CatInt(123);
      expect(sb.ToString()).toBe('123');
    });

    it('appends negative integers', () => {
      const sb = new StringBuffer();
      sb.CatInt(-456);
      expect(sb.ToString()).toBe('-456');
    });

    it('floors floating point numbers', () => {
      const sb = new StringBuffer();
      sb.CatInt(12.99);
      expect(sb.ToString()).toBe('12');
    });

    it('chains integer concatenation', () => {
      const sb = new StringBuffer();
      sb.Cat('Count: ').CatInt(42);
      expect(sb.ToString()).toBe('Count: 42');
    });
  });

  describe('CatDouble', () => {
    it('appends floating point numbers', () => {
      const sb = new StringBuffer();
      sb.CatDouble(3.14159);
      expect(sb.ToString()).toBe('3.14159');
    });

    it('appends with precision', () => {
      const sb = new StringBuffer();
      sb.CatDouble(3.14159, 2);
      expect(sb.ToString()).toBe('3.14');
    });

    it('handles negative numbers', () => {
      const sb = new StringBuffer();
      sb.CatDouble(-67.89);
      expect(sb.ToString()).toBe('-67.89');
    });

    it('chains double concatenation', () => {
      const sb = new StringBuffer();
      sb.Cat('Pi: ').CatDouble(3.14159, 3);
      expect(sb.ToString()).toBe('Pi: 3.142');
    });
  });

  describe('Clear', () => {
    it('clears buffer content', () => {
      const sb = new StringBuffer();
      sb.Cat('hello').Cat(' ').Cat('world');
      expect(sb.GetLength()).toBeGreaterThan(0);

      sb.Clear();
      expect(sb.IsEmpty()).toBe(true);
      expect(sb.GetLength()).toBe(0);
      expect(sb.ToString()).toBe('');
    });

    it('allows reuse after clear', () => {
      const sb = new StringBuffer();
      sb.Cat('first');
      sb.Clear();
      sb.Cat('second');
      expect(sb.ToString()).toBe('second');
    });
  });

  describe('GetLength and GetPartCount', () => {
    it('returns correct length', () => {
      const sb = new StringBuffer();
      expect(sb.GetLength()).toBe(0);

      sb.Cat('hello');
      expect(sb.GetLength()).toBe(5);

      sb.Cat(' world');
      expect(sb.GetLength()).toBe(11);
    });

    it('returns correct part count', () => {
      const sb = new StringBuffer();
      expect(sb.GetPartCount()).toBe(0);

      sb.Cat('hello');
      expect(sb.GetPartCount()).toBe(1);

      sb.Cat(' ').Cat('world');
      expect(sb.GetPartCount()).toBe(3);
    });

    it('caches string result', () => {
      const sb = new StringBuffer();
      sb.Cat('hello').Cat(' ').Cat('world');

      // First call builds the string
      const str1 = sb.ToString();
      // Subsequent calls should return cached result
      const str2 = sb.ToString();
      expect(str1).toBe(str2);
      expect(sb.GetLength()).toBe(11);
    });
  });

  describe('IsEmpty', () => {
    it('returns true for empty buffer', () => {
      const sb = new StringBuffer();
      expect(sb.IsEmpty()).toBe(true);
    });

    it('returns false for non-empty buffer', () => {
      const sb = new StringBuffer('hello');
      expect(sb.IsEmpty()).toBe(false);
    });

    it('returns true after clear', () => {
      const sb = new StringBuffer('hello');
      sb.Clear();
      expect(sb.IsEmpty()).toBe(true);
    });
  });

  describe('Set', () => {
    it('sets buffer content', () => {
      const sb = new StringBuffer();
      sb.Cat('old');
      sb.Set('new');
      expect(sb.ToString()).toBe('new');
    });

    it('sets content from String object', () => {
      const sb = new StringBuffer();
      sb.Set(new String('hello'));
      expect(sb.ToString()).toBe('hello');
    });

    it('replaces existing content', () => {
      const sb = new StringBuffer('first');
      sb.Set('second');
      expect(sb.ToString()).toBe('second');
      expect(sb.GetPartCount()).toBe(1);
    });
  });

  describe('ToStringObject', () => {
    it('converts to String object', () => {
      const sb = new StringBuffer();
      sb.Cat('hello');
      const str = sb.ToStringObject();
      expect(str).toBeInstanceOf(String);
      expect(str.ToString()).toBe('hello');
    });
  });

  describe('Insert', () => {
    it('inserts at beginning', () => {
      const sb = new StringBuffer('world');
      sb.Insert(0, 'hello ');
      expect(sb.ToString()).toBe('hello world');
    });

    it('inserts in middle', () => {
      const sb = new StringBuffer('hello world');
      sb.Insert(5, ' beautiful');
      expect(sb.ToString()).toBe('hello beautiful world');
    });

    it('inserts at end', () => {
      const sb = new StringBuffer('hello');
      sb.Insert(5, ' world');
      expect(sb.ToString()).toBe('hello world');
    });

    it('inserts String object', () => {
      const sb = new StringBuffer('hello world');
      sb.Insert(6, new String('beautiful '));
      expect(sb.ToString()).toBe('hello beautiful world');
    });
  });

  describe('Remove', () => {
    it('removes from beginning', () => {
      const sb = new StringBuffer('hello world');
      sb.Remove(0, 6);
      expect(sb.ToString()).toBe('world');
    });

    it('removes from middle', () => {
      const sb = new StringBuffer('hello beautiful world');
      sb.Remove(5, 10); // Remove " beautiful"
      expect(sb.ToString()).toBe('hello world');
    });

    it('removes from end', () => {
      const sb = new StringBuffer('hello world');
      sb.Remove(5, 6);
      expect(sb.ToString()).toBe('hello');
    });

    it('handles remove with large count', () => {
      const sb = new StringBuffer('hello');
      sb.Remove(2, 100); // Remove from position 2 to end
      expect(sb.ToString()).toBe('he');
    });
  });

  describe('At and Get', () => {
    it('gets character at index', () => {
      const sb = new StringBuffer('hello');
      expect(sb.At(0)).toBe('h');
      expect(sb.At(1)).toBe('e');
      expect(sb.At(4)).toBe('o');
    });

    it('throws on out of bounds access', () => {
      const sb = new StringBuffer('hello');
      expect(() => sb.At(-1)).toThrow('StringBuffer index out of bounds');
      expect(() => sb.At(5)).toThrow('StringBuffer index out of bounds');
    });

    it('Get returns empty string for out of bounds', () => {
      const sb = new StringBuffer('hello');
      expect(sb.Get(-1)).toBe('');
      expect(sb.Get(5)).toBe('');
      expect(sb.Get(0)).toBe('h');
    });
  });

  describe('CatFormat', () => {
    it('formats strings', () => {
      const sb = new StringBuffer();
      sb.CatFormat('Hello %s', 'World');
      expect(sb.ToString()).toBe('Hello World');
    });

    it('formats integers', () => {
      const sb = new StringBuffer();
      sb.CatFormat('Count: %d', 42);
      expect(sb.ToString()).toBe('Count: 42');
    });

    it('formats floats', () => {
      const sb = new StringBuffer();
      sb.CatFormat('Pi: %f', 3.14159);
      expect(sb.ToString()).toBe('Pi: 3.14159');
    });

    it('formats mixed types', () => {
      const sb = new StringBuffer();
      sb.CatFormat('Name: %s, Age: %d, Score: %f', 'Alice', 30, 95.5);
      expect(sb.ToString()).toBe('Name: Alice, Age: 30, Score: 95.5');
    });

    it('chains format calls', () => {
      const sb = new StringBuffer();
      sb.CatFormat('Hello %s', 'World').CatFormat(' - Count: %d', 42);
      expect(sb.ToString()).toBe('Hello World - Count: 42');
    });

    it('handles insufficient arguments', () => {
      const sb = new StringBuffer();
      sb.CatFormat('%s %s', 'Hello');
      expect(sb.ToString()).toBe('Hello %s');
    });
  });

  describe('CatLine', () => {
    it('appends line with string', () => {
      const sb = new StringBuffer();
      sb.CatLine('hello');
      expect(sb.ToString()).toBe('hello\n');
    });

    it('appends empty line', () => {
      const sb = new StringBuffer();
      sb.CatLine();
      expect(sb.ToString()).toBe('\n');
    });

    it('appends multiple lines', () => {
      const sb = new StringBuffer();
      sb.CatLine('line1').CatLine('line2').CatLine('line3');
      expect(sb.ToString()).toBe('line1\nline2\nline3\n');
    });

    it('appends String object line', () => {
      const sb = new StringBuffer();
      sb.CatLine(new String('hello'));
      expect(sb.ToString()).toBe('hello\n');
    });
  });

  describe('Reserve', () => {
    it('accepts reserve call (no-op)', () => {
      const sb = new StringBuffer();
      expect(() => sb.Reserve(1000)).not.toThrow();
      sb.Cat('hello');
      expect(sb.ToString()).toBe('hello');
    });
  });

  describe('toString and valueOf', () => {
    it('returns string from toString', () => {
      const sb = new StringBuffer();
      sb.Cat('hello');
      expect(sb.toString()).toBe('hello');
    });

    it('returns string from valueOf', () => {
      const sb = new StringBuffer();
      sb.Cat('hello');
      expect(sb.valueOf()).toBe('hello');
    });

    it('allows implicit string conversion', () => {
      const sb = new StringBuffer();
      sb.Cat('hello');
      expect('' + sb).toBe('hello');
    });
  });

  describe('Static From', () => {
    it('creates from array of strings', () => {
      const sb = StringBuffer.From(['hello', ' ', 'world']);
      expect(sb.ToString()).toBe('hello world');
    });

    it('creates from array of String objects', () => {
      const sb = StringBuffer.From([new String('hello'), new String(' '), new String('world')]);
      expect(sb.ToString()).toBe('hello world');
    });

    it('creates from mixed array', () => {
      const sb = StringBuffer.From(['hello', new String(' '), 'world']);
      expect(sb.ToString()).toBe('hello world');
    });

    it('creates from empty array', () => {
      const sb = StringBuffer.From([]);
      expect(sb.IsEmpty()).toBe(true);
    });
  });

  describe('Static Join', () => {
    it('joins with separator', () => {
      const sb = StringBuffer.Join(', ', ['apple', 'banana', 'cherry']);
      expect(sb.ToString()).toBe('apple, banana, cherry');
    });

    it('joins String objects', () => {
      const sb = StringBuffer.Join('-', [new String('a'), new String('b'), new String('c')]);
      expect(sb.ToString()).toBe('a-b-c');
    });

    it('joins mixed array', () => {
      const sb = StringBuffer.Join(' ', ['hello', new String('world')]);
      expect(sb.ToString()).toBe('hello world');
    });

    it('joins empty array', () => {
      const sb = StringBuffer.Join(',', []);
      expect(sb.IsEmpty()).toBe(true);
    });

    it('joins single element', () => {
      const sb = StringBuffer.Join(',', ['alone']);
      expect(sb.ToString()).toBe('alone');
    });
  });

  describe('Complex usage', () => {
    it('builds multi-line document', () => {
      const sb = new StringBuffer();
      sb.CatLine('# Title')
        .CatLine()
        .CatLine('This is a paragraph.')
        .CatLine()
        .CatFormat('Version: %d.%d.%d', 1, 2, 3)
        .CatLine();

      const expected = '# Title\n\nThis is a paragraph.\n\nVersion: 1.2.3\n';
      expect(sb.ToString()).toBe(expected);
    });

    it('builds formatted table', () => {
      const sb = new StringBuffer();
      // Simple format without padding (our implementation doesn't support %-10s style)
      sb.CatFormat('%s | %s | %s', 'Name', 'Age', 'Score').CatLine();
      expect(sb.ToString()).toContain('Name');
      expect(sb.ToString()).toContain('Age');
      expect(sb.ToString()).toContain('Score');
      expect(sb.ToString()).toContain('|');
    });

    it('builds JSON-like structure', () => {
      const sb = new StringBuffer();
      sb.Cat('{')
        .CatLine()
        .Cat('  "name": "Alice",')
        .CatLine()
        .Cat('  "age": ')
        .CatInt(30)
        .CatLine()
        .Cat('}');

      expect(sb.ToString()).toContain('"name": "Alice"');
      expect(sb.ToString()).toContain('"age": 30');
    });
  });

  describe('Performance', () => {
    it('handles many concatenations efficiently', () => {
      const sb = new StringBuffer();

      for (let i = 0; i < 1000; i++) {
        sb.Cat('x');
      }

      expect(sb.GetLength()).toBe(1000);
      expect(sb.GetPartCount()).toBe(1000);
      expect(sb.ToString().length).toBe(1000);
    });

    it('builds large strings efficiently', () => {
      const sb = new StringBuffer();

      for (let i = 0; i < 100; i++) {
        sb.CatFormat('Line %d: ', i).CatLine('Some text here');
      }

      expect(sb.GetLength()).toBeGreaterThan(1000);
      expect(sb.ToString().split('\n').length).toBe(101); // 100 lines + empty last
    });
  });

  describe('Edge cases', () => {
    it('handles empty operations', () => {
      const sb = new StringBuffer();
      sb.Cat('').CatChar('').Cat('');
      expect(sb.ToString()).toBe('');
    });

    it('handles zero and negative numbers', () => {
      const sb = new StringBuffer();
      sb.CatInt(0).Cat(' ').CatInt(-5);
      expect(sb.ToString()).toBe('0 -5');
    });

    it('handles special characters', () => {
      const sb = new StringBuffer();
      sb.Cat('Line1\n').Cat('Tab\there').Cat('\r\nCRLF');
      expect(sb.ToString()).toContain('\n');
      expect(sb.ToString()).toContain('\t');
      expect(sb.ToString()).toContain('\r');
    });

    it('handles Unicode', () => {
      const sb = new StringBuffer();
      sb.Cat('Hello ').Cat('世界').Cat(' 🌍');
      expect(sb.ToString()).toBe('Hello 世界 🌍');
    });
  });
});
