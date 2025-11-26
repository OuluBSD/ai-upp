# Ownership Patterns in uppts

This document describes the ownership and memory management patterns available in uppts, particularly for Phase 4 smart pointers.

## Overview

uppts provides several smart pointer types that implement different ownership semantics:

1. **One<T>** - Unique ownership (like `std::unique_ptr` or Rust's ownership)
2. **Ptr<T>** - Shared ownership with reference counting (like `std::shared_ptr`)
3. **WeakPtr<T>** - Weak reference that doesn't affect ownership (like `std::weak_ptr`)

## One<T> - Unique Ownership

`One<T>` represents unique ownership of a value. Only one `One` can own a value at any time.

### Key Features

- **Unique ownership**: Value is owned by exactly one `One` instance
- **Move semantics**: Value can be transferred via `Pick()` or `Detach()`
- **Null-safe**: Provides safe access with `Get()`, `GetOr()`, etc.
- **No copying**: Values must be explicitly moved

### Basic Usage

```typescript
import { One, Pick } from '@uppts/core';

// Create One with value
const one = new One(42);
console.log(one.Get()); // 42

// Transfer ownership
const moved = one.PickToOne();
console.log(one.IsEmpty()); // true
console.log(moved.Get()); // 42

// Extract value (leaving empty)
const value = moved.Pick();
console.log(moved.IsEmpty()); // true
console.log(value); // 42
```

### Common Patterns

#### Resource Management

```typescript
class Resource {
  constructor(public name: string) {
    console.log(`Resource ${name} created`);
  }
}

function useResource(resource: One<Resource>): void {
  const r = resource.Get();
  console.log(`Using ${r.name}`);
  // resource still owned by caller
}

const res = new One(new Resource('database'));
useResource(res);
res.Clear(); // Clean up
```

#### Transfer of Ownership

```typescript
function createResource(): One<Resource> {
  return new One(new Resource('temp'));
}

function consumeResource(resource: One<Resource>): void {
  const r = resource.Pick(); // Take ownership
  console.log(`Consumed ${r.name}`);
  // resource is now empty
}

const res = createResource();
consumeResource(res);
console.log(res.IsEmpty()); // true
```

#### Optional Values

```typescript
function findUser(id: number): One<User> {
  if (id === 1) {
    return new One({ id: 1, name: 'Alice' });
  }
  return new One(); // Empty
}

const user = findUser(1);
if (user.HasValue()) {
  console.log(user.Get().name);
} else {
  console.log('User not found');
}
```

### Advanced Operations

#### Map and FlatMap

```typescript
const one = new One(42);

// Transform value
const doubled = one.Map(x => x * 2);
console.log(doubled.Get()); // 84

// Chain operations
const result = one
  .Map(x => x + 10)
  .Filter(x => x > 50)
  .Map(x => `Value: ${x}`);
```

#### Clone

```typescript
// Clone primitive values
const one = new One(42);
const cloned = one.Clone();
console.log(cloned.Get()); // 42

// Clone objects with Clone method
class MyClass {
  constructor(public value: number) {}
  Clone(): MyClass {
    return new MyClass(this.value);
  }
}

const obj = new One(new MyClass(42));
const objCloned = obj.Clone();
```

## Ptr<T> - Shared Ownership

`Ptr<T>` represents shared ownership with reference counting. Multiple `Ptr` instances can reference the same object.

### Key Features

- **Shared ownership**: Multiple Ptrs can reference the same value
- **Reference counting**: Automatically tracks how many Ptrs reference a value
- **Shared mutations**: Changes through one Ptr visible to all
- **Weak reference support**: Can create WeakPtr for non-owning references

### Basic Usage

```typescript
import { Ptr } from '@uppts/core';

// Create Ptr with value
const ptr1 = new Ptr({ x: 10, y: 20 });

// Share the reference
const ptr2 = ptr1.Share();
console.log(ptr1.GetRefCount()); // 2

// Both point to same object
ptr1.Get().x = 30;
console.log(ptr2.Get().x); // 30

// Clear one reference
ptr2.Clear();
console.log(ptr1.GetRefCount()); // 1
```

### Common Patterns

#### Shared Resources

```typescript
class DatabaseConnection {
  constructor(public connectionString: string) {}

  query(sql: string): any[] {
    console.log(`Executing: ${sql}`);
    return [];
  }
}

// Multiple components share same connection
const db = new Ptr(new DatabaseConnection('localhost:5432'));

function queryUsers(connection: Ptr<DatabaseConnection>) {
  const shared = connection.Share();
  return shared.Get().query('SELECT * FROM users');
}

function queryOrders(connection: Ptr<DatabaseConnection>) {
  const shared = connection.Share();
  return shared.Get().query('SELECT * FROM orders');
}

queryUsers(db);
queryOrders(db);
// db automatically cleaned up when all references cleared
```

#### Observer Pattern

```typescript
class Subject {
  private observers: Ptr<Observer>[] = [];

  attach(observer: Ptr<Observer>): void {
    this.observers.push(observer.Share());
  }

  notify(): void {
    for (const obs of this.observers) {
      if (obs.HasValue()) {
        obs.Get().update();
      }
    }
  }
}
```

#### Cache with Reference Counting

```typescript
class Cache<K, V> {
  private data = new Map<K, Ptr<V>>();

  get(key: K): Ptr<V> | null {
    const ptr = this.data.get(key);
    return ptr ? ptr.Share() : null;
  }

  set(key: K, value: V): void {
    this.data.set(key, new Ptr(value));
  }

  getRefCount(key: K): number {
    const ptr = this.data.get(key);
    return ptr ? ptr.GetRefCount() : 0;
  }
}
```

## WeakPtr<T> - Weak References

`WeakPtr<T>` holds a weak reference that doesn't affect reference counting. The referenced object may be deleted even if WeakPtr exists.

### Key Features

- **Non-owning**: Doesn't prevent value from being destroyed
- **Safe access**: Must "lock" to access (upgrade to Ptr)
- **Expiration detection**: Can check if reference is still valid
- **No cycles**: Breaks reference cycles

### Basic Usage

```typescript
import { Ptr, WeakPtr } from '@uppts/core';

const ptr = new Ptr({ data: 'hello' });
const weak = ptr.ToWeak();

console.log(weak.IsExpired()); // false
console.log(weak.UseCount()); // 1

// Access via Lock
const locked = weak.Lock();
if (locked.HasValue()) {
  console.log(locked.Get().data); // 'hello'
}

// Clear strong reference
ptr.Clear();
locked.Clear();

// Weak reference now expired
console.log(weak.IsExpired()); // true
```

### Common Patterns

#### Cache with Weak References

```typescript
class WeakCache<K, V> {
  private cache = new Map<K, WeakPtr<V>>();

  set(key: K, ptr: Ptr<V>): void {
    this.cache.set(key, ptr.ToWeak());
  }

  get(key: K): Ptr<V> | null {
    const weak = this.cache.get(key);
    if (!weak) return null;

    if (weak.IsExpired()) {
      this.cache.delete(key); // Clean up expired entry
      return null;
    }

    return weak.Lock();
  }
}
```

#### Parent-Child Relationships

```typescript
class Node {
  public parent: WeakPtr<Node> | null = null;
  public children: Ptr<Node>[] = [];

  constructor(public value: number) {}

  addChild(child: Ptr<Node>): void {
    this.children.push(child.Share());
    child.Get().parent = new WeakPtr(new Ptr(this));
  }

  getParent(): Ptr<Node> | null {
    if (!this.parent || this.parent.IsExpired()) {
      return null;
    }
    return this.parent.Lock();
  }
}
```

#### Observer Pattern (Breaking Cycles)

```typescript
class Observable {
  private observers: WeakPtr<Observer>[] = [];

  attach(observer: Ptr<Observer>): void {
    this.observers.push(observer.ToWeak());
  }

  notify(): void {
    // Clean up expired observers
    this.observers = this.observers.filter(weak => !weak.IsExpired());

    // Notify remaining observers
    for (const weak of this.observers) {
      const obs = weak.Lock();
      if (obs.HasValue()) {
        obs.Get().update();
      }
    }
  }
}
```

## Choosing the Right Type

### Use `One<T>` when:
- Value should have exactly one owner
- Transfer of ownership is explicit
- No sharing is needed
- Similar to `std::unique_ptr`

### Use `Ptr<T>` when:
- Multiple owners need to share a value
- Lifetime is managed by reference counting
- Shared mutations are acceptable
- Similar to `std::shared_ptr`

### Use `WeakPtr<T>` when:
- Need to observe without owning
- Want to break reference cycles
- Need to check if value still exists
- Similar to `std::weak_ptr`

## Comparison with JavaScript/TypeScript

### Standard JavaScript
```typescript
// JavaScript: References are shared by default
let obj1 = { x: 10 };
let obj2 = obj1; // Share reference
obj1.x = 20;
console.log(obj2.x); // 20 (shared)

// No explicit ownership or move semantics
// Garbage collection handles cleanup
```

### With uppts Smart Pointers
```typescript
// uppts: Explicit ownership
const one = new One({ x: 10 });
// const another = one; // Won't compile - no copy
const another = one.PickToOne(); // Explicit transfer

// Shared ownership when needed
const ptr1 = new Ptr({ x: 10 });
const ptr2 = ptr1.Share(); // Explicit sharing
```

## Best Practices

1. **Use One<T> by default** - Start with unique ownership, only share when necessary
2. **Avoid cycles with Ptr** - Use WeakPtr for back-references
3. **Check WeakPtr before use** - Always check `IsExpired()` or handle null from `Lock()`
4. **Clear explicitly** - Don't rely solely on garbage collection
5. **Document ownership** - Make ownership transfer explicit in function signatures

## Performance Considerations

- **One<T>**: Very lightweight, no ref counting overhead
- **Ptr<T>**: Small overhead for reference counting
- **WeakPtr<T>**: Slightly more overhead than Ptr
- **All**: Much lighter than custom reference counting implementations

## Migration from U++

### U++ One<T>
```cpp
// U++
One<String> str;
str = new String("hello");
String* p = str.Detach(); // Pick out
```

### uppts One<T>
```typescript
// uppts
const str = new One<String>();
str.Set(new String("hello"));
const p = str.Detach(); // Pick out
```

### U++ Ptr<T>
```cpp
// U++
Ptr<Object> ptr = new Object;
Ptr<Object> ptr2 = ptr; // Share
```

### uppts Ptr<T>
```typescript
// uppts
const ptr = new Ptr(new Object());
const ptr2 = ptr.Share(); // Share
```

## See Also

- [One API Reference](../src/Core/One.ts)
- [Ptr API Reference](../src/Core/Ptr.ts)
- [U++ Documentation](https://www.ultimatepp.org/)
