import Algorithms, { 
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
} from '../../src/Core/Algorithms';

describe('Algorithms', () => {
    test('Sort sorts array correctly', () => {
        const arr = [3, 1, 4, 1, 5, 9, 2, 6];
        const sorted = Sort([...arr]);
        expect(sorted).toEqual([1, 1, 2, 3, 4, 5, 6, 9]);
    });

    test('BinarySearch finds element in sorted array', () => {
        const arr = [1, 3, 5, 7, 9, 11, 13, 15];
        expect(BinarySearch(arr, 7)).toBe(3); // 7 is at index 3
        expect(BinarySearch(arr, 1)).toBe(0); // 1 is at index 0
        expect(BinarySearch(arr, 15)).toBe(7); // 15 is at index 7
        expect(BinarySearch(arr, 4)).toBe(-1); // 4 is not in the array
        expect(BinarySearch(arr, 16)).toBe(-1); // 16 is not in the array
    });

    test('Find finds first element matching predicate', () => {
        const arr = [1, 2, 3, 4, 5, 6];
        const result = Find(arr, x => x > 3);
        expect(result).toBe(4);

        const notFound = Find(arr, x => x > 10);
        expect(notFound).toBeUndefined();
    });

    test('FindIndex finds index of first element matching predicate', () => {
        const arr = [1, 2, 3, 4, 5, 6];
        const index = FindIndex(arr, x => x > 3);
        expect(index).toBe(3); // 4 is at index 3

        const notFoundIndex = FindIndex(arr, x => x > 10);
        expect(notFoundIndex).toBe(-1);
    });

    test('Contains checks if any element matches predicate', () => {
        const arr = [1, 2, 3, 4, 5];
        expect(Contains(arr, x => x > 4)).toBe(true);
        expect(Contains(arr, x => x > 10)).toBe(false);
    });

    test('All checks if all elements match predicate', () => {
        const arr = [2, 4, 6, 8];
        expect(All(arr, x => x % 2 === 0)).toBe(true);
        
        const arr2 = [2, 4, 5, 8];
        expect(All(arr2, x => x % 2 === 0)).toBe(false);
    });

    test('Filter filters elements based on predicate', () => {
        const arr = [1, 2, 3, 4, 5, 6];
        const filtered = Filter(arr, x => x % 2 === 0);
        expect(filtered).toEqual([2, 4, 6]);
    });

    test('Map transforms elements using mapping function', () => {
        const arr = [1, 2, 3, 4];
        const mapped = Map(arr, x => x * 2);
        expect(mapped).toEqual([2, 4, 6, 8]);
    });

    test('Reduce reduces array to single value', () => {
        const arr = [1, 2, 3, 4];
        const sum = Reduce(arr, (acc, val) => acc + val, 0);
        expect(sum).toBe(10);
        
        const product = Reduce(arr, (acc, val) => acc * val, 1);
        expect(product).toBe(24);
    });

    test('ForEach executes function for each element', () => {
        const arr = [1, 2, 3];
        const mockFn = jest.fn();
        
        ForEach(arr, mockFn);
        expect(mockFn).toHaveBeenCalledTimes(3);
        expect(mockFn).toHaveBeenNthCalledWith(1, 1, 0, arr);
        expect(mockFn).toHaveBeenNthCalledWith(2, 2, 1, arr);
        expect(mockFn).toHaveBeenNthCalledWith(3, 3, 2, arr);
    });

    test('Flatten flattens nested arrays', () => {
        const arr = [1, [2, 3], [4, [5, 6]]];
        const flattened = Flatten(arr, 2);
        expect(flattened).toEqual([1, 2, 3, 4, 5, 6]);
    });

    test('Distinct removes duplicates based on key selector', () => {
        const arr = ['apple', 'banana', 'apricot', 'blueberry'];
        const distinct = Distinct(arr, s => s[0]); // Distinct by first letter
        expect(distinct).toEqual(['apple', 'banana']);
    });

    test('Concat joins arrays', () => {
        const arr1 = [1, 2];
        const arr2 = [3, 4];
        const arr3 = [5, 6];
        const result = Concat(arr1, arr2, arr3);
        expect(result).toEqual([1, 2, 3, 4, 5, 6]);
    });

    test('Slice returns section of array', () => {
        const arr = [1, 2, 3, 4, 5];
        const sliced = Slice(arr, 1, 3);
        expect(sliced).toEqual([2, 3]);
    });

    test('Reverse returns reversed array', () => {
        const arr = [1, 2, 3, 4];
        const reversed = Reverse(arr);
        expect(reversed).toEqual([4, 3, 2, 1]);
        // Original array should be unchanged
        expect(arr).toEqual([1, 2, 3, 4]);
    });

    test('First returns first element or undefined', () => {
        const arr1 = [1, 2, 3];
        expect(First(arr1)).toBe(1);

        const arr2: number[] = [];
        expect(First(arr2)).toBeUndefined();
    });

    test('Last returns last element or undefined', () => {
        const arr1 = [1, 2, 3];
        expect(Last(arr1)).toBe(3);

        const arr2: number[] = [];
        expect(Last(arr2)).toBeUndefined();
    });

    test('Includes checks if array includes element', () => {
        const arr = [1, 2, 3, 4, 5];
        expect(Includes(arr, 3)).toBe(true);
        expect(Includes(arr, 6)).toBe(false);
        expect(Includes(arr, 3, 3)).toBe(false); // start searching from index 3
    });

    test('Count returns number of elements', () => {
        const arr = [1, 2, 3, 4, 5];
        expect(Count(arr)).toBe(5);

        const empty: number[] = [];
        expect(Count(empty)).toBe(0);
    });

    test('IndexOf returns index of element', () => {
        const arr = [1, 2, 3, 2, 4];
        expect(IndexOf(arr, 2)).toBe(1);
        expect(IndexOf(arr, 5)).toBe(-1);
        expect(IndexOf(arr, 2, 2)).toBe(3); // start searching from index 2
    });

    test('All algorithms are exported in default export object', () => {
        expect(Algorithms.Sort).toBe(Sort);
        expect(Algorithms.BinarySearch).toBe(BinarySearch);
        expect(Algorithms.Find).toBe(Find);
        expect(Algorithms.Contains).toBe(Contains);
        expect(Algorithms.All).toBe(All);
        expect(Algorithms.Filter).toBe(Filter);
        expect(Algorithms.Map).toBe(Map);
        expect(Algorithms.Reduce).toBe(Reduce);
        expect(Algorithms.ForEach).toBe(ForEach);
        expect(Algorithms.Flatten).toBe(Flatten);
        expect(Algorithms.Distinct).toBe(Distinct);
        expect(Algorithms.Concat).toBe(Concat);
        expect(Algorithms.Slice).toBe(Slice);
        expect(Algorithms.Reverse).toBe(Reverse);
        expect(Algorithms.First).toBe(First);
        expect(Algorithms.Last).toBe(Last);
        expect(Algorithms.Includes).toBe(Includes);
        expect(Algorithms.Count).toBe(Count);
        expect(Algorithms.IndexOf).toBe(IndexOf);
    });
});