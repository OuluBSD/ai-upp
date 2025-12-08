# uppts Examples

This directory contains example applications demonstrating various features of the uppts library.

## Available Examples

### 1. Basic Containers (`basic-containers.ts`)
Demonstrates core container types:
- `Vector<T>` - Dynamic array
- `Map<K,V>` - Key-value storage
- `String` - U++ style string operations

### 2. Network Operations (`network-operations.ts`)
Shows networking capabilities:
- HTTP requests with `HttpRequest`
- Base64 encoding/decoding
- Data compression with gzip
- JSON handling

### 3. Threading and Async (`threading-async.ts`)
Illustrates concurrency features:
- Thread creation and management
- Mutex for thread synchronization
- Parallel processing with CoWork
- Async operations and Future patterns

## How to Run

To run the examples, first build the project:

```bash
npm run build
```

Then run an example with ts-node (if installed) or compile and run with Node.js:

```bash
# Using ts-node
npx ts-node examples/basic-containers.ts

# Or compile and run
npm run build
node dist/examples/basic-containers.js
```

## Learning Path

1. Start with `basic-containers.ts` to understand core container usage
2. Move to `network-operations.ts` to see HTTP and data processing
3. Explore `threading-async.ts` for concurrent programming patterns

These examples demonstrate how U++ patterns translate to TypeScript/Node.js while maintaining familiar APIs.