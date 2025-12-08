# Performance Benchmarks

This directory contains performance benchmarking code for the uppts library to measure the performance of various operations.

## Running Benchmarks

To run the benchmarks:

```bash
# Install ts-node if not already installed
npm install -g ts-node

# Run the benchmarks
npx ts-node benchmarks/performance.ts
```

Or build and run with Node.js:

```bash
npm run build
node dist/benchmarks/performance.js
```

## Benchmark Categories

The benchmarks measure performance of:

- Basic container operations (Vector, Map)
- String operations
- Memory management (One, Ptr)
- Comparison with native JavaScript equivalents
- Complex real-world scenarios

## Results

Benchmark results are output to the console and saved to `benchmark-results.json` for analysis.

## Contributing New Benchmarks

To add new benchmarks:
1. Add your benchmark function to `performance.ts`
2. Use the `BenchmarkRunner` class to ensure consistent measurement
3. Make sure to include warmup iterations for JIT compilation
4. Consider the impact of garbage collection on results