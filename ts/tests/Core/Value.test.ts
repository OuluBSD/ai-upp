import { Value, Variant } from '../../src/Core/Value';

describe('Value', () => {
    test('constructor creates Value with initial value', () => {
        const value = new Value(42);
        expect(value.Is('number')).toBe(true);
        expect(value.Get()).toBe(42);
    });

    test('Is method checks type correctly', () => {
        let value = new Value(42);
        expect(value.Is('number')).toBe(true);
        expect(value.Is('string')).toBe(false);

        value.Set('hello');
        expect(value.Is('string')).toBe(true);
        expect(value.Is('number')).toBe(false);
    });

    test('Get method retrieves value correctly', () => {
        const value = new Value('hello');
        expect(value.Get()).toBe('hello');
    });

    test('Set method updates value', () => {
        const value = new Value(42);
        expect(value.Get()).toBe(42);

        value.Set('hello');
        expect(value.Get()).toBe('hello');
    });

    test('GetType returns correct type', () => {
        const value = new Value(42);
        expect(value.GetType()).toBe('number');
    });

    test('IsVoid checks for undefined correctly', () => {
        const value = new Value();
        expect(value.IsVoid()).toBe(true);

        value.Set(42);
        expect(value.IsVoid()).toBe(false);
    });

    test('Clear makes value undefined', () => {
        const value = new Value(42);
        expect(value.IsVoid()).toBe(false);

        value.Clear();
        expect(value.IsVoid()).toBe(true);
    });

    test('Static Create method creates Value', () => {
        const value = Value.Create('hello');
        expect(value.Get()).toBe('hello');
    });

    test('Pick transfers ownership', () => {
        const value = new Value('hello');
        const result = value.Pick();
        expect(result).toBe('hello');
        expect(value.IsVoid()).toBe(true);
    });
});

describe('Variant', () => {
    test('constructor creates Variant with initial value', () => {
        const variant = new Variant(42);
        expect(variant.Is('number')).toBe(true);
        expect(variant.Get()).toBe(42);
    });

    test('Is method checks type correctly', () => {
        const variant = new Variant(42);
        expect(variant.Is('number')).toBe(true);
        expect(variant.Is('string')).toBe(false);
    });

    test('Get method retrieves value correctly', () => {
        const variant = new Variant('hello');
        expect(variant.Get()).toBe('hello');
    });

    test('Set method updates value', () => {
        // Using a union type to allow both string and number
        const variant = new Variant(42 as number | string);
        expect(variant.Get()).toBe(42);

        variant.Set('hello');
        expect(variant.Get()).toBe('hello');
    });

    test('GetType returns correct type', () => {
        const variant = new Variant(42);
        expect(variant.GetType()).toBe('number');
    });

    test('Pick transfers ownership', () => {
        const variant = new Variant('hello');
        const result = variant.Pick();
        expect(result).toBe('hello');
    });
});