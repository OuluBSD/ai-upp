/**
 * Performance Benchmarks for uppts library
 * Compares performance of various operations
 */

import { Vector } from '../src/Core/Vector';
import { Map } from '../src/Core/Map';
import { String } from '../src/Core/String';
import { One } from '../src/Core/One';
import { Ptr } from '../src/Core/Ptr';

interface BenchmarkResult {
  name: string;
  opsPerSec: number;
  avgTime: number; // in milliseconds
  iterations: number;
}

class BenchmarkRunner {
  private results: BenchmarkResult[] = [];

  async runBenchmark(name: string, benchmarkFn: () => void, iterations: number = 100000): Promise<void> {
    console.log(`Running benchmark: ${name} (${iterations} iterations)`);
    
    // Warm up to ensure JIT compilation
    for (let i = 0; i < 1000; i++) {
      benchmarkFn();
    }
    
    const startTime = process.hrtime.bigint();
    
    for (let i = 0; i < iterations; i++) {
      benchmarkFn();
    }
    
    const endTime = process.hrtime.bigint();
    const totalTimeNs = Number(endTime - startTime); // Convert to number of nanoseconds
    const totalTimeMs = totalTimeNs / 1_000_000; // Convert to milliseconds
    const avgTime = totalTimeMs / iterations;
    const opsPerSec = (iterations / totalTimeMs) * 1000; // Operations per second
    
    const result: BenchmarkResult = {
      name,
      opsPerSec,
      avgTime,
      iterations
    };
    
    this.results.push(result);
    
    console.log(`  Result: ${opsPerSec.toFixed(2)} ops/sec, avg: ${avgTime.toFixed(6)}ms per op\n`);
  }

  addResult(result: BenchmarkResult): void {
    this.results.push(result);
  }

  printResults(): void {
    console.log('=== Performance Benchmark Results ===\n');
    console.table(this.results, ['name', 'opsPerSec', 'avgTime']);
    
    console.log('\nNote: Results may vary between runs due to system load and Node.js optimizations.');
  }
}

async function runBenchmarks() {
  console.log('Starting uppts performance benchmarks...\n');
  
  const benchmark = new BenchmarkRunner();

  // Benchmark Vector operations
  await benchmark.runBenchmark('Vector.Add (int)', () => {
    const vec = new Vector<number>();
    vec.Add(42);
  }, 100000);

  await benchmark.runBenchmark('Vector.At (get by index)', () => {
    const vec = new Vector<number>();
    vec.Add(42);
    vec.Add(43);
    const val = vec.At(0);
  }, 100000);

  await benchmark.runBenchmark('Vector iteration', () => {
    const vec = new Vector<number>();
    for (let i = 0; i < 10; i++) vec.Add(i);
    let sum = 0;
    for (const item of vec) {
      sum += item;
    }
  }, 10000);

  // Benchmark Map operations
  await benchmark.runBenchmark('Map.Set (string key)', () => {
    const map = new Map<string, number>();
    map.Set('testKey', 42);
  }, 100000);

  await benchmark.runBenchmark('Map.Get (string key)', () => {
    const map = new Map<string, number>();
    map.Set('testKey', 42);
    const val = map.Get('testKey', 0);
  }, 100000);

  // Benchmark String operations
  await benchmark.runBenchmark('String concatenation', () => {
    const s1 = new String('Hello');
    const s2 = new String(' World');
    const result = s1 + s2;
  }, 100000);

  await benchmark.runBenchmark('String.GetLength', () => {
    const s = new String('Hello, Performance World!');
    const len = s.GetLength();
  }, 100000);

  // Benchmark memory management (One and Ptr)
  await benchmark.runBenchmark('One<T> creation and access', () => {
    const one = new One<number>(42);
    const val = one.Get();
  }, 100000);

  await benchmark.runBenchmark('One<T> Pick operation', () => {
    const one = new One<number>(42);
    const val = one.Pick();
    new One<number>(val); // Reassign
  }, 10000);

  await benchmark.runBenchmark('Ptr<T> creation and access', () => {
    const ptr = new Ptr<number>(42);
    const val = ptr.Get();
  }, 100000);

  // Benchmark array vs Vector performance
  await benchmark.runBenchmark('Native Array operations', () => {
    const arr: number[] = [];
    arr.push(42);
    const val = arr[0];
  }, 100000);

  // Benchmark complex operations
  await benchmark.runBenchmark('Complex object creation', () => {
    const complexObj = {
      vector: new Vector<string>(),
      map: new Map<number, string>(),
      str: new String('test')
    };
    complexObj.vector.Add('item1');
    complexObj.map.Set(1, 'value1');
  }, 50000);

  // Run a realistic scenario
  await benchmark.runBenchmark('Inventory management scenario', () => {
    // Simulate managing an inventory with multiple operations
    const inventory = new Map<number, { name: string, quantity: number }>();
    
    // Add items
    inventory.Set(1, { name: 'Item 1', quantity: 10 });
    inventory.Set(2, { name: 'Item 2', quantity: 5 });
    
    // Update quantities
    const item1 = inventory.Get(1, { name: 'Default', quantity: 0 });
    if (item1) {
      inventory.Set(1, { name: item1.name, quantity: item1.quantity - 1 });
    }
    
    // Get values
    const item2 = inventory.Get(2, { name: 'Default', quantity: 0 });
  }, 25000);

  benchmark.printResults();
  
  // Save benchmark results to a file
  const fs = require('fs');
  const resultsPath = './benchmark-results.json';
  fs.writeFileSync(resultsPath, JSON.stringify(benchmark['results'], null, 2));
  console.log(`\nBenchmark results saved to ${resultsPath}`);
}

// Run the benchmarks
runBenchmarks().catch(err => {
  console.error('Benchmark error:', err);
});