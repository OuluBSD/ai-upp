# Migration Guide: U++ to uppts

This guide helps U++ developers transition to using the uppts (U++ TypeScript Interface) library in Node.js projects. The goal is to provide familiar APIs while leveraging TypeScript/JavaScript features.

## Overview

The uppts library provides U++-like APIs implemented in TypeScript for Node.js. This allows developers familiar with U++ to use similar patterns and APIs in JavaScript/TypeScript environments.

## Key Similarities

- Same class names (Vector, String, Map, etc.)
- Similar method signatures
- Familiar API patterns (GetCount, Add, At, etc.)
- Similar functionality for core containers

## Key Differences

### 1. Environment
- U++: C++ framework with compiled binaries
- uppts: TypeScript/Node.js with runtime execution

### 2. Memory Management
- U++: Manual memory management with optional smart pointers
- uppts: Garbage-collected environment with automatic memory management
- Smart pointers (One, Ptr) implemented for U++ compatibility but behave differently

### 3. Compilation vs. Interpretation
- U++: Ahead-of-time compilation
- uppts: Just-in-time compilation (Node.js) or ahead-of-time (bundled)

## Migrating Common Types

### Containers

#### Vector
```cpp
// U++
Vector<int> vec;
vec.Add(42);
int count = vec.GetCount();
int item = vec[0];
```

```typescript
// uppts
const vec = new Vector<number>();
vec.Add(42);
const count = vec.GetCount();
const item = vec.At(0);  // Note: At() instead of [] operator
```

#### Map
```cpp
// U++
Map<String, int> map;
map.Add("key", 42);
int value = map.Get("key", 0);  // Provide default value
```

```typescript
// uppts
const map = new Map<string, number>();
map.Set("key", 42);  // Set instead of Add for key-value pairs
const value = map.Get("key", 0);  // Same interface as U++
```

#### String
```cpp
// U++
String s = "Hello";
s.Cat(" World");
int length = s.GetLength();
```

```typescript
// uppts
const s = new String("Hello");
s.Cat(" World");
const length = s.GetLength();  // Same interface as U++
```

### Memory Management

#### One<T> (unique pointer equivalent)
```cpp
// U++
One<Vector<int>> vec = One<Vector<int>>::Create(new Vector<int>());
vec->Add(42);
One<Vector<int>> moved = std::move(vec);  // C++ move semantics
```

```typescript
// uppts
const vec = new One<Vector<number>>(new Vector<number>());
vec.Get().Add(42);
const moved = vec.PickToOne();  // Transfer ownership with Pick/Attach pattern
```

#### Ptr<T> (shared pointer equivalent)
```cpp
// U++
Ptr<Vector<int>> vec = new Vector<int>();
vec->Add(42);
Ptr<Vector<int>> shared = vec;  // Shared ownership
```

```typescript
// uppts
const vec = new Ptr<Vector<number>>(new Vector<number>());
vec.Get().Add(42);
const shared = vec;  // Reference counting handled automatically
```

### Threading

#### Thread
```cpp
// U++
Thread thread;
thread.Run([&]() { /* work */ });
thread.Wait();
```

```typescript
// uppts
const thread = new Thread(() => { /* work */ });
thread.Start();
// In TypeScript, functions often use async/await instead of thread joining
```

### I/O Operations

#### File Reading
```cpp
// U++
FileIn file;
if(file.Open("data.txt")) {
    String line;
    while(file.GetLine(line)) {
        // process line
    }
}
```

```typescript
// uppts
const file = new FileIn();
if(file.Open("data.txt")) {
    let line: string | null;
    while((line = file.ReadLine()) !== null) {
        // process line
    }
    file.Close();
}
```

## Async Patterns

JavaScript/TypeScript has built-in async capabilities that differ from U++:

```cpp
// U++ synchronous approach
HttpRequest req;
HttpResponse resp = req.Url("https://api.example.com").Execute();
```

```typescript
// uppts async approach
const req = new HttpRequest();
const resp = await req.Url("https://api.example.com").ExecuteAsync();
// Or, uppts also provides convenience functions:
const resp2 = await HttpGet("https://api.example.com");
```

## Error Handling

U++ often uses exceptions or return codes, while TypeScript has exceptions and promises:

```cpp
// U++
FileOut file;
if (!file.Open("output.txt")) {
    // Handle error based on return value
}
```

```typescript
// uppts
const file = new FileOut();
if (!file.Open("output.txt")) {
    // Check return value as in U++
    console.error("Failed to open file:", file.GetLastError());
}
// Or use exceptions
try {
    const file = new FileOut();
    if (!file.Open("output.txt")) {
        throw new Error("Failed to open file");
    }
} catch (error) {
    console.error("Error:", error);
}
```

## Best Practices for Migration

1. **Start with core containers**: Vector, Map, String, etc. have the closest API match
2. **Use async alternatives**: When possible, prefer async methods in Node.js environment
3. **Leverage TypeScript**: Use strong typing to catch errors at compile time
4. **Consider Node.js ecosystem**: While maintaining U++ patterns, consider using Node.js built-in features when appropriate
5. **Test thoroughly**: Different runtime environment may reveal new edge cases

## Examples of Migration

### U++ Example
```cpp
Vector<String> ProcessFile(const char* filename) {
    Vector<String> lines;
    FileIn file;
    if (file.Open(filename)) {
        String line;
        while (file.GetLine(line)) {
            lines.Add(line);
        }
    }
    return lines;
}
```

### uppts Equivalent
```typescript
async function ProcessFile(filename: string): Promise<Vector<String>> {
    const lines = new Vector<String>();
    const file = new FileIn();
    if (await file.OpenAsync(filename)) {
        let line: string | null;
        while ((line = await file.ReadLineAsync()) !== null) {
            lines.Add(new String(line));
        }
        await file.CloseAsync();
    }
    return lines;
}
```

## Additional Resources

- [API Documentation](./docs/api): Complete reference for all uppts classes and methods
- [Examples](./examples): Real-world usage examples
- [Comparison with U++ and STL](./comparison.md): Detailed feature comparison

## Troubleshooting Common Issues

1. **Undefined behavior**: JavaScript/TypeScript has undefined instead of U++'s undefined behavior - handle null/undefined explicitly
2. **Async/Sync confusion**: Be clear about which methods are synchronous vs. asynchronous
3. **Memory management**: Remember that garbage collection handles most memory management automatically
4. **Type coercion**: TypeScript is stricter about types than C++ - use explicit type conversions when needed