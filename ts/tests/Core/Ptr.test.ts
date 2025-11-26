import { Ptr, WeakPtr } from '../../src/Core/Ptr';

describe('Ptr', () => {
  describe('constructor', () => {
    it('creates null Ptr', () => {
      const ptr = new Ptr<number>();
      expect(ptr.IsNull()).toBe(true);
      expect(ptr.HasValue()).toBe(false);
    });

    it('creates Ptr with value', () => {
      const ptr = new Ptr(42);
      expect(ptr.IsNull()).toBe(false);
      expect(ptr.HasValue()).toBe(true);
      expect(ptr.Get()).toBe(42);
    });

    it('creates Ptr with object', () => {
      const obj = { x: 10, y: 20 };
      const ptr = new Ptr(obj);
      expect(ptr.Get()).toBe(obj);
    });
  });

  describe('Static Create and Null', () => {
    it('creates Ptr with value', () => {
      const ptr = Ptr.Create(42);
      expect(ptr.Get()).toBe(42);
    });

    it('creates null Ptr', () => {
      const ptr = Ptr.Null<number>();
      expect(ptr.IsNull()).toBe(true);
    });
  });

  describe('Get operations', () => {
    it('gets value', () => {
      const ptr = new Ptr(42);
      expect(ptr.Get()).toBe(42);
    });

    it('throws on Get from null Ptr', () => {
      const ptr = new Ptr<number>();
      expect(() => ptr.Get()).toThrow('Ptr<T> is null');
    });

    it('gets value or default', () => {
      const ptr1 = new Ptr(42);
      expect(ptr1.GetOr(99)).toBe(42);

      const ptr2 = new Ptr<number>();
      expect(ptr2.GetOr(99)).toBe(99);
    });

    it('gets value or null', () => {
      const ptr1 = new Ptr(42);
      expect(ptr1.GetOrNull()).toBe(42);

      const ptr2 = new Ptr<number>();
      expect(ptr2.GetOrNull()).toBe(null);
    });
  });

  describe('Set and Clear', () => {
    it('sets value', () => {
      const ptr = new Ptr<number>();
      expect(ptr.IsNull()).toBe(true);

      ptr.Set(42);
      expect(ptr.IsNull()).toBe(false);
      expect(ptr.Get()).toBe(42);
    });

    it('replaces existing value', () => {
      const ptr = new Ptr(42);
      ptr.Set(99);
      expect(ptr.Get()).toBe(99);
    });

    it('clears value', () => {
      const ptr = new Ptr(42);
      expect(ptr.HasValue()).toBe(true);

      ptr.Clear();
      expect(ptr.IsNull()).toBe(true);
    });
  });

  describe('Reference counting', () => {
    it('starts with ref count 1', () => {
      const ptr = new Ptr(42);
      expect(ptr.GetRefCount()).toBe(1);
    });

    it('increments ref count on Attach', () => {
      const ptr1 = new Ptr(42);
      const ptr2 = new Ptr<number>();

      expect(ptr1.GetRefCount()).toBe(1);

      ptr2.Attach(ptr1);
      expect(ptr1.GetRefCount()).toBe(2);
      expect(ptr2.GetRefCount()).toBe(2);
    });

    it('decrements ref count on Clear', () => {
      const ptr1 = new Ptr(42);
      const ptr2 = new Ptr<number>();
      ptr2.Attach(ptr1);

      expect(ptr1.GetRefCount()).toBe(2);

      ptr2.Clear();
      expect(ptr1.GetRefCount()).toBe(1);
      expect(ptr2.GetRefCount()).toBe(0);
    });

    it('shares reference through Attach', () => {
      const ptr1 = new Ptr({ x: 10 });
      const ptr2 = new Ptr<{ x: number }>();
      ptr2.Attach(ptr1);

      // Both point to same object
      expect(ptr1.Get()).toBe(ptr2.Get());

      // Mutation through one affects the other
      ptr1.Get().x = 20;
      expect(ptr2.Get().x).toBe(20);
    });
  });

  describe('Share', () => {
    it('creates shared Ptr', () => {
      const ptr1 = new Ptr(42);
      const ptr2 = ptr1.Share();

      expect(ptr1.GetRefCount()).toBe(2);
      expect(ptr2.GetRefCount()).toBe(2);
      expect(ptr1.Get()).toBe(42);
      expect(ptr2.Get()).toBe(42);
    });

    it('shares object reference', () => {
      const ptr1 = new Ptr({ x: 10 });
      const ptr2 = ptr1.Share();

      ptr1.Get().x = 20;
      expect(ptr2.Get().x).toBe(20);
    });
  });

  describe('Detach', () => {
    it('detaches value and clears Ptr', () => {
      const ptr = new Ptr(42);
      const val = ptr.Detach();

      expect(val).toBe(42);
      expect(ptr.IsNull()).toBe(true);
    });

    it('throws on detach from null', () => {
      const ptr = new Ptr<number>();
      expect(() => ptr.Detach()).toThrow();
    });
  });

  describe('Equals', () => {
    it('compares reference equality', () => {
      const ptr1 = new Ptr(42);
      const ptr2 = ptr1.Share();
      const ptr3 = new Ptr(42);

      expect(ptr1.Equals(ptr2)).toBe(true); // Same reference
      expect(ptr1.Equals(ptr3)).toBe(false); // Different reference
    });

    it('compares null Ptrs', () => {
      const ptr1 = new Ptr<number>();
      const ptr2 = new Ptr<number>();

      expect(ptr1.Equals(ptr2)).toBe(true); // Both null
    });
  });

  describe('ValueEquals', () => {
    it('compares value equality', () => {
      const ptr1 = new Ptr(42);
      const ptr2 = new Ptr(42);
      const ptr3 = new Ptr(99);

      expect(ptr1.ValueEquals(ptr2)).toBe(true); // Same value
      expect(ptr1.ValueEquals(ptr3)).toBe(false); // Different value
    });

    it('compares null Ptrs', () => {
      const ptr1 = new Ptr<number>();
      const ptr2 = new Ptr<number>();

      expect(ptr1.ValueEquals(ptr2)).toBe(true);
    });

    it('compares null with non-null', () => {
      const ptr1 = new Ptr(42);
      const ptr2 = new Ptr<number>();

      expect(ptr1.ValueEquals(ptr2)).toBe(false);
    });
  });

  describe('Arrow', () => {
    it('provides member access', () => {
      const ptr = new Ptr({ x: 10, y: 20 });
      const obj = ptr.Arrow();

      expect(obj.x).toBe(10);
      expect(obj.y).toBe(20);
    });
  });

  describe('toString', () => {
    it('shows value and ref count', () => {
      const ptr = new Ptr(42);
      expect(ptr.toString()).toBe('Ptr(42, refs: 1)');
    });

    it('shows null', () => {
      const ptr = new Ptr<number>();
      expect(ptr.toString()).toBe('Ptr(null)');
    });

    it('shows ref count after sharing', () => {
      const ptr1 = new Ptr(42);
      const ptr2 = ptr1.Share();
      expect(ptr1.toString()).toContain('refs: 2');
    });
  });

  describe('Shared ownership', () => {
    it('demonstrates shared reference', () => {
      const obj = { count: 0 };
      const ptr1 = new Ptr(obj);
      const ptr2 = ptr1.Share();
      const ptr3 = ptr2.Share();

      expect(ptr1.GetRefCount()).toBe(3);
      expect(ptr2.GetRefCount()).toBe(3);
      expect(ptr3.GetRefCount()).toBe(3);

      // All point to same object
      ptr1.Get().count++;
      expect(ptr2.Get().count).toBe(1);
      expect(ptr3.Get().count).toBe(1);

      // Clear one
      ptr2.Clear();
      expect(ptr1.GetRefCount()).toBe(2);
      expect(ptr3.GetRefCount()).toBe(2);

      // Still shared
      ptr1.Get().count++;
      expect(ptr3.Get().count).toBe(2);
    });
  });

  describe('ToWeak', () => {
    it('creates weak reference', () => {
      const ptr = new Ptr(42);
      const weak = ptr.ToWeak();

      expect(weak.IsExpired()).toBe(false);
      expect(weak.UseCount()).toBe(1);
      expect(ptr.GetWeakCount()).toBe(1);
    });
  });

  describe('Complex types', () => {
    it('works with arrays', () => {
      const ptr = new Ptr([1, 2, 3]);
      expect(ptr.Get()).toEqual([1, 2, 3]);

      const shared = ptr.Share();
      ptr.Get().push(4);
      expect(shared.Get()).toEqual([1, 2, 3, 4]);
    });

    it('works with nested objects', () => {
      interface User {
        name: string;
        address: { city: string };
      }

      const ptr = new Ptr<User>({
        name: 'Alice',
        address: { city: 'NYC' },
      });

      expect(ptr.Get().name).toBe('Alice');
      expect(ptr.Get().address.city).toBe('NYC');
    });
  });

  describe('Edge cases', () => {
    it('handles zero values', () => {
      const ptr = new Ptr(0);
      expect(ptr.HasValue()).toBe(true);
      expect(ptr.Get()).toBe(0);
    });

    it('handles false values', () => {
      const ptr = new Ptr(false);
      expect(ptr.HasValue()).toBe(true);
      expect(ptr.Get()).toBe(false);
    });

    it('handles empty string', () => {
      const ptr = new Ptr('');
      expect(ptr.HasValue()).toBe(true);
      expect(ptr.Get()).toBe('');
    });

    it('handles Attach to self', () => {
      const ptr = new Ptr(42);
      const refCount = ptr.GetRefCount();

      ptr.Attach(ptr); // Should be no-op
      expect(ptr.GetRefCount()).toBe(refCount);
    });
  });
});

describe('WeakPtr', () => {
  describe('constructor', () => {
    it('creates from Ptr', () => {
      const ptr = new Ptr(42);
      const weak = new WeakPtr(ptr);

      expect(weak.IsExpired()).toBe(false);
      expect(weak.UseCount()).toBe(1);
    });

    it('creates empty WeakPtr', () => {
      const weak = new WeakPtr<number>();
      expect(weak.IsExpired()).toBe(true);
    });
  });

  describe('IsExpired', () => {
    it('returns false when Ptr exists', () => {
      const ptr = new Ptr(42);
      const weak = new WeakPtr(ptr);

      expect(weak.IsExpired()).toBe(false);
    });

    it('returns true when Ptr is cleared', () => {
      const ptr = new Ptr(42);
      const weak = new WeakPtr(ptr);

      ptr.Clear();
      expect(weak.IsExpired()).toBe(true);
    });

    it('returns true for empty WeakPtr', () => {
      const weak = new WeakPtr<number>();
      expect(weak.IsExpired()).toBe(true);
    });
  });

  describe('Lock', () => {
    it('upgrades to Ptr when valid', () => {
      const ptr = new Ptr(42);
      const weak = new WeakPtr(ptr);

      const locked = weak.Lock();
      expect(locked.HasValue()).toBe(true);
      expect(locked.Get()).toBe(42);
    });

    it('returns null Ptr when expired', () => {
      const ptr = new Ptr(42);
      const weak = new WeakPtr(ptr);

      ptr.Clear();

      const locked = weak.Lock();
      expect(locked.IsNull()).toBe(true);
    });

    it('increments ref count on Lock', () => {
      const ptr = new Ptr(42);
      const weak = new WeakPtr(ptr);

      expect(ptr.GetRefCount()).toBe(1);

      const locked = weak.Lock();
      expect(ptr.GetRefCount()).toBe(2);
      expect(locked.GetRefCount()).toBe(2);
    });
  });

  describe('UseCount', () => {
    it('returns strong reference count', () => {
      const ptr1 = new Ptr(42);
      const weak = new WeakPtr(ptr1);

      expect(weak.UseCount()).toBe(1);

      const ptr2 = ptr1.Share();
      expect(weak.UseCount()).toBe(2);

      ptr2.Clear();
      expect(weak.UseCount()).toBe(1);

      ptr1.Clear();
      expect(weak.UseCount()).toBe(0);
    });
  });

  describe('Clear', () => {
    it('clears weak reference', () => {
      const ptr = new Ptr(42);
      const weak = new WeakPtr(ptr);

      expect(ptr.GetWeakCount()).toBe(1);

      weak.Clear();
      expect(ptr.GetWeakCount()).toBe(0);
      expect(weak.IsExpired()).toBe(true);
    });
  });

  describe('toString', () => {
    it('shows ref count when valid', () => {
      const ptr = new Ptr(42);
      const weak = new WeakPtr(ptr);

      expect(weak.toString()).toBe('WeakPtr(refs: 1)');
    });

    it('shows expired', () => {
      const ptr = new Ptr(42);
      const weak = new WeakPtr(ptr);

      ptr.Clear();
      expect(weak.toString()).toBe('WeakPtr(expired)');
    });
  });

  describe('Weak reference semantics', () => {
    it('does not prevent object destruction', () => {
      const ptr = new Ptr({ x: 10 });
      const weak = new WeakPtr(ptr);

      expect(weak.IsExpired()).toBe(false);

      // Clear all strong references
      ptr.Clear();

      // Weak reference is now expired
      expect(weak.IsExpired()).toBe(true);
      expect(weak.Lock().IsNull()).toBe(true);
    });

    it('multiple weak references', () => {
      const ptr = new Ptr(42);
      const weak1 = new WeakPtr(ptr);
      const weak2 = new WeakPtr(ptr);

      expect(ptr.GetWeakCount()).toBe(2);
      expect(weak1.UseCount()).toBe(1);
      expect(weak2.UseCount()).toBe(1);

      weak1.Clear();
      expect(ptr.GetWeakCount()).toBe(1);

      ptr.Clear();
      expect(weak2.IsExpired()).toBe(true);
    });

    it('weak reference survives temporary Ptr', () => {
      const ptr = new Ptr(42);
      const weak = new WeakPtr(ptr);

      // Lock creates temporary Ptr
      const locked1 = weak.Lock();
      expect(locked1.Get()).toBe(42);

      // After locked1 goes out of scope (cleared), weak still valid
      locked1.Clear();
      expect(weak.IsExpired()).toBe(false);
      expect(ptr.HasValue()).toBe(true);

      // Can lock again
      const locked2 = weak.Lock();
      expect(locked2.Get()).toBe(42);
    });
  });

  describe('Complex scenarios', () => {
    it('demonstrates cache with weak references', () => {
      // Simulating a cache where weak references don't keep objects alive
      const cache: Map<string, WeakPtr<{ data: string }>> = new Map();

      // Add item
      const ptr1 = new Ptr({ data: 'value1' });
      cache.set('key1', new WeakPtr(ptr1));

      // Can retrieve while Ptr exists
      {
        const cached = cache.get('key1')!.Lock();
        expect(cached.Get().data).toBe('value1');
        cached.Clear(); // Clear the temporary Ptr
      }

      // Clear strong reference
      ptr1.Clear();

      // Weak reference expired
      expect(cache.get('key1')!.IsExpired()).toBe(true);
      expect(cache.get('key1')!.Lock().IsNull()).toBe(true);
    });

    it('demonstrates observer pattern', () => {
      // Subject holds strong reference, observers hold weak references
      const subject = new Ptr({ value: 0 });
      const observer1 = new WeakPtr(subject);
      const observer2 = new WeakPtr(subject);

      // Observers can access subject
      const obj1 = observer1.Lock();
      obj1.Get().value = 10;

      const obj2 = observer2.Lock();
      expect(obj2.Get().value).toBe(10);

      // Subject destroyed
      subject.Clear();
      obj1.Clear();
      obj2.Clear();

      // Observers now expired
      expect(observer1.IsExpired()).toBe(true);
      expect(observer2.IsExpired()).toBe(true);
    });
  });
});
