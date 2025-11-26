# uppts - U++ TypeScript Interface

TypeScript/Node.js interface library with U++-like API.

## Overview

This package provides familiar U++ class names and method signatures, implemented using TypeScript/JavaScript features and npm libraries. It enables U++ developers to work with Node.js using familiar patterns and facilitates code conversion between U++ and TypeScript.

## Project Status

**Status**: Phase 1 - Project Setup (IN PROGRESS)

This is an early-stage project. Core containers and utilities are under development.

## Installation

```bash
npm install @uppts/core
```

## Quick Start

```typescript
import { Vector, String } from '@uppts/core';

// Create a vector of numbers
const v = new Vector<number>();
v.Add(1);
v.Add(2);
v.Add(3);

console.log(v.GetCount()); // 3
console.log(v.At(0));      // 1

// Use U++ style string
const s = new String("Hello");
s.Cat(" World");
console.log(s.ToString()); // "Hello World"
```

## Architecture

```
ts/
├── src/
│   ├── Core/          # Containers, strings, smart pointers
│   ├── IO/            # File and stream I/O
│   ├── Threading/     # Async and concurrency
│   ├── DateTime/      # Time and date
│   └── Network/       # HTTP, TCP, WebSocket
├── tests/             # Test suite
├── docs/              # Documentation
└── examples/          # Usage examples
```

## Development

```bash
# Install dependencies
npm install

# Build
npm run build

# Run tests
npm test

# Lint
npm run lint

# Format code
npm run format

# Generate docs
npm run docs
```

## Documentation

See the [task/uppts.md](../task/uppts.md) and [roadmap/uppts.md](../roadmap/uppts.md) for detailed information about:
- Project goals and architecture
- 10-phase development roadmap
- API design principles
- Implementation notes

## Comparison

| Aspect | uppts | stdsrc | uppstd |
|--------|-------|--------|--------|
| Language | TypeScript/JavaScript | C++ | Documentation |
| Purpose | Node.js library | STL-backed U++ | Mapping reference |
| Implementation | npm packages + custom | STL + custom wrappers | N/A |
| Target | JavaScript developers | C++ developers | AI & converters |
| Runtime | Node.js | Compiled binary | N/A |

## Requirements

- Node.js 18+ (LTS)
- TypeScript 5.x

## License

BSD-3-Clause (matching U++ framework)

## Contributing

This project is part of the ai-upp development initiative. See project documentation for contribution guidelines.

## Related Projects

- **U++ Framework**: https://www.ultimatepp.org/
- **stdsrc**: U++ API implementation using STL
- **uppstd**: U++ ↔ STL mapping documentation
