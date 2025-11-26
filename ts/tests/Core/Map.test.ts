import { Map } from '../../src/Core/Map';

describe('Map', () => {
  describe('constructor', () => {
    it('creates empty map', () => {
      const map = new Map<string, number>();
      expect(map.GetCount()).toBe(0);
      expect(map.IsEmpty()).toBe(true);
    });
  });

  describe('Add and Get', () => {
    it('adds and retrieves key-value pairs', () => {
      const map = new Map<string, number>();
      map.Add('one', 1);
      map.Add('two', 2);
      map.Add('three', 3);

      expect(map.Get('one', 0)).toBe(1);
      expect(map.Get('two', 0)).toBe(2);
      expect(map.Get('three', 0)).toBe(3);
      expect(map.GetCount()).toBe(3);
    });

    it('overwrites existing value', () => {
      const map = new Map<string, number>();
      map.Add('key', 1);
      map.Add('key', 2);

      expect(map.Get('key', 0)).toBe(2);
      expect(map.GetCount()).toBe(1);
    });

    it('returns default for non-existent key', () => {
      const map = new Map<string, number>();
      expect(map.Get('missing', 999)).toBe(999);
    });

    it('returns reference to added value', () => {
      const map = new Map<string, number>();
      const val = map.Add('test', 42);
      expect(val).toBe(42);
    });
  });

  describe('GetThrow', () => {
    it('returns value for existing key', () => {
      const map = new Map<string, number>();
      map.Add('key', 123);
      expect(map.GetThrow('key')).toBe(123);
    });

    it('throws for non-existent key', () => {
      const map = new Map<string, number>();
      expect(() => map.GetThrow('missing')).toThrow('Key not found');
    });
  });

  describe('Find', () => {
    it('finds key in iteration order', () => {
      const map = new Map<string, number>();
      map.Add('a', 1);
      map.Add('b', 2);
      map.Add('c', 3);

      expect(map.Find('a')).toBe(0);
      expect(map.Find('b')).toBe(1);
      expect(map.Find('c')).toBe(2);
    });

    it('returns -1 for non-existent key', () => {
      const map = new Map<string, number>();
      map.Add('a', 1);
      expect(map.Find('missing')).toBe(-1);
    });
  });

  describe('Contains', () => {
    it('returns true for existing key', () => {
      const map = new Map<string, number>();
      map.Add('key', 123);
      expect(map.Contains('key')).toBe(true);
    });

    it('returns false for non-existent key', () => {
      const map = new Map<string, number>();
      expect(map.Contains('missing')).toBe(false);
    });
  });

  describe('Set', () => {
    it('sets value for existing key', () => {
      const map = new Map<string, number>();
      map.Add('key', 1);
      map.Set('key', 2);
      expect(map.Get('key', 0)).toBe(2);
    });

    it('adds new key-value pair if key does not exist', () => {
      const map = new Map<string, number>();
      map.Set('key', 123);
      expect(map.Get('key', 0)).toBe(123);
      expect(map.GetCount()).toBe(1);
    });
  });

  describe('GetKey and GetValue', () => {
    it('gets key at index', () => {
      const map = new Map<string, number>();
      map.Add('a', 1);
      map.Add('b', 2);
      map.Add('c', 3);

      expect(map.GetKey(0)).toBe('a');
      expect(map.GetKey(1)).toBe('b');
      expect(map.GetKey(2)).toBe('c');
    });

    it('gets value at index', () => {
      const map = new Map<string, number>();
      map.Add('a', 1);
      map.Add('b', 2);
      map.Add('c', 3);

      expect(map.GetValue(0)).toBe(1);
      expect(map.GetValue(1)).toBe(2);
      expect(map.GetValue(2)).toBe(3);
    });

    it('throws on out of bounds key access', () => {
      const map = new Map<string, number>();
      map.Add('a', 1);
      expect(() => map.GetKey(-1)).toThrow('Map index out of bounds');
      expect(() => map.GetKey(1)).toThrow('Map index out of bounds');
    });

    it('throws on out of bounds value access', () => {
      const map = new Map<string, number>();
      map.Add('a', 1);
      expect(() => map.GetValue(-1)).toThrow('Map index out of bounds');
      expect(() => map.GetValue(1)).toThrow('Map index out of bounds');
    });
  });

  describe('GetCount and IsEmpty', () => {
    it('returns correct count', () => {
      const map = new Map<string, number>();
      expect(map.GetCount()).toBe(0);
      map.Add('a', 1);
      expect(map.GetCount()).toBe(1);
      map.Add('b', 2);
      expect(map.GetCount()).toBe(2);
    });

    it('reports empty status correctly', () => {
      const map = new Map<string, number>();
      expect(map.IsEmpty()).toBe(true);
      map.Add('a', 1);
      expect(map.IsEmpty()).toBe(false);
      map.Clear();
      expect(map.IsEmpty()).toBe(true);
    });
  });

  describe('Clear', () => {
    it('removes all entries', () => {
      const map = new Map<string, number>();
      map.Add('a', 1);
      map.Add('b', 2);
      map.Add('c', 3);
      map.Clear();
      expect(map.GetCount()).toBe(0);
      expect(map.IsEmpty()).toBe(true);
    });
  });

  describe('RemoveKey', () => {
    it('removes entry by key', () => {
      const map = new Map<string, number>();
      map.Add('a', 1);
      map.Add('b', 2);
      map.Add('c', 3);

      const removed = map.RemoveKey('b');
      expect(removed).toBe(true);
      expect(map.GetCount()).toBe(2);
      expect(map.Contains('b')).toBe(false);
    });

    it('returns false for non-existent key', () => {
      const map = new Map<string, number>();
      map.Add('a', 1);
      const removed = map.RemoveKey('missing');
      expect(removed).toBe(false);
      expect(map.GetCount()).toBe(1);
    });
  });

  describe('RemoveAt', () => {
    it('removes entry at index', () => {
      const map = new Map<string, number>();
      map.Add('a', 1);
      map.Add('b', 2);
      map.Add('c', 3);

      map.RemoveAt(1);
      expect(map.GetCount()).toBe(2);
      expect(map.Contains('b')).toBe(false);
    });

    it('throws on invalid index', () => {
      const map = new Map<string, number>();
      expect(() => map.RemoveAt(0)).toThrow('Map index out of bounds');
    });
  });

  describe('GetKeys and GetValues', () => {
    it('returns array of keys', () => {
      const map = new Map<string, number>();
      map.Add('a', 1);
      map.Add('b', 2);
      map.Add('c', 3);

      const keys = map.GetKeys();
      expect(keys).toEqual(['a', 'b', 'c']);
    });

    it('returns array of values', () => {
      const map = new Map<string, number>();
      map.Add('a', 1);
      map.Add('b', 2);
      map.Add('c', 3);

      const values = map.GetValues();
      expect(values).toEqual([1, 2, 3]);
    });

    it('returns empty arrays for empty map', () => {
      const map = new Map<string, number>();
      expect(map.GetKeys()).toEqual([]);
      expect(map.GetValues()).toEqual([]);
    });
  });

  describe('FindAdd', () => {
    it('inserts new entry', () => {
      const map = new Map<string, number>();
      const [idx, wasInserted] = map.FindAdd('key', 123);

      expect(wasInserted).toBe(true);
      expect(idx).toBe(0);
      expect(map.GetCount()).toBe(1);
      expect(map.Get('key', 0)).toBe(123);
    });

    it('updates existing entry', () => {
      const map = new Map<string, number>();
      map.Add('key', 1);

      const [idx, wasInserted] = map.FindAdd('key', 2);

      expect(wasInserted).toBe(false);
      expect(idx).toBe(0);
      expect(map.GetCount()).toBe(1);
      expect(map.Get('key', 0)).toBe(2);
    });
  });

  describe('GetAdd', () => {
    it('returns existing value', () => {
      const map = new Map<string, number>();
      map.Add('key', 123);

      const val = map.GetAdd('key', () => 999);
      expect(val).toBe(123);
      expect(map.GetCount()).toBe(1);
    });

    it('creates and returns new value', () => {
      const map = new Map<string, number>();
      const val = map.GetAdd('key', () => 999);

      expect(val).toBe(999);
      expect(map.GetCount()).toBe(1);
      expect(map.Get('key', 0)).toBe(999);
    });

    it('calls factory only when needed', () => {
      const map = new Map<string, number>();
      map.Add('key', 123);

      let factoryCalled = false;
      map.GetAdd('key', () => {
        factoryCalled = true;
        return 999;
      });

      expect(factoryCalled).toBe(false);
    });
  });

  describe('Static From', () => {
    it('creates map from entries', () => {
      const map = Map.From([
        ['a', 1],
        ['b', 2],
        ['c', 3],
      ]);

      expect(map.GetCount()).toBe(3);
      expect(map.Get('a', 0)).toBe(1);
      expect(map.Get('b', 0)).toBe(2);
      expect(map.Get('c', 0)).toBe(3);
    });

    it('handles duplicate keys by keeping last', () => {
      const map = Map.From([
        ['key', 1],
        ['key', 2],
      ]);

      expect(map.GetCount()).toBe(1);
      expect(map.Get('key', 0)).toBe(2);
    });
  });

  describe('Iterator', () => {
    it('supports for-of iteration over entries', () => {
      const map = new Map<string, number>();
      map.Add('a', 1);
      map.Add('b', 2);
      map.Add('c', 3);

      const entries: [string, number][] = [];
      for (const entry of map) {
        entries.push(entry);
      }

      expect(entries).toEqual([
        ['a', 1],
        ['b', 2],
        ['c', 3],
      ]);
    });

    it('provides entries() iterator', () => {
      const map = new Map<string, number>();
      map.Add('a', 1);
      map.Add('b', 2);

      const entries = Array.from(map.entries());
      expect(entries).toEqual([
        ['a', 1],
        ['b', 2],
      ]);
    });

    it('provides keys() iterator', () => {
      const map = new Map<string, number>();
      map.Add('a', 1);
      map.Add('b', 2);

      const keys = Array.from(map.keys());
      expect(keys).toEqual(['a', 'b']);
    });

    it('provides values() iterator', () => {
      const map = new Map<string, number>();
      map.Add('a', 1);
      map.Add('b', 2);

      const values = Array.from(map.values());
      expect(values).toEqual([1, 2]);
    });
  });

  describe('toString', () => {
    it('returns string representation', () => {
      const map = new Map<string, number>();
      map.Add('a', 1);
      map.Add('b', 2);

      const str = map.toString();
      expect(str).toContain('Map');
      expect(str).toContain('a: 1');
      expect(str).toContain('b: 2');
    });

    it('shows empty map', () => {
      const map = new Map<string, number>();
      const str = map.toString();
      expect(str).toContain('Map[0]');
    });
  });

  describe('Complex types', () => {
    interface Person {
      id: number;
      name: string;
    }

    it('works with number keys', () => {
      const map = new Map<number, string>();
      map.Add(1, 'one');
      map.Add(2, 'two');
      map.Add(3, 'three');

      expect(map.Get(2, '')).toBe('two');
      expect(map.GetCount()).toBe(3);
    });

    it('works with object values', () => {
      const map = new Map<string, Person>();
      map.Add('alice', { id: 1, name: 'Alice' });
      map.Add('bob', { id: 2, name: 'Bob' });

      const person = map.Get('alice', { id: 0, name: '' });
      expect(person.id).toBe(1);
      expect(person.name).toBe('Alice');
    });

    it('maintains object references', () => {
      const map = new Map<string, Person>();
      const person = { id: 1, name: 'Alice' };
      map.Add('alice', person);

      person.name = 'Alice Updated';
      expect(map.Get('alice', { id: 0, name: '' }).name).toBe('Alice Updated');
    });
  });

  describe('Performance', () => {
    it('handles large datasets efficiently', () => {
      const map = new Map<number, number>();
      const size = 10000;

      for (let i = 0; i < size; i++) {
        map.Add(i, i * 2);
      }

      expect(map.GetCount()).toBe(size);

      // Fast lookup
      for (let i = 0; i < 100; i++) {
        const key = Math.floor(Math.random() * size);
        expect(map.Get(key, -1)).toBe(key * 2);
      }
    });
  });
});
