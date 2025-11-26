import { Tuple } from '../../src/Core/Tuple';

describe('Tuple', () => {
    test('constructor creates tuple with provided values', () => {
        const tuple = new Tuple(1, 'hello', true);
        expect(tuple.Get(0)).toBe(1);
        expect(tuple.Get(1)).toBe('hello');
        expect(tuple.Get(2)).toBe(true);
    });

    test('GetCount returns correct number of elements', () => {
        const tuple = new Tuple(1, 'hello', true);
        expect(tuple.GetCount()).toBe(3);
    });

    test('IsEmpty returns correct value', () => {
        const nonEmptyTuple = new Tuple(1, 'hello');
        expect(nonEmptyTuple.IsEmpty()).toBe(false);
    });

    test('At method returns same as Get', () => {
        const tuple = new Tuple('test', 42);
        expect(tuple.At(0)).toBe('test');
        expect(tuple.At(1)).toBe(42);
    });

    test('ToArray returns array with same elements', () => {
        const tuple = new Tuple(10, 'hello');
        const array = tuple.ToArray();
        expect(Array.isArray(array)).toBe(true);
        expect(array[0]).toBe(10);
        expect(array[1]).toBe('hello');
    });

    test('Elements returns the internal elements array', () => {
        const tuple = new Tuple(10, 'hello');
        const elements = tuple.Elements();
        expect(elements[0]).toBe(10);
        expect(elements[1]).toBe('hello');
    });

    test('Set updates value at specific index', () => {
        const tuple = new Tuple(10, 'hello');
        tuple.Set(0, 20);
        expect(tuple.Get(0)).toBe(20);
        expect(tuple.Get(1)).toBe('hello');
    });

    test('Make static method creates new tuple', () => {
        const tuple = Tuple.Make(1, 'hello', true);
        expect(tuple.Get(0)).toBe(1);
        expect(tuple.Get(1)).toBe('hello');
        expect(tuple.Get(2)).toBe(true);
    });

    test('Get throws error for invalid index', () => {
        const tuple = new Tuple(1, 'hello');
        expect(() => tuple.Get(5)).toThrow();
        expect(() => tuple.Get(-1)).toThrow();
    });

    test('Set throws error for invalid index', () => {
        const tuple = new Tuple(1, 'hello');
        expect(() => tuple.Set(5 as any, 30)).toThrow();
        expect(() => tuple.Set(-1 as any, 30)).toThrow();
    });
});