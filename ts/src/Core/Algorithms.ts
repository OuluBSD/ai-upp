/**
 * Utility functions for sorting and searching operations similar to U++ algorithms.
 */

/**
 * Sorts an array in place using the provided comparison function.
 * @param array The array to sort
 * @param compareFn Function that defines the sort order
 * @returns The sorted array
 */
export function Sort<T>(array: T[], compareFn?: (a: T, b: T) => number): T[] {
    return array.sort(compareFn);
}

/**
 * Searches for an element in a sorted array using binary search.
 * @param array The sorted array to search in
 * @param element The element to search for
 * @param compareFn Function that defines the comparison
 * @returns The index of the element if found, otherwise -1
 */
export function BinarySearch<T>(array: T[], element: T, compareFn?: (a: T, b: T) => number): number {
    if (compareFn === undefined) {
        compareFn = (a, b) => (a < b ? -1 : a > b ? 1 : 0);
    }

    let left = 0;
    let right = array.length - 1;

    while (left <= right) {
        const mid = Math.floor((left + right) / 2);
        const cmp = compareFn(element, array[mid]);

        if (cmp === 0) {
            return mid;
        } else if (cmp < 0) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }

    return -1;
}

/**
 * Finds the first element in an array that matches the provided predicate.
 * @param array The array to search in
 * @param predicate Function to test each element
 * @returns The first element that matches the predicate, or undefined if no match
 */
export function Find<T>(array: T[], predicate: (value: T, index: number, obj: T[]) => boolean): T | undefined {
    return array.find(predicate);
}

/**
 * Finds the index of the first element in an array that matches the provided predicate.
 * @param array The array to search in
 * @param predicate Function to test each element
 * @returns The index of the first element that matches the predicate, or -1 if no match
 */
export function FindIndex<T>(array: T[], predicate: (value: T, index: number, obj: T[]) => boolean): number {
    return array.findIndex(predicate);
}

/**
 * Checks if at least one element in an array passes the test implemented by the provided function.
 * @param array The array to test
 * @param predicate Function to test each element
 * @returns True if at least one element passes the test, false otherwise
 */
export function Contains<T>(array: T[], predicate: (value: T, index: number, obj: T[]) => boolean): boolean {
    return array.some(predicate);
}

/**
 * Checks if all elements in an array pass the test implemented by the provided function.
 * @param array The array to test
 * @param predicate Function to test each element
 * @returns True if all elements pass the test, false otherwise
 */
export function All<T>(array: T[], predicate: (value: T, index: number, obj: T[]) => boolean): boolean {
    return array.every(predicate);
}

/**
 * Filters an array based on the provided predicate function.
 * @param array The array to filter
 * @param predicate Function to test each element
 * @returns A new array containing elements that pass the test
 */
export function Filter<T>(array: T[], predicate: (value: T, index: number, obj: T[]) => boolean): T[] {
    return array.filter(predicate);
}

/**
 * Transforms each element in an array using the provided mapping function.
 * @param array The array to map
 * @param callbackFn Function that produces an element of the new array
 * @returns A new array containing transformed elements
 */
export function Map<T, U>(array: T[], callbackFn: (value: T, index: number, array: T[]) => U): U[] {
    return array.map(callbackFn);
}

/**
 * Reduces an array to a single value using the provided reducer function.
 * @param array The array to reduce
 * @param callbackFn Function to execute on each element
 * @param initialValue Value to use as the first argument to the first call of the callback
 * @returns The reduced value
 */
export function Reduce<T, U>(array: T[], callbackFn: (previousValue: U, currentValue: T, currentIndex: number, array: T[]) => U, initialValue: U): U {
    return array.reduce(callbackFn, initialValue);
}

/**
 * Executes a provided function once for each array element.
 * @param array The array to iterate over
 * @param callbackFn Function to execute for each element
 */
export function ForEach<T>(array: T[], callbackFn: (value: T, index: number, array: T[]) => void): void {
    array.forEach(callbackFn);
}

/**
 * Creates a new array with all sub-array elements concatenated into it recursively up to the specified depth.
 * @param array The array to flatten
 * @param depth The maximum recursion depth
 * @returns A new array with sub-array elements concatenated
 */
export function Flatten<T>(array: T[], depth?: number): T extends readonly (infer U)[] ? U[] : T[] {
    return array.flat(depth) as T extends readonly (infer U)[] ? U[] : T[];
}

/**
 * Removes duplicate values from an array based on the provided key selector function.
 * @param array The array to remove duplicates from
 * @param keySelector Function that produces a key for each element
 * @returns A new array with duplicates removed
 */
export function Distinct<T, K>(array: T[], keySelector: (item: T) => K): T[] {
    const seen = new Set<K>();
    const result: T[] = [];

    for (const item of array) {
        const key = keySelector(item);
        if (!seen.has(key)) {
            seen.add(key);
            result.push(item);
        }
    }

    return result;
}

/**
 * Joins two or more arrays and returns a new array.
 * @param arrays The arrays to join
 * @returns A new array containing all elements from the joined arrays
 */
export function Concat<T>(...arrays: T[][]): T[] {
    return ([] as T[]).concat(...arrays);
}

/**
 * Returns a section of an array.
 * @param array The array to extract from
 * @param start The beginning index (inclusive)
 * @param end The ending index (exclusive)
 * @returns A new array containing the extracted elements
 */
export function Slice<T>(array: T[], start?: number, end?: number): T[] {
    return array.slice(start, end);
}

/**
 * Reverses the order of elements in an array.
 * @param array The array to reverse
 * @returns A new array with elements in reversed order
 */
export function Reverse<T>(array: T[]): T[] {
    return [...array].reverse();
}

/**
 * Returns the first element of an array, or undefined if the array is empty.
 * @param array The array to get the first element from
 * @returns The first element or undefined
 */
export function First<T>(array: T[]): T | undefined {
    return array.length > 0 ? array[0] : undefined;
}

/**
 * Returns the last element of an array, or undefined if the array is empty.
 * @param array The array to get the last element from
 * @returns The last element or undefined
 */
export function Last<T>(array: T[]): T | undefined {
    return array.length > 0 ? array[array.length - 1] : undefined;
}

/**
 * Checks if an array includes a certain value among its entries.
 * @param array The array to search in
 * @param searchElement The element to search for
 * @param fromIndex The position in the array to start searching
 * @returns True if the element is found, false otherwise
 */
export function Includes<T>(array: T[], searchElement: T, fromIndex?: number): boolean {
    return array.includes(searchElement, fromIndex);
}

/**
 * Returns the number of elements in an array.
 * @param array The array to count elements in
 * @returns The number of elements in the array
 */
export function Count<T>(array: T[]): number {
    return array.length;
}

/**
 * Returns the index of the first occurrence of a value in an array.
 * @param array The array to search in
 * @param searchElement The element to search for
 * @param fromIndex The position in the array to start searching
 * @returns The index of the first occurrence, or -1 if not found
 */
export function IndexOf<T>(array: T[], searchElement: T, fromIndex?: number): number {
    return array.indexOf(searchElement, fromIndex);
}


// Export all functions for easy import
export default {
    Sort,
    BinarySearch,
    Find,
    FindIndex,
    Contains,
    All,
    Filter,
    Map,
    Reduce,
    ForEach,
    Flatten,
    Distinct,
    Concat,
    Slice,
    Reverse,
    First,
    Last,
    Includes,
    Count,
    IndexOf
};