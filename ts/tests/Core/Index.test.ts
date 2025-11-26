import { Index } from '../../src/Core/Index';

describe('Index', () => {
  describe('constructor', () => {
    it('creates empty index', () => {
      const idx = new Index<number>();
      expect(idx.GetCount()).toBe(0);
      expect(idx.IsEmpty()).toBe(true);
    });

    it('accepts custom comparison function', () => {
      // Reverse order
      const idx = new Index<number>((a, b) => b - a);
      idx.Add(1);
      idx.Add(2);
      idx.Add(3);
      expect(idx.At(0)).toBe(3);
      expect(idx.At(1)).toBe(2);
      expect(idx.At(2)).toBe(1);
    });
  });

  describe('Add', () => {
    it('maintains sorted order', () => {
      const idx = new Index<number>();
      idx.Add(3);
      idx.Add(1);
      idx.Add(2);
      expect(idx.At(0)).toBe(1);
      expect(idx.At(1)).toBe(2);
      expect(idx.At(2)).toBe(3);
    });

    it('returns insertion position', () => {
      const idx = new Index<number>();
      const pos1 = idx.Add(5);
      const pos2 = idx.Add(3);
      const pos3 = idx.Add(7);
      expect(pos1).toBe(0); // First element
      expect(pos2).toBe(0); // Inserted before 5
      expect(pos3).toBe(2); // Inserted at end
    });

    it('handles duplicate values', () => {
      const idx = new Index<number>();
      idx.Add(1);
      idx.Add(1);
      idx.Add(1);
      expect(idx.GetCount()).toBe(3);
    });

    it('works with strings', () => {
      const idx = new Index<string>();
      idx.Add('charlie');
      idx.Add('alice');
      idx.Add('bob');
      expect(idx.At(0)).toBe('alice');
      expect(idx.At(1)).toBe('bob');
      expect(idx.At(2)).toBe('charlie');
    });
  });

  describe('Find', () => {
    it('finds existing element', () => {
      const idx = new Index<number>();
      idx.Add(10);
      idx.Add(20);
      idx.Add(30);
      expect(idx.Find(20)).toBe(1);
    });

    it('returns -1 for non-existent element', () => {
      const idx = new Index<number>();
      idx.Add(10);
      idx.Add(30);
      expect(idx.Find(20)).toBe(-1);
    });

    it('finds in large dataset', () => {
      const idx = new Index<number>();
      for (let i = 0; i < 1000; i++) {
        idx.Add(i * 2);
      }
      expect(idx.Find(500)).toBe(250);
      expect(idx.Find(501)).toBe(-1);
    });
  });

  describe('FindMatch', () => {
    it('finds element by predicate', () => {
      const idx = new Index<number>();
      idx.Add(1);
      idx.Add(2);
      idx.Add(3);
      const pos = idx.FindMatch((x) => x > 1 && x < 3);
      expect(pos).toBe(1);
      expect(idx.At(pos)).toBe(2);
    });

    it('returns -1 when no match', () => {
      const idx = new Index<number>();
      idx.Add(1);
      idx.Add(2);
      const pos = idx.FindMatch((x) => x > 10);
      expect(pos).toBe(-1);
    });
  });

  describe('Contains', () => {
    it('returns true for existing element', () => {
      const idx = new Index<number>();
      idx.Add(1);
      idx.Add(2);
      idx.Add(3);
      expect(idx.Contains(2)).toBe(true);
    });

    it('returns false for non-existent element', () => {
      const idx = new Index<number>();
      idx.Add(1);
      expect(idx.Contains(99)).toBe(false);
    });
  });

  describe('At', () => {
    it('gets element at valid index', () => {
      const idx = new Index<number>();
      idx.Add(3);
      idx.Add(1);
      idx.Add(2);
      expect(idx.At(0)).toBe(1);
      expect(idx.At(1)).toBe(2);
      expect(idx.At(2)).toBe(3);
    });

    it('throws on out of bounds access', () => {
      const idx = new Index<number>();
      idx.Add(1);
      expect(() => idx.At(-1)).toThrow('Index out of bounds');
      expect(() => idx.At(1)).toThrow('Index out of bounds');
    });
  });

  describe('GetCount and IsEmpty', () => {
    it('returns correct count', () => {
      const idx = new Index<number>();
      expect(idx.GetCount()).toBe(0);
      idx.Add(1);
      expect(idx.GetCount()).toBe(1);
      idx.Add(2);
      expect(idx.GetCount()).toBe(2);
    });

    it('reports empty status correctly', () => {
      const idx = new Index<number>();
      expect(idx.IsEmpty()).toBe(true);
      idx.Add(1);
      expect(idx.IsEmpty()).toBe(false);
      idx.Clear();
      expect(idx.IsEmpty()).toBe(true);
    });
  });

  describe('Clear', () => {
    it('removes all elements', () => {
      const idx = new Index<number>();
      idx.Add(1);
      idx.Add(2);
      idx.Add(3);
      idx.Clear();
      expect(idx.GetCount()).toBe(0);
      expect(idx.IsEmpty()).toBe(true);
    });
  });

  describe('RemoveAt', () => {
    it('removes element at index', () => {
      const idx = new Index<number>();
      idx.Add(1);
      idx.Add(2);
      idx.Add(3);
      idx.RemoveAt(1);
      expect(idx.GetCount()).toBe(2);
      expect(idx.At(0)).toBe(1);
      expect(idx.At(1)).toBe(3);
    });

    it('throws on invalid index', () => {
      const idx = new Index<number>();
      expect(() => idx.RemoveAt(0)).toThrow('Index out of bounds');
    });
  });

  describe('RemoveKey', () => {
    it('removes element by value', () => {
      const idx = new Index<number>();
      idx.Add(1);
      idx.Add(2);
      idx.Add(3);
      const removed = idx.RemoveKey(2);
      expect(removed).toBe(true);
      expect(idx.GetCount()).toBe(2);
      expect(idx.Contains(2)).toBe(false);
    });

    it('returns false for non-existent value', () => {
      const idx = new Index<number>();
      idx.Add(1);
      const removed = idx.RemoveKey(99);
      expect(removed).toBe(false);
      expect(idx.GetCount()).toBe(1);
    });
  });

  describe('GetKeys', () => {
    it('returns sorted array of all elements', () => {
      const idx = new Index<number>();
      idx.Add(3);
      idx.Add(1);
      idx.Add(2);
      const keys = idx.GetKeys();
      expect(keys).toEqual([1, 2, 3]);
    });

    it('returns copy not reference', () => {
      const idx = new Index<number>();
      idx.Add(1);
      const keys = idx.GetKeys();
      keys.push(999);
      expect(idx.GetCount()).toBe(1);
    });
  });

  describe('FindAdd', () => {
    it('inserts new element', () => {
      const idx = new Index<number>();
      const [pos, wasInserted] = idx.FindAdd(5);
      expect(wasInserted).toBe(true);
      expect(pos).toBe(0);
      expect(idx.GetCount()).toBe(1);
    });

    it('finds existing element without inserting', () => {
      const idx = new Index<number>();
      idx.Add(5);
      const [pos, wasInserted] = idx.FindAdd(5);
      expect(wasInserted).toBe(false);
      expect(pos).toBe(0);
      expect(idx.GetCount()).toBe(1);
    });

    it('prevents duplicates when using FindAdd', () => {
      const idx = new Index<number>();
      const [pos1, inserted1] = idx.FindAdd(1);
      expect(inserted1).toBe(true);

      const [pos2, inserted2] = idx.FindAdd(2);
      expect(inserted2).toBe(true);

      const [pos3, inserted3] = idx.FindAdd(1); // Duplicate
      expect(inserted3).toBe(false);
      expect(pos3).toBe(pos1); // Same position as first insert

      idx.FindAdd(3);
      // Only 3 unique elements
      expect(idx.GetCount()).toBe(3);
      expect(idx.GetKeys()).toEqual([1, 2, 3]);
    });
  });

  describe('Get', () => {
    it('returns element at valid index', () => {
      const idx = new Index<number>();
      idx.Add(10);
      expect(idx.Get(0, 999)).toBe(10);
    });

    it('returns default for invalid index', () => {
      const idx = new Index<number>();
      expect(idx.Get(0, 999)).toBe(999);
      expect(idx.Get(-1, 999)).toBe(999);
    });
  });

  describe('Detach', () => {
    it('removes and returns element', () => {
      const idx = new Index<number>();
      idx.Add(1);
      idx.Add(2);
      idx.Add(3);
      const val = idx.Detach(1);
      expect(val).toBe(2);
      expect(idx.GetCount()).toBe(2);
      expect(idx.At(0)).toBe(1);
      expect(idx.At(1)).toBe(3);
    });

    it('throws on invalid index', () => {
      const idx = new Index<number>();
      expect(() => idx.Detach(0)).toThrow('Index out of bounds');
    });
  });

  describe('Top and Head', () => {
    it('Top returns last element', () => {
      const idx = new Index<number>();
      idx.Add(1);
      idx.Add(3);
      idx.Add(2);
      expect(idx.Top()).toBe(3); // Highest in sorted order
    });

    it('Head returns first element', () => {
      const idx = new Index<number>();
      idx.Add(3);
      idx.Add(1);
      idx.Add(2);
      expect(idx.Head()).toBe(1); // Lowest in sorted order
    });

    it('throws on empty index', () => {
      const idx = new Index<number>();
      expect(() => idx.Top()).toThrow('Index is empty');
      expect(() => idx.Head()).toThrow('Index is empty');
    });
  });

  describe('Static From', () => {
    it('creates index from array', () => {
      const idx = Index.From([3, 1, 2]);
      expect(idx.GetCount()).toBe(3);
      expect(idx.At(0)).toBe(1);
      expect(idx.At(1)).toBe(2);
      expect(idx.At(2)).toBe(3);
    });

    it('accepts comparison function', () => {
      const idx = Index.From([1, 2, 3], (a, b) => b - a);
      expect(idx.At(0)).toBe(3);
      expect(idx.At(1)).toBe(2);
      expect(idx.At(2)).toBe(1);
    });
  });

  describe('Iterator', () => {
    it('supports for-of iteration', () => {
      const idx = new Index<number>();
      idx.Add(3);
      idx.Add(1);
      idx.Add(2);

      const result: number[] = [];
      for (const x of idx) {
        result.push(x);
      }

      expect(result).toEqual([1, 2, 3]);
    });
  });

  describe('toString', () => {
    it('returns string representation', () => {
      const idx = new Index<number>();
      idx.Add(3);
      idx.Add(1);
      idx.Add(2);
      const str = idx.toString();
      expect(str).toContain('Index');
      expect(str).toContain('1');
      expect(str).toContain('2');
      expect(str).toContain('3');
    });
  });

  describe('Complex types', () => {
    interface Person {
      id: number;
      name: string;
    }

    it('works with objects using custom comparator', () => {
      const idx = new Index<Person>((a, b) => a.id - b.id);
      idx.Add({ id: 3, name: 'Charlie' });
      idx.Add({ id: 1, name: 'Alice' });
      idx.Add({ id: 2, name: 'Bob' });

      expect(idx.GetCount()).toBe(3);
      expect(idx.At(0).name).toBe('Alice');
      expect(idx.At(1).name).toBe('Bob');
      expect(idx.At(2).name).toBe('Charlie');
    });

    it('finds objects using custom comparison', () => {
      const idx = new Index<Person>((a, b) => a.id - b.id);
      idx.Add({ id: 1, name: 'Alice' });
      idx.Add({ id: 2, name: 'Bob' });

      // Find by creating search object with same comparator key
      const found = idx.FindMatch((p) => p.id === 2);
      expect(found).toBe(1);
      expect(idx.At(found).name).toBe('Bob');
    });
  });

  describe('Performance', () => {
    it('handles large datasets efficiently', () => {
      const idx = new Index<number>();
      const size = 10000;

      // Add in random order
      for (let i = 0; i < size; i++) {
        idx.Add(Math.floor(Math.random() * size * 2));
      }

      expect(idx.GetCount()).toBe(size);

      // Verify sorted order
      for (let i = 1; i < idx.GetCount(); i++) {
        expect(idx.At(i)).toBeGreaterThanOrEqual(idx.At(i - 1));
      }
    });
  });
});
