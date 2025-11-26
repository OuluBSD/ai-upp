import { Optional } from '../../src/Core/Optional';

describe('Optional', () => {
    test('constructor creates empty Optional', () => {
        const opt = new Optional<number>();
        expect(opt.IsNull()).toBe(true);
    });

    test('Set and Get methods work correctly', () => {
        const opt = new Optional<string>();
        opt.Set('hello');
        expect(opt.IsNull()).toBe(false);
        expect(opt.Get()).toBe('hello');
    });

    test('GetWithDefault returns value if present, default if not', () => {
        const opt = new Optional<number>();
        expect(opt.GetWithDefault(42)).toBe(42);

        opt.Set(10);
        expect(opt.GetWithDefault(42)).toBe(10);
    });

    test('Clear makes Optional empty', () => {
        const opt = Optional.Of('hello');
        expect(opt.IsNull()).toBe(false);
        
        opt.Clear();
        expect(opt.IsNull()).toBe(true);
    });

    test('Static Of method creates Optional with value', () => {
        const opt = Optional.Of('hello');
        expect(opt.IsNull()).toBe(false);
        expect(opt.Get()).toBe('hello');
    });

    test('Static Of method creates empty Optional for null/undefined', () => {
        const opt1 = Optional.Of(null);
        expect(opt1.IsNull()).toBe(true);

        const opt2 = Optional.Of(undefined);
        expect(opt2.IsNull()).toBe(true);
    });

    test('Static Null method creates empty Optional', () => {
        const opt = Optional.Null<number>();
        expect(opt.IsNull()).toBe(true);
    });

    test('Pick transfers ownership', () => {
        const opt = Optional.Of('hello');
        const value = opt.Pick();
        expect(value).toBe('hello');
        expect(opt.IsNull()).toBe(true);
    });

    test('IsEqual compares values correctly', () => {
        const opt1 = Optional.Of('hello');
        const opt2 = Optional.Of('hello');
        const opt3 = Optional.Of('world');
        const opt4 = new Optional<string>();

        expect(opt1.IsEqual('hello')).toBe(true);
        expect(opt2.IsEqual('hello')).toBe(true);
        expect(opt3.IsEqual('hello')).toBe(false);
        expect(opt4.IsEqual(null)).toBe(true);
        expect(opt1.IsEqual(null)).toBe(false);
    });

    test('Pick throws error on empty Optional', () => {
        const opt = new Optional<string>();
        expect(() => opt.Pick()).toThrow();
    });
});