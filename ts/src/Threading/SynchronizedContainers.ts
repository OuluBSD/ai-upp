/**
 * Thread-safe container wrappers that provide synchronized access
 * to data structures from multiple threads
 */

import { Mutex } from './Mutex';
import { Vector } from '../Core/Vector';
import { Array } from '../Core/Array';
import { Index } from '../Core/Index';
import { Map } from '../Core/Map';

/**
 * Thread-safe wrapper for Vector
 */
export class SynchronizedVector<T> {
    private vector: Vector<T>;
    private mutex: Mutex;

    constructor() {
        this.vector = new Vector<T>();
        this.mutex = new Mutex();
    }

    /**
     * Adds an element to the vector in a thread-safe manner
     * @param element The element to add
     */
    async Add(element: T): Promise<void> {
        const release = await this.mutex.Lock();
        try {
            this.vector.Add(element);
        } finally {
            release();
        }
    }

    /**
     * Gets an element at the specified index in a thread-safe manner
     * @param index The index of the element to get
     * @returns The element at the specified index
     */
    async At(index: number): Promise<T> {
        const release = await this.mutex.Lock();
        try {
            return this.vector.At(index);
        } finally {
            release();
        }
    }

    /**
     * Gets the count of elements in a thread-safe manner
     * @returns The number of elements in the vector
     */
    async GetCount(): Promise<number> {
        const release = await this.mutex.Lock();
        try {
            return this.vector.GetCount();
        } finally {
            release();
        }
    }

    /**
     * Removes an element at the specified index in a thread-safe manner
     * @param index The index of the element to remove
     * @returns The removed element
     */
    async Remove(index: number): Promise<T> {
        const release = await this.mutex.Lock();
        try {
            const element = this.vector.At(index);
            this.vector.Remove(index);
            return element;
        } finally {
            release();
        }
    }

    /**
     * Checks if the vector is empty in a thread-safe manner
     * @returns true if the vector is empty, false otherwise
     */
    async IsEmpty(): Promise<boolean> {
        const release = await this.mutex.Lock();
        try {
            return this.vector.GetCount() === 0;
        } finally {
            release();
        }
    }

    /**
     * Clears the vector in a thread-safe manner
     */
    async Clear(): Promise<void> {
        const release = await this.mutex.Lock();
        try {
            this.vector.Clear();
        } finally {
            release();
        }
    }
}

/**
 * Thread-safe wrapper for Array
 */
export class SynchronizedArray<T> {
    private array: Array<T>;
    private mutex: Mutex;

    constructor() {
        this.array = new Array<T>();
        this.mutex = new Mutex();
    }

    /**
     * Adds an element to the array in a thread-safe manner
     * @param element The element to add
     */
    async Add(element: T): Promise<void> {
        const release = await this.mutex.Lock();
        try {
            this.array.Add(element);
        } finally {
            release();
        }
    }

    /**
     * Gets an element at the specified index in a thread-safe manner
     * @param index The index of the element to get
     * @returns The element at the specified index
     */
    async At(index: number): Promise<T> {
        const release = await this.mutex.Lock();
        try {
            return this.array.At(index);
        } finally {
            release();
        }
    }

    /**
     * Gets the count of elements in a thread-safe manner
     * @returns The number of elements in the array
     */
    async GetCount(): Promise<number> {
        const release = await this.mutex.Lock();
        try {
            return this.array.GetCount();
        } finally {
            release();
        }
    }

    /**
     * Checks if the array is empty in a thread-safe manner
     * @returns true if the array is empty, false otherwise
     */
    async IsEmpty(): Promise<boolean> {
        const release = await this.mutex.Lock();
        try {
            return this.array.GetCount() === 0;
        } finally {
            release();
        }
    }

    /**
     * Clears the array in a thread-safe manner
     */
    async Clear(): Promise<void> {
        const release = await this.mutex.Lock();
        try {
            this.array.Clear();
        } finally {
            release();
        }
    }
}

/**
 * Thread-safe wrapper for Index
 */
export class SynchronizedIndex<T> {
    private index: Index<T>;
    private mutex: Mutex;

    constructor() {
        this.index = new Index<T>();
        this.mutex = new Mutex();
    }

    /**
     * Adds an element to the index in a thread-safe manner
     * @param element The element to add
     * @returns true if the element was added, false if it already existed
     */
    async Add(element: T): Promise<boolean> {
        const release = await this.mutex.Lock();
        try {
            // The Index.Add method returns the index at which the element was added
            // but we want to return whether it was added (true) or already existed (false)
            const currentIndex = this.index.Find(element);
            if (currentIndex >= 0) {
                return false; // Already exists
            }
            this.index.Add(element);
            return true; // Added successfully
        } finally {
            release();
        }
    }

    /**
     * Finds an element in the index in a thread-safe manner
     * @param element The element to find
     * @returns The index of the element, or -1 if not found
     */
    async Find(element: T): Promise<number> {
        const release = await this.mutex.Lock();
        try {
            return this.index.Find(element);
        } finally {
            release();
        }
    }

    /**
     * Gets the count of elements in a thread-safe manner
     * @returns The number of elements in the index
     */
    async GetCount(): Promise<number> {
        const release = await this.mutex.Lock();
        try {
            return this.index.GetCount();
        } finally {
            release();
        }
    }

    /**
     * Checks if the index contains the specified element in a thread-safe manner
     * @param element The element to check for
     * @returns true if the element is in the index, false otherwise
     */
    async Contains(element: T): Promise<boolean> {
        const release = await this.mutex.Lock();
        try {
            return this.index.Find(element) >= 0;
        } finally {
            release();
        }
    }

    /**
     * Removes an element from the index in a thread-safe manner
     * @param element The element to remove
     */
    async Remove(element: T): Promise<void> {
        const release = await this.mutex.Lock();
        try {
            const idx = this.index.Find(element);
            if (idx >= 0) {
                this.index.RemoveAt(idx);
            }
        } finally {
            release();
        }
    }
}

/**
 * Thread-safe wrapper for Map
 */
export class SynchronizedMap<K, V> {
    private map: Map<K, V>;
    private mutex: Mutex;

    constructor() {
        this.map = new Map<K, V>();
        this.mutex = new Mutex();
    }

    /**
     * Sets a key-value pair in the map in a thread-safe manner
     * @param key The key
     * @param value The value
     */
    async Set(key: K, value: V): Promise<void> {
        const release = await this.mutex.Lock();
        try {
            this.map.Set(key, value);
        } finally {
            release();
        }
    }

    /**
     * Gets a value by key in a thread-safe manner
     * @param key The key
     * @returns The value associated with the key, or undefined if not found
     */
    async Get(key: K): Promise<V | undefined> {
        const release = await this.mutex.Lock();
        try {
            // The Map.Get method requires a default value, so we provide undefined
            const result = this.map.Get(key, undefined as any as V);
            // If the key doesn't exist, Get will return the default value
            // Check if the key exists first
            if (this.map.Find(key) >= 0) {
                return result;
            } else {
                return undefined;
            }
        } finally {
            release();
        }
    }

    /**
     * Checks if the map contains the specified key in a thread-safe manner
     * @param key The key to check for
     * @returns true if the key exists in the map, false otherwise
     */
    async Has(key: K): Promise<boolean> {
        const release = await this.mutex.Lock();
        try {
            return this.map.Find(key) >= 0;
        } finally {
            release();
        }
    }

    /**
     * Removes a key-value pair from the map in a thread-safe manner
     * @param key The key to remove
     * @returns true if the key existed and was removed, false otherwise
     */
    async Remove(key: K): Promise<boolean> {
        const release = await this.mutex.Lock();
        try {
            const exists = this.map.Find(key) >= 0;
            if (exists) {
                return this.map.RemoveKey(key);
            }
            return false;
        } finally {
            release();
        }
    }

    /**
     * Gets the count of key-value pairs in a thread-safe manner
     * @returns The number of key-value pairs in the map
     */
    async GetCount(): Promise<number> {
        const release = await this.mutex.Lock();
        try {
            return this.map.GetCount();
        } finally {
            release();
        }
    }

    /**
     * Gets all keys in a thread-safe manner
     * @returns Array of keys in the map
     */
    async GetKeys(): Promise<K[]> {
        const release = await this.mutex.Lock();
        try {
            return this.map.GetKeys();
        } finally {
            release();
        }
    }

    /**
     * Gets all values in a thread-safe manner
     * @returns Array of values in the map
     */
    async GetValues(): Promise<V[]> {
        const release = await this.mutex.Lock();
        try {
            return this.map.GetValues();
        } finally {
            release();
        }
    }
}

/**
 * Generic thread-safe container that can wrap any object with mutex protection
 */
export class Synchronized<T> {
    private obj: T;
    private mutex: Mutex;

    constructor(obj: T) {
        this.obj = obj;
        this.mutex = new Mutex();
    }

    /**
     * Executes a function with synchronized access to the wrapped object
     * @param fn The function to execute with synchronized access
     * @returns The result of the function
     */
    async WithSync<R>(fn: (obj: T) => R): Promise<R> {
        const release = await this.mutex.Lock();
        try {
            return fn(this.obj);
        } finally {
            release();
        }
    }

    /**
     * Gets a reference to the wrapped object (not thread-safe)
     * This should only be used carefully when you manage synchronization externally
     * @returns The wrapped object
     */
    Get(): T {
        return this.obj;
    }
}