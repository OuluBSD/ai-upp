import { BiVector } from '../../src/Core/BiVector';

describe('BiVector', () => {
  describe('constructor', () => {
    it('creates empty bivector', () => {
      const bv = new BiVector<number>();
      expect(bv.GetCount()).toBe(0);
      expect(bv.IsEmpty()).toBe(true);
    });
  });

  describe('AddTail and AddHead', () => {
    it('adds elements to tail', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(2);
      bv.AddTail(3);

      expect(bv.GetCount()).toBe(3);
      expect(bv.At(0)).toBe(1);
      expect(bv.At(1)).toBe(2);
      expect(bv.At(2)).toBe(3);
    });

    it('adds elements to head', () => {
      const bv = new BiVector<number>();
      bv.AddHead(1);
      bv.AddHead(2);
      bv.AddHead(3);

      expect(bv.GetCount()).toBe(3);
      expect(bv.At(0)).toBe(3);
      expect(bv.At(1)).toBe(2);
      expect(bv.At(2)).toBe(1);
    });

    it('mixes head and tail additions', () => {
      const bv = new BiVector<number>();
      bv.AddTail(2); // [2]
      bv.AddHead(1); // [1, 2]
      bv.AddTail(3); // [1, 2, 3]
      bv.AddHead(0); // [0, 1, 2, 3]

      expect(bv.GetCount()).toBe(4);
      expect(bv.At(0)).toBe(0);
      expect(bv.At(1)).toBe(1);
      expect(bv.At(2)).toBe(2);
      expect(bv.At(3)).toBe(3);
    });

    it('returns added value', () => {
      const bv = new BiVector<number>();
      const tail = bv.AddTail(42);
      const head = bv.AddHead(24);

      expect(tail).toBe(42);
      expect(head).toBe(24);
    });
  });

  describe('Add alias', () => {
    it('adds to tail by default', () => {
      const bv = new BiVector<number>();
      bv.Add(1);
      bv.Add(2);

      expect(bv.GetCount()).toBe(2);
      expect(bv.At(0)).toBe(1);
      expect(bv.At(1)).toBe(2);
    });
  });

  describe('PopTail and PopHead', () => {
    it('pops from tail', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(2);
      bv.AddTail(3);

      const val = bv.PopTail();
      expect(val).toBe(3);
      expect(bv.GetCount()).toBe(2);
      expect(bv.At(0)).toBe(1);
      expect(bv.At(1)).toBe(2);
    });

    it('pops from head', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(2);
      bv.AddTail(3);

      const val = bv.PopHead();
      expect(val).toBe(1);
      expect(bv.GetCount()).toBe(2);
      expect(bv.At(0)).toBe(2);
      expect(bv.At(1)).toBe(3);
    });

    it('throws when popping empty tail', () => {
      const bv = new BiVector<number>();
      expect(() => bv.PopTail()).toThrow('BiVector is empty');
    });

    it('throws when popping empty head', () => {
      const bv = new BiVector<number>();
      expect(() => bv.PopHead()).toThrow('BiVector is empty');
    });
  });

  describe('Tail and Head', () => {
    it('gets tail element without removing', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(2);
      bv.AddTail(3);

      expect(bv.Tail()).toBe(3);
      expect(bv.GetCount()).toBe(3);
    });

    it('gets head element without removing', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(2);
      bv.AddTail(3);

      expect(bv.Head()).toBe(1);
      expect(bv.GetCount()).toBe(3);
    });

    it('throws when accessing empty tail', () => {
      const bv = new BiVector<number>();
      expect(() => bv.Tail()).toThrow('BiVector is empty');
    });

    it('throws when accessing empty head', () => {
      const bv = new BiVector<number>();
      expect(() => bv.Head()).toThrow('BiVector is empty');
    });
  });

  describe('Top alias', () => {
    it('returns tail element', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(2);

      expect(bv.Top()).toBe(2);
      expect(bv.Top()).toBe(bv.Tail());
    });
  });

  describe('DropTail and DropHead', () => {
    it('drops from tail', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(2);
      bv.AddTail(3);

      bv.DropTail();
      expect(bv.GetCount()).toBe(2);
      expect(bv.At(0)).toBe(1);
      expect(bv.At(1)).toBe(2);
    });

    it('drops from head', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(2);
      bv.AddTail(3);

      bv.DropHead();
      expect(bv.GetCount()).toBe(2);
      expect(bv.At(0)).toBe(2);
      expect(bv.At(1)).toBe(3);
    });

    it('does nothing on empty bivector', () => {
      const bv = new BiVector<number>();
      bv.DropTail();
      bv.DropHead();
      expect(bv.GetCount()).toBe(0);
    });
  });

  describe('Drop alias', () => {
    it('drops from tail', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(2);

      bv.Drop();
      expect(bv.GetCount()).toBe(1);
      expect(bv.At(0)).toBe(1);
    });
  });

  describe('At and Set', () => {
    it('gets element at index', () => {
      const bv = new BiVector<number>();
      bv.AddTail(10);
      bv.AddTail(20);
      bv.AddTail(30);

      expect(bv.At(0)).toBe(10);
      expect(bv.At(1)).toBe(20);
      expect(bv.At(2)).toBe(30);
    });

    it('throws on out of bounds access', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);

      expect(() => bv.At(-1)).toThrow('BiVector index out of bounds');
      expect(() => bv.At(1)).toThrow('BiVector index out of bounds');
    });

    it('sets element at index', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(2);
      bv.AddTail(3);

      bv.Set(1, 99);
      expect(bv.At(1)).toBe(99);
    });

    it('throws when setting out of bounds', () => {
      const bv = new BiVector<number>();
      expect(() => bv.Set(0, 1)).toThrow('BiVector index out of bounds');
    });
  });

  describe('GetCount and IsEmpty', () => {
    it('returns correct count', () => {
      const bv = new BiVector<number>();
      expect(bv.GetCount()).toBe(0);
      bv.AddTail(1);
      expect(bv.GetCount()).toBe(1);
      bv.AddHead(2);
      expect(bv.GetCount()).toBe(2);
    });

    it('reports empty status correctly', () => {
      const bv = new BiVector<number>();
      expect(bv.IsEmpty()).toBe(true);
      bv.AddTail(1);
      expect(bv.IsEmpty()).toBe(false);
      bv.Clear();
      expect(bv.IsEmpty()).toBe(true);
    });
  });

  describe('Clear', () => {
    it('removes all elements', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(2);
      bv.AddTail(3);

      bv.Clear();
      expect(bv.GetCount()).toBe(0);
      expect(bv.IsEmpty()).toBe(true);
    });
  });

  describe('Insert', () => {
    it('inserts at beginning', () => {
      const bv = new BiVector<number>();
      bv.AddTail(2);
      bv.AddTail(3);

      bv.Insert(0, 1);
      expect(bv.GetCount()).toBe(3);
      expect(bv.At(0)).toBe(1);
    });

    it('inserts at end', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(2);

      bv.Insert(2, 3);
      expect(bv.GetCount()).toBe(3);
      expect(bv.At(2)).toBe(3);
    });

    it('inserts in middle', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(3);

      bv.Insert(1, 2);
      expect(bv.GetCount()).toBe(3);
      expect(bv.At(0)).toBe(1);
      expect(bv.At(1)).toBe(2);
      expect(bv.At(2)).toBe(3);
    });

    it('throws on invalid index', () => {
      const bv = new BiVector<number>();
      expect(() => bv.Insert(-1, 1)).toThrow('BiVector index out of bounds');
    });
  });

  describe('Remove', () => {
    it('removes element at index', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(2);
      bv.AddTail(3);

      bv.Remove(1);
      expect(bv.GetCount()).toBe(2);
      expect(bv.At(0)).toBe(1);
      expect(bv.At(1)).toBe(3);
    });

    it('throws on invalid index', () => {
      const bv = new BiVector<number>();
      expect(() => bv.Remove(0)).toThrow('BiVector index out of bounds');
    });
  });

  describe('RemoveRange', () => {
    it('removes multiple elements', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(2);
      bv.AddTail(3);
      bv.AddTail(4);

      bv.RemoveRange(1, 2);
      expect(bv.GetCount()).toBe(2);
      expect(bv.At(0)).toBe(1);
      expect(bv.At(1)).toBe(4);
    });
  });

  describe('Shrink', () => {
    it('reduces size', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(2);
      bv.AddTail(3);

      bv.Shrink(1);
      expect(bv.GetCount()).toBe(1);
      expect(bv.At(0)).toBe(1);
    });

    it('does nothing if size already smaller', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);

      bv.Shrink(10);
      expect(bv.GetCount()).toBe(1);
    });
  });

  describe('SetCount', () => {
    it('shrinks when new size is smaller', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(2);
      bv.AddTail(3);

      bv.SetCount(1);
      expect(bv.GetCount()).toBe(1);
    });

    it('grows when new size is larger', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);

      bv.SetCount(3);
      expect(bv.GetCount()).toBe(3);
    });
  });

  describe('Find', () => {
    it('finds element by predicate', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(2);
      bv.AddTail(3);

      const idx = bv.Find((x) => x > 1 && x < 3);
      expect(idx).toBe(1);
    });

    it('returns -1 when not found', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);

      const idx = bv.Find((x) => x > 10);
      expect(idx).toBe(-1);
    });
  });

  describe('FindValue', () => {
    it('finds element by value', () => {
      const bv = new BiVector<number>();
      bv.AddTail(10);
      bv.AddTail(20);
      bv.AddTail(30);

      expect(bv.FindValue(20)).toBe(1);
    });

    it('returns -1 when not found', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);

      expect(bv.FindValue(99)).toBe(-1);
    });
  });

  describe('Detach', () => {
    it('removes and returns element', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(2);
      bv.AddTail(3);

      const val = bv.Detach(1);
      expect(val).toBe(2);
      expect(bv.GetCount()).toBe(2);
      expect(bv.At(0)).toBe(1);
      expect(bv.At(1)).toBe(3);
    });

    it('throws on invalid index', () => {
      const bv = new BiVector<number>();
      expect(() => bv.Detach(0)).toThrow('BiVector index out of bounds');
    });
  });

  describe('GetData', () => {
    it('returns copy of data', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(2);
      bv.AddTail(3);

      const data = bv.GetData();
      expect(data).toEqual([1, 2, 3]);

      // Verify it's a copy
      data.push(999);
      expect(bv.GetCount()).toBe(3);
    });
  });

  describe('Append and Prepend', () => {
    it('appends array to tail', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.Append([2, 3, 4]);

      expect(bv.GetCount()).toBe(4);
      expect(bv.At(0)).toBe(1);
      expect(bv.At(1)).toBe(2);
      expect(bv.At(2)).toBe(3);
      expect(bv.At(3)).toBe(4);
    });

    it('prepends array to head', () => {
      const bv = new BiVector<number>();
      bv.AddTail(4);
      bv.Prepend([1, 2, 3]);

      expect(bv.GetCount()).toBe(4);
      expect(bv.At(0)).toBe(1);
      expect(bv.At(1)).toBe(2);
      expect(bv.At(2)).toBe(3);
      expect(bv.At(3)).toBe(4);
    });
  });

  describe('Static From', () => {
    it('creates bivector from array', () => {
      const bv = BiVector.From([1, 2, 3]);

      expect(bv.GetCount()).toBe(3);
      expect(bv.At(0)).toBe(1);
      expect(bv.At(1)).toBe(2);
      expect(bv.At(2)).toBe(3);
    });
  });

  describe('Iterator', () => {
    it('supports for-of iteration', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(2);
      bv.AddTail(3);

      const result: number[] = [];
      for (const x of bv) {
        result.push(x);
      }

      expect(result).toEqual([1, 2, 3]);
    });
  });

  describe('toString', () => {
    it('returns string representation', () => {
      const bv = new BiVector<number>();
      bv.AddTail(1);
      bv.AddTail(2);
      bv.AddTail(3);

      const str = bv.toString();
      expect(str).toContain('BiVector');
      expect(str).toContain('1');
      expect(str).toContain('2');
      expect(str).toContain('3');
    });
  });

  describe('Queue operations', () => {
    it('works as FIFO queue', () => {
      const bv = new BiVector<string>();

      // Enqueue
      bv.AddTail('first');
      bv.AddTail('second');
      bv.AddTail('third');

      // Dequeue
      expect(bv.PopHead()).toBe('first');
      expect(bv.PopHead()).toBe('second');
      expect(bv.PopHead()).toBe('third');
      expect(bv.IsEmpty()).toBe(true);
    });

    it('works as LIFO stack', () => {
      const bv = new BiVector<string>();

      // Push
      bv.AddTail('first');
      bv.AddTail('second');
      bv.AddTail('third');

      // Pop
      expect(bv.PopTail()).toBe('third');
      expect(bv.PopTail()).toBe('second');
      expect(bv.PopTail()).toBe('first');
      expect(bv.IsEmpty()).toBe(true);
    });

    it('works as deque', () => {
      const bv = new BiVector<number>();

      bv.AddHead(2); // [2]
      bv.AddHead(1); // [1, 2]
      bv.AddTail(3); // [1, 2, 3]
      bv.AddTail(4); // [1, 2, 3, 4]

      expect(bv.PopHead()).toBe(1); // [2, 3, 4]
      expect(bv.PopTail()).toBe(4); // [2, 3]
      expect(bv.Head()).toBe(2);
      expect(bv.Tail()).toBe(3);
      expect(bv.GetCount()).toBe(2);
    });
  });

  describe('Complex types', () => {
    interface Task {
      id: number;
      priority: number;
    }

    it('works with objects', () => {
      const bv = new BiVector<Task>();
      bv.AddTail({ id: 1, priority: 1 });
      bv.AddHead({ id: 2, priority: 2 });

      expect(bv.GetCount()).toBe(2);
      expect(bv.At(0).id).toBe(2);
      expect(bv.At(1).id).toBe(1);
    });
  });

  describe('Performance', () => {
    it('handles large datasets', () => {
      const bv = new BiVector<number>();
      const size = 1000;

      for (let i = 0; i < size; i++) {
        bv.AddTail(i);
      }

      expect(bv.GetCount()).toBe(size);
      expect(bv.Head()).toBe(0);
      expect(bv.Tail()).toBe(size - 1);
    });
  });
});
