import { Vector } from '../../src/Core/Vector';

describe('Vector', () => {
  describe('constructor', () => {
    it('creates empty vector', () => {
      const v = new Vector<number>();
      expect(v.GetCount()).toBe(0);
      expect(v.IsEmpty()).toBe(true);
    });
  });

  describe('Add', () => {
    it('adds element to end', () => {
      const v = new Vector<number>();
      v.Add(1);
      v.Add(2);
      v.Add(3);
      expect(v.GetCount()).toBe(3);
      expect(v.At(0)).toBe(1);
      expect(v.At(1)).toBe(2);
      expect(v.At(2)).toBe(3);
    });

    it('returns added value', () => {
      const v = new Vector<number>();
      const result = v.Add(42);
      expect(result).toBe(42);
    });
  });

  describe('AddPick', () => {
    it('adds element and returns index', () => {
      const v = new Vector<string>();
      const idx = v.AddPick('test');
      expect(idx).toBe(0);
      expect(v.At(idx)).toBe('test');
    });
  });

  describe('At and Set', () => {
    it('gets element at valid index', () => {
      const v = new Vector<number>();
      v.Add(10);
      v.Add(20);
      expect(v.At(0)).toBe(10);
      expect(v.At(1)).toBe(20);
    });

    it('throws on out of bounds access', () => {
      const v = new Vector<number>();
      v.Add(1);
      expect(() => v.At(-1)).toThrow('Vector index out of bounds');
      expect(() => v.At(1)).toThrow('Vector index out of bounds');
    });

    it('sets element at valid index', () => {
      const v = new Vector<number>();
      v.Add(1);
      v.Add(2);
      v.Set(0, 99);
      expect(v.At(0)).toBe(99);
      expect(v.At(1)).toBe(2);
    });

    it('throws when setting out of bounds', () => {
      const v = new Vector<number>();
      expect(() => v.Set(0, 1)).toThrow('Vector index out of bounds');
    });
  });

  describe('GetCount and IsEmpty', () => {
    it('returns correct count', () => {
      const v = new Vector<number>();
      expect(v.GetCount()).toBe(0);
      v.Add(1);
      expect(v.GetCount()).toBe(1);
      v.Add(2);
      expect(v.GetCount()).toBe(2);
    });

    it('reports empty status correctly', () => {
      const v = new Vector<number>();
      expect(v.IsEmpty()).toBe(true);
      v.Add(1);
      expect(v.IsEmpty()).toBe(false);
      v.Clear();
      expect(v.IsEmpty()).toBe(true);
    });
  });

  describe('Clear', () => {
    it('removes all elements', () => {
      const v = new Vector<number>();
      v.Add(1);
      v.Add(2);
      v.Add(3);
      v.Clear();
      expect(v.GetCount()).toBe(0);
      expect(v.IsEmpty()).toBe(true);
    });
  });

  describe('Shrink', () => {
    it('reduces vector size', () => {
      const v = new Vector<number>();
      v.Add(1);
      v.Add(2);
      v.Add(3);
      v.Shrink(1);
      expect(v.GetCount()).toBe(1);
      expect(v.At(0)).toBe(1);
    });

    it('does nothing if size already smaller', () => {
      const v = new Vector<number>();
      v.Add(1);
      v.Shrink(10);
      expect(v.GetCount()).toBe(1);
    });
  });

  describe('SetCount', () => {
    it('shrinks when new size is smaller', () => {
      const v = new Vector<number>();
      v.Add(1);
      v.Add(2);
      v.Add(3);
      v.SetCount(1);
      expect(v.GetCount()).toBe(1);
    });

    it('grows when new size is larger', () => {
      const v = new Vector<number>();
      v.Add(1);
      v.SetCount(3, 0);
      expect(v.GetCount()).toBe(3);
      expect(v.At(0)).toBe(1);
      expect(v.At(1)).toBe(0);
      expect(v.At(2)).toBe(0);
    });

    it('fills with undefined when no fill value provided', () => {
      const v = new Vector<number | undefined>();
      v.Add(1);
      v.SetCount(3);
      expect(v.GetCount()).toBe(3);
      expect(v.At(1)).toBeUndefined();
    });
  });

  describe('Remove', () => {
    it('removes element at index', () => {
      const v = new Vector<number>();
      v.Add(1);
      v.Add(2);
      v.Add(3);
      v.Remove(1);
      expect(v.GetCount()).toBe(2);
      expect(v.At(0)).toBe(1);
      expect(v.At(1)).toBe(3);
    });

    it('throws on invalid index', () => {
      const v = new Vector<number>();
      v.Add(1);
      expect(() => v.Remove(5)).toThrow('Vector index out of bounds');
    });
  });

  describe('RemoveRange', () => {
    it('removes multiple elements', () => {
      const v = new Vector<number>();
      v.Add(1);
      v.Add(2);
      v.Add(3);
      v.Add(4);
      v.RemoveRange(1, 2);
      expect(v.GetCount()).toBe(2);
      expect(v.At(0)).toBe(1);
      expect(v.At(1)).toBe(4);
    });
  });

  describe('Insert', () => {
    it('inserts at beginning', () => {
      const v = new Vector<number>();
      v.Add(2);
      v.Add(3);
      v.Insert(0, 1);
      expect(v.GetCount()).toBe(3);
      expect(v.At(0)).toBe(1);
      expect(v.At(1)).toBe(2);
      expect(v.At(2)).toBe(3);
    });

    it('inserts at end', () => {
      const v = new Vector<number>();
      v.Add(1);
      v.Add(2);
      v.Insert(2, 3);
      expect(v.GetCount()).toBe(3);
      expect(v.At(2)).toBe(3);
    });

    it('inserts in middle', () => {
      const v = new Vector<number>();
      v.Add(1);
      v.Add(3);
      v.Insert(1, 2);
      expect(v.GetCount()).toBe(3);
      expect(v.At(1)).toBe(2);
    });
  });

  describe('InsertPick', () => {
    it('inserts and returns index', () => {
      const v = new Vector<number>();
      v.Add(1);
      const idx = v.InsertPick(1, 2);
      expect(idx).toBe(1);
      expect(v.At(idx)).toBe(2);
    });
  });

  describe('Top', () => {
    it('returns last element', () => {
      const v = new Vector<number>();
      v.Add(1);
      v.Add(2);
      v.Add(3);
      expect(v.Top()).toBe(3);
    });

    it('throws on empty vector', () => {
      const v = new Vector<number>();
      expect(() => v.Top()).toThrow('Vector is empty');
    });
  });

  describe('Pop', () => {
    it('removes and returns last element', () => {
      const v = new Vector<number>();
      v.Add(1);
      v.Add(2);
      v.Add(3);
      const last = v.Pop();
      expect(last).toBe(3);
      expect(v.GetCount()).toBe(2);
    });

    it('throws on empty vector', () => {
      const v = new Vector<number>();
      expect(() => v.Pop()).toThrow('Vector is empty');
    });
  });

  describe('Drop', () => {
    it('removes last element without returning', () => {
      const v = new Vector<number>();
      v.Add(1);
      v.Add(2);
      v.Drop();
      expect(v.GetCount()).toBe(1);
    });

    it('does nothing on empty vector', () => {
      const v = new Vector<number>();
      v.Drop();
      expect(v.GetCount()).toBe(0);
    });
  });

  describe('Find', () => {
    it('finds element by predicate', () => {
      const v = new Vector<number>();
      v.Add(1);
      v.Add(2);
      v.Add(3);
      const idx = v.Find((x) => x === 2);
      expect(idx).toBe(1);
    });

    it('returns -1 when not found', () => {
      const v = new Vector<number>();
      v.Add(1);
      const idx = v.Find((x) => x === 99);
      expect(idx).toBe(-1);
    });
  });

  describe('FindValue', () => {
    it('finds element by value', () => {
      const v = new Vector<string>();
      v.Add('a');
      v.Add('b');
      v.Add('c');
      const idx = v.FindValue('b');
      expect(idx).toBe(1);
    });

    it('returns -1 when not found', () => {
      const v = new Vector<string>();
      v.Add('a');
      const idx = v.FindValue('z');
      expect(idx).toBe(-1);
    });
  });

  describe('Append', () => {
    it('appends another vector', () => {
      const v1 = new Vector<number>();
      v1.Add(1);
      v1.Add(2);

      const v2 = new Vector<number>();
      v2.Add(3);
      v2.Add(4);

      v1.Append(v2);
      expect(v1.GetCount()).toBe(4);
      expect(v1.At(2)).toBe(3);
      expect(v1.At(3)).toBe(4);
    });
  });

  describe('Static From', () => {
    it('creates vector from array', () => {
      const v = Vector.From([1, 2, 3]);
      expect(v.GetCount()).toBe(3);
      expect(v.At(0)).toBe(1);
      expect(v.At(1)).toBe(2);
      expect(v.At(2)).toBe(3);
    });
  });

  describe('Iterator', () => {
    it('supports for-of iteration', () => {
      const v = new Vector<number>();
      v.Add(1);
      v.Add(2);
      v.Add(3);

      const result: number[] = [];
      for (const x of v) {
        result.push(x);
      }

      expect(result).toEqual([1, 2, 3]);
    });
  });

  describe('toString', () => {
    it('returns string representation', () => {
      const v = new Vector<number>();
      v.Add(1);
      v.Add(2);
      const str = v.toString();
      expect(str).toContain('Vector');
      expect(str).toContain('1');
      expect(str).toContain('2');
    });
  });

  describe('Complex types', () => {
    it('works with objects', () => {
      interface Point {
        x: number;
        y: number;
      }

      const v = new Vector<Point>();
      v.Add({ x: 1, y: 2 });
      v.Add({ x: 3, y: 4 });

      expect(v.GetCount()).toBe(2);
      expect(v.At(0).x).toBe(1);
      expect(v.At(1).y).toBe(4);
    });
  });
});
