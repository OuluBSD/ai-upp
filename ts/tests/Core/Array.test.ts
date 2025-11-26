import { Array } from '../../src/Core/Array';

interface TestObject {
  id: number;
  name: string;
}

describe('Array', () => {
  describe('constructor', () => {
    it('creates empty array', () => {
      const arr = new Array<TestObject>();
      expect(arr.GetCount()).toBe(0);
      expect(arr.IsEmpty()).toBe(true);
    });
  });

  describe('Create', () => {
    it('creates element using factory', () => {
      const arr = new Array<TestObject>();
      const obj = arr.Create(() => ({ id: 1, name: 'test' }));
      expect(arr.GetCount()).toBe(1);
      expect(obj.id).toBe(1);
      expect(obj.name).toBe('test');
    });

    it('returns reference to created element', () => {
      const arr = new Array<TestObject>();
      const obj = arr.Create(() => ({ id: 1, name: 'test' }));
      obj.name = 'modified';
      expect(arr.At(0).name).toBe('modified');
    });
  });

  describe('Add', () => {
    it('adds element to end', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'first' });
      arr.Add({ id: 2, name: 'second' });
      expect(arr.GetCount()).toBe(2);
      expect(arr.At(0).id).toBe(1);
      expect(arr.At(1).id).toBe(2);
    });

    it('returns reference to added element', () => {
      const arr = new Array<TestObject>();
      const obj = arr.Add({ id: 1, name: 'test' });
      expect(obj.id).toBe(1);
    });
  });

  describe('At and Set', () => {
    it('gets element at valid index', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'first' });
      arr.Add({ id: 2, name: 'second' });
      expect(arr.At(0).name).toBe('first');
      expect(arr.At(1).name).toBe('second');
    });

    it('throws on out of bounds access', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'test' });
      expect(() => arr.At(-1)).toThrow('Array index out of bounds');
      expect(() => arr.At(1)).toThrow('Array index out of bounds');
    });

    it('sets element at valid index', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'first' });
      arr.Set(0, { id: 99, name: 'replaced' });
      expect(arr.At(0).id).toBe(99);
    });

    it('throws when setting out of bounds', () => {
      const arr = new Array<TestObject>();
      expect(() => arr.Set(0, { id: 1, name: 'test' })).toThrow(
        'Array index out of bounds'
      );
    });
  });

  describe('GetCount and IsEmpty', () => {
    it('returns correct count', () => {
      const arr = new Array<TestObject>();
      expect(arr.GetCount()).toBe(0);
      arr.Add({ id: 1, name: 'test' });
      expect(arr.GetCount()).toBe(1);
    });

    it('reports empty status correctly', () => {
      const arr = new Array<TestObject>();
      expect(arr.IsEmpty()).toBe(true);
      arr.Add({ id: 1, name: 'test' });
      expect(arr.IsEmpty()).toBe(false);
      arr.Clear();
      expect(arr.IsEmpty()).toBe(true);
    });
  });

  describe('Clear', () => {
    it('removes all elements', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'first' });
      arr.Add({ id: 2, name: 'second' });
      arr.Clear();
      expect(arr.GetCount()).toBe(0);
      expect(arr.IsEmpty()).toBe(true);
    });
  });

  describe('Shrink', () => {
    it('reduces array size', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'first' });
      arr.Add({ id: 2, name: 'second' });
      arr.Add({ id: 3, name: 'third' });
      arr.Shrink(1);
      expect(arr.GetCount()).toBe(1);
      expect(arr.At(0).id).toBe(1);
    });

    it('does nothing if size already smaller', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'test' });
      arr.Shrink(10);
      expect(arr.GetCount()).toBe(1);
    });
  });

  describe('SetCount', () => {
    it('shrinks when new size is smaller', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'first' });
      arr.Add({ id: 2, name: 'second' });
      arr.SetCount(1);
      expect(arr.GetCount()).toBe(1);
    });

    it('grows when new size is larger', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'test' });
      arr.SetCount(3);
      expect(arr.GetCount()).toBe(3);
    });
  });

  describe('Remove', () => {
    it('removes element at index', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'first' });
      arr.Add({ id: 2, name: 'second' });
      arr.Add({ id: 3, name: 'third' });
      arr.Remove(1);
      expect(arr.GetCount()).toBe(2);
      expect(arr.At(0).id).toBe(1);
      expect(arr.At(1).id).toBe(3);
    });

    it('throws on invalid index', () => {
      const arr = new Array<TestObject>();
      expect(() => arr.Remove(0)).toThrow('Array index out of bounds');
    });
  });

  describe('RemoveRange', () => {
    it('removes multiple elements', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'first' });
      arr.Add({ id: 2, name: 'second' });
      arr.Add({ id: 3, name: 'third' });
      arr.Add({ id: 4, name: 'fourth' });
      arr.RemoveRange(1, 2);
      expect(arr.GetCount()).toBe(2);
      expect(arr.At(0).id).toBe(1);
      expect(arr.At(1).id).toBe(4);
    });
  });

  describe('Insert', () => {
    it('inserts at beginning', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 2, name: 'second' });
      arr.Insert(0, { id: 1, name: 'first' });
      expect(arr.GetCount()).toBe(2);
      expect(arr.At(0).id).toBe(1);
    });

    it('inserts at end', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'first' });
      arr.Insert(1, { id: 2, name: 'second' });
      expect(arr.GetCount()).toBe(2);
      expect(arr.At(1).id).toBe(2);
    });

    it('inserts in middle', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'first' });
      arr.Add({ id: 3, name: 'third' });
      arr.Insert(1, { id: 2, name: 'second' });
      expect(arr.GetCount()).toBe(3);
      expect(arr.At(1).id).toBe(2);
    });
  });

  describe('Top', () => {
    it('returns last element', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'first' });
      arr.Add({ id: 2, name: 'second' });
      expect(arr.Top().id).toBe(2);
    });

    it('throws on empty array', () => {
      const arr = new Array<TestObject>();
      expect(() => arr.Top()).toThrow('Array is empty');
    });
  });

  describe('Pop', () => {
    it('removes and returns last element', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'first' });
      arr.Add({ id: 2, name: 'second' });
      const last = arr.Pop();
      expect(last.id).toBe(2);
      expect(arr.GetCount()).toBe(1);
    });

    it('throws on empty array', () => {
      const arr = new Array<TestObject>();
      expect(() => arr.Pop()).toThrow('Array is empty');
    });
  });

  describe('Drop', () => {
    it('removes last element', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'first' });
      arr.Add({ id: 2, name: 'second' });
      arr.Drop();
      expect(arr.GetCount()).toBe(1);
    });

    it('does nothing on empty array', () => {
      const arr = new Array<TestObject>();
      arr.Drop();
      expect(arr.GetCount()).toBe(0);
    });
  });

  describe('Find', () => {
    it('finds element by predicate', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'first' });
      arr.Add({ id: 2, name: 'second' });
      const idx = arr.Find((x) => x.id === 2);
      expect(idx).toBe(1);
    });

    it('returns -1 when not found', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'test' });
      const idx = arr.Find((x) => x.id === 99);
      expect(idx).toBe(-1);
    });
  });

  describe('Detach', () => {
    it('removes and returns element', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'first' });
      arr.Add({ id: 2, name: 'second' });
      const detached = arr.Detach(0);
      expect(detached.id).toBe(1);
      expect(arr.GetCount()).toBe(1);
      expect(arr.At(0).id).toBe(2);
    });

    it('throws on invalid index', () => {
      const arr = new Array<TestObject>();
      expect(() => arr.Detach(0)).toThrow('Array index out of bounds');
    });
  });

  describe('Swap', () => {
    it('swaps two elements', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'first' });
      arr.Add({ id: 2, name: 'second' });
      arr.Swap(0, 1);
      expect(arr.At(0).id).toBe(2);
      expect(arr.At(1).id).toBe(1);
    });

    it('throws on invalid indices', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'test' });
      expect(() => arr.Swap(0, 5)).toThrow('Array index out of bounds');
    });
  });

  describe('Static From', () => {
    it('creates array from elements', () => {
      const objects = [
        { id: 1, name: 'first' },
        { id: 2, name: 'second' },
      ];
      const arr = Array.From(objects);
      expect(arr.GetCount()).toBe(2);
      expect(arr.At(0).id).toBe(1);
    });
  });

  describe('Iterator', () => {
    it('supports for-of iteration', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'first' });
      arr.Add({ id: 2, name: 'second' });

      const ids: number[] = [];
      for (const obj of arr) {
        ids.push(obj.id);
      }

      expect(ids).toEqual([1, 2]);
    });
  });

  describe('toString', () => {
    it('returns string representation', () => {
      const arr = new Array<TestObject>();
      arr.Add({ id: 1, name: 'test' });
      const str = arr.toString();
      expect(str).toContain('Array');
      expect(str).toContain('1');
    });
  });

  describe('Reference semantics', () => {
    it('maintains references to objects', () => {
      const arr = new Array<TestObject>();
      const obj = { id: 1, name: 'test' };
      arr.Add(obj);

      // Modify original object
      obj.name = 'modified';

      // Array should see the change
      expect(arr.At(0).name).toBe('modified');
    });

    it('Create returns mutable reference', () => {
      const arr = new Array<TestObject>();
      const obj = arr.Create(() => ({ id: 1, name: 'test' }));

      // Modify through reference
      obj.name = 'modified';

      // Array should see the change
      expect(arr.At(0).name).toBe('modified');
    });
  });
});
