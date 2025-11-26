import { One, Pick } from '../../src/Core/One';

describe('One', () => {
  describe('constructor', () => {
    it('creates empty One', () => {
      const one = new One<number>();
      expect(one.IsEmpty()).toBe(true);
      expect(one.HasValue()).toBe(false);
    });

    it('creates One with value', () => {
      const one = new One(42);
      expect(one.IsEmpty()).toBe(false);
      expect(one.HasValue()).toBe(true);
      expect(one.Get()).toBe(42);
    });

    it('creates empty One without arguments', () => {
      // One without arguments is empty
      const one = new One<number>();
      expect(one.IsEmpty()).toBe(true);
      expect(one.HasValue()).toBe(false);
    });
  });

  describe('Static Create', () => {
    it('creates One with value', () => {
      const one = One.Create(42);
      expect(one.Get()).toBe(42);
    });

    it('creates One with object', () => {
      const obj = { x: 10, y: 20 };
      const one = One.Create(obj);
      expect(one.Get()).toBe(obj);
      expect(one.Get().x).toBe(10);
    });
  });

  describe('Get operations', () => {
    it('gets value', () => {
      const one = new One(42);
      expect(one.Get()).toBe(42);
    });

    it('throws on Get from empty One', () => {
      const one = new One<number>();
      expect(() => one.Get()).toThrow('One<T> is empty');
    });

    it('gets value or default', () => {
      const one1 = new One(42);
      expect(one1.GetOr(99)).toBe(42);

      const one2 = new One<number>();
      expect(one2.GetOr(99)).toBe(99);
    });

    it('gets value or null', () => {
      const one1 = new One(42);
      expect(one1.GetOrNull()).toBe(42);

      const one2 = new One<number>();
      expect(one2.GetOrNull()).toBe(null);
    });
  });

  describe('Set and Clear', () => {
    it('sets value', () => {
      const one = new One<number>();
      expect(one.IsEmpty()).toBe(true);

      one.Set(42);
      expect(one.IsEmpty()).toBe(false);
      expect(one.Get()).toBe(42);
    });

    it('replaces existing value', () => {
      const one = new One(42);
      one.Set(99);
      expect(one.Get()).toBe(99);
    });

    it('clears value', () => {
      const one = new One(42);
      expect(one.HasValue()).toBe(true);

      one.Clear();
      expect(one.IsEmpty()).toBe(true);
      expect(() => one.Get()).toThrow();
    });
  });

  describe('Detach and Pick', () => {
    it('detaches value leaving empty', () => {
      const one = new One(42);
      const val = one.Detach();

      expect(val).toBe(42);
      expect(one.IsEmpty()).toBe(true);
    });

    it('picks value (same as detach)', () => {
      const one = new One(42);
      const val = one.Pick();

      expect(val).toBe(42);
      expect(one.IsEmpty()).toBe(true);
    });

    it('throws on detach from empty One', () => {
      const one = new One<number>();
      expect(() => one.Detach()).toThrow('One<T> is empty, cannot detach');
    });

    it('throws on pick from empty One', () => {
      const one = new One<number>();
      expect(() => one.Pick()).toThrow();
    });

    it('detaches object reference', () => {
      const obj = { x: 10, y: 20 };
      const one = new One(obj);
      const val = one.Detach();

      expect(val).toBe(obj);
      expect(val.x).toBe(10);
      expect(one.IsEmpty()).toBe(true);
    });
  });

  describe('Attach', () => {
    it('attaches value', () => {
      const one = new One<number>();
      one.Attach(42);

      expect(one.HasValue()).toBe(true);
      expect(one.Get()).toBe(42);
    });

    it('replaces existing value', () => {
      const one = new One(42);
      one.Attach(99);
      expect(one.Get()).toBe(99);
    });
  });

  describe('Swap', () => {
    it('swaps values between two Ones', () => {
      const one1 = new One(42);
      const one2 = new One(99);

      one1.Swap(one2);

      expect(one1.Get()).toBe(99);
      expect(one2.Get()).toBe(42);
    });

    it('swaps with empty One', () => {
      const one1 = new One(42);
      const one2 = new One<number>();

      one1.Swap(one2);

      expect(one1.IsEmpty()).toBe(true);
      expect(one2.Get()).toBe(42);
    });

    it('swaps two empty Ones', () => {
      const one1 = new One<number>();
      const one2 = new One<number>();

      one1.Swap(one2);

      expect(one1.IsEmpty()).toBe(true);
      expect(one2.IsEmpty()).toBe(true);
    });
  });

  describe('PickToOne', () => {
    it('transfers value to new One', () => {
      const one1 = new One(42);
      const one2 = one1.PickToOne();

      expect(one1.IsEmpty()).toBe(true);
      expect(one2.Get()).toBe(42);
    });

    it('throws when picking from empty', () => {
      const one = new One<number>();
      expect(() => one.PickToOne()).toThrow();
    });
  });

  describe('Map', () => {
    it('maps value to new type', () => {
      const one = new One(42);
      const mapped = one.Map(x => x * 2);

      expect(mapped.Get()).toBe(84);
      expect(one.Get()).toBe(42); // Original unchanged
    });

    it('maps to different type', () => {
      const one = new One(42);
      const mapped = one.Map(x => `Value: ${x}`);

      expect(mapped.Get()).toBe('Value: 42');
    });

    it('maps empty One to empty One', () => {
      const one = new One<number>();
      const mapped = one.Map(x => x * 2);

      expect(mapped.IsEmpty()).toBe(true);
    });
  });

  describe('FlatMap', () => {
    it('flat maps value', () => {
      const one = new One(42);
      const result = one.FlatMap(x => new One(x * 2));

      expect(result.Get()).toBe(84);
    });

    it('flat maps to empty', () => {
      const one = new One(42);
      const result = one.FlatMap(x => new One<number>());

      expect(result.IsEmpty()).toBe(true);
    });

    it('flat maps empty One', () => {
      const one = new One<number>();
      const result = one.FlatMap(x => new One(x * 2));

      expect(result.IsEmpty()).toBe(true);
    });
  });

  describe('IfPresent', () => {
    it('executes function if present', () => {
      const one = new One(42);
      let called = false;
      let value = 0;

      one.IfPresent(x => {
        called = true;
        value = x;
      });

      expect(called).toBe(true);
      expect(value).toBe(42);
    });

    it('does not execute function if empty', () => {
      const one = new One<number>();
      let called = false;

      one.IfPresent(x => {
        called = true;
      });

      expect(called).toBe(false);
    });
  });

  describe('Filter', () => {
    it('keeps value if predicate true', () => {
      const one = new One(42);
      const filtered = one.Filter(x => x > 40);

      expect(filtered.Get()).toBe(42);
    });

    it('returns empty if predicate false', () => {
      const one = new One(42);
      const filtered = one.Filter(x => x > 50);

      expect(filtered.IsEmpty()).toBe(true);
    });

    it('filters empty One', () => {
      const one = new One<number>();
      const filtered = one.Filter(x => x > 0);

      expect(filtered.IsEmpty()).toBe(true);
    });
  });

  describe('Equals', () => {
    it('compares two Ones with same value', () => {
      const one1 = new One(42);
      const one2 = new One(42);

      expect(one1.Equals(one2)).toBe(true);
    });

    it('compares two Ones with different values', () => {
      const one1 = new One(42);
      const one2 = new One(99);

      expect(one1.Equals(one2)).toBe(false);
    });

    it('compares two empty Ones', () => {
      const one1 = new One<number>();
      const one2 = new One<number>();

      expect(one1.Equals(one2)).toBe(true);
    });

    it('compares empty with non-empty', () => {
      const one1 = new One(42);
      const one2 = new One<number>();

      expect(one1.Equals(one2)).toBe(false);
      expect(one2.Equals(one1)).toBe(false);
    });
  });

  describe('Clone', () => {
    it('clones primitive value', () => {
      const one = new One(42);
      const cloned = one.Clone();

      expect(cloned.Get()).toBe(42);
      expect(one.Get()).toBe(42); // Original unchanged
    });

    it('clones empty One', () => {
      const one = new One<number>();
      const cloned = one.Clone();

      expect(cloned.IsEmpty()).toBe(true);
    });

    it('clones string', () => {
      const one = new One('hello');
      const cloned = one.Clone();

      expect(cloned.Get()).toBe('hello');
    });

    it('clones plain object via structuredClone', () => {
      const one = new One({ x: 10, y: 20 });
      const cloned = one.Clone();

      expect(cloned.Get()).toEqual({ x: 10, y: 20 });
      expect(cloned.Get()).not.toBe(one.Get()); // Different references
    });

    it('clones object with Clone method', () => {
      class Cloneable {
        constructor(public value: number) {}
        Clone(): Cloneable {
          return new Cloneable(this.value);
        }
      }

      const one = new One(new Cloneable(42));
      const cloned = one.Clone();

      expect(cloned.Get().value).toBe(42);
      expect(cloned.Get()).not.toBe(one.Get());
    });
  });

  describe('toString', () => {
    it('shows value in string representation', () => {
      const one = new One(42);
      expect(one.toString()).toBe('One(42)');
    });

    it('shows empty in string representation', () => {
      const one = new One<number>();
      expect(one.toString()).toBe('One(empty)');
    });
  });

  describe('Pick helper function', () => {
    it('picks value from One', () => {
      const one = new One(42);
      const val = Pick(one);

      expect(val).toBe(42);
      expect(one.IsEmpty()).toBe(true);
    });

    it('throws when picking from empty', () => {
      const one = new One<number>();
      expect(() => Pick(one)).toThrow();
    });
  });

  describe('Ownership semantics', () => {
    it('demonstrates unique ownership', () => {
      const one = new One(42);

      // Can get value
      expect(one.Get()).toBe(42);

      // Pick transfers ownership
      const val = one.Pick();
      expect(val).toBe(42);
      expect(one.IsEmpty()).toBe(true);

      // Cannot get after pick
      expect(() => one.Get()).toThrow();
    });

    it('demonstrates transfer between Ones', () => {
      const one1 = new One(42);
      const one2 = new One<number>();

      // Transfer via pick and attach
      const val = one1.Pick();
      one2.Attach(val);

      expect(one1.IsEmpty()).toBe(true);
      expect(one2.Get()).toBe(42);
    });

    it('demonstrates swap ownership', () => {
      const one1 = new One({ id: 1 });
      const one2 = new One({ id: 2 });

      one1.Swap(one2);

      expect(one1.Get().id).toBe(2);
      expect(one2.Get().id).toBe(1);
    });
  });

  describe('Complex types', () => {
    it('works with arrays', () => {
      const one = new One([1, 2, 3]);
      expect(one.Get()).toEqual([1, 2, 3]);

      const picked = one.Pick();
      expect(picked).toEqual([1, 2, 3]);
      expect(one.IsEmpty()).toBe(true);
    });

    it('works with objects', () => {
      interface User {
        name: string;
        age: number;
      }

      const one = new One<User>({ name: 'Alice', age: 30 });
      expect(one.Get().name).toBe('Alice');
      expect(one.Get().age).toBe(30);
    });

    it('works with nested Ones', () => {
      const inner = new One(42);
      const outer = new One(inner);

      expect(outer.Get().Get()).toBe(42);

      const pickedInner = outer.Pick();
      expect(pickedInner.Get()).toBe(42);
      expect(outer.IsEmpty()).toBe(true);
    });
  });

  describe('Edge cases', () => {
    it('handles zero values', () => {
      const one = new One(0);
      expect(one.HasValue()).toBe(true);
      expect(one.Get()).toBe(0);
    });

    it('handles false values', () => {
      const one = new One(false);
      expect(one.HasValue()).toBe(true);
      expect(one.Get()).toBe(false);
    });

    it('handles empty string', () => {
      const one = new One('');
      expect(one.HasValue()).toBe(true);
      expect(one.Get()).toBe('');
    });

    it('handles multiple operations', () => {
      const one = new One(42);

      one.Set(99);
      expect(one.Get()).toBe(99);

      one.Clear();
      expect(one.IsEmpty()).toBe(true);

      one.Attach(77);
      expect(one.Get()).toBe(77);

      const val = one.Detach();
      expect(val).toBe(77);
      expect(one.IsEmpty()).toBe(true);
    });
  });
});
