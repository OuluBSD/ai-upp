import { Callback } from '../../src/Core/Callback';

describe('Callback', () => {
    test('constructor creates Callback with provided function', () => {
        const greet = (name: string): string => `Hello, ${name}!`;
        const callback = new Callback<(name: string) => string>(greet);
        
        expect(callback.Execute('World')).toBe('Hello, World!');
    });

    test('IsNull returns correct value', () => {
        const greet = (name: string): string => `Hello, ${name}!`;
        const callback = new Callback<(name: string) => string>(greet);
        expect(callback.IsNull()).toBe(false);

        const nullCallback = new Callback<(name: string) => string>();
        expect(nullCallback.IsNull()).toBe(true);
    });

    test('Execute calls the stored callback', () => {
        const square = (x: number): number => x * x;
        const callback = new Callback<(x: number) => number>(square);
        
        expect(callback.Execute(5)).toBe(25);
    });

    test('Set updates the stored callback', () => {
        const square = (x: number): number => x * x;
        const cube = (x: number): number => x * x * x;
        
        const callback = new Callback<(x: number) => number>(square);
        expect(callback.Execute(3)).toBe(9);

        callback.Set(cube);
        expect(callback.Execute(3)).toBe(27);
    });

    test('Clear makes the Callback null', () => {
        const square = (x: number): number => x * x;
        const callback = new Callback<(x: number) => number>(square);
        
        callback.Clear();
        expect(callback.IsNull()).toBe(true);
    });

    test('Static Create method creates Callback', () => {
        const multiply = (a: number, b: number): number => a * b;
        const callback = Callback.Create(multiply);
        
        expect(callback.Execute(4, 5)).toBe(20);
    });

    test('Execute throws error for null callback', () => {
        const callback = new Callback<(name: string) => string>();
        expect(() => callback.Execute('World')).toThrow();
    });

    test('Pick transfers ownership', () => {
        const add = (a: number, b: number): number => a + b;
        const callback = new Callback<(a: number, b: number) => number>(add);
        
        const storedCallback = callback.Pick();
        expect(storedCallback(2, 3)).toBe(5);
        expect(callback.IsNull()).toBe(true);
    });

    test('Pick throws error on null callback', () => {
        const callback = new Callback<(name: string) => string>();
        expect(() => callback.Pick()).toThrow();
    });

    test('can be created and executed', () => {
        const greet = (name: string): string => `Hello, ${name}!`;
        const callback = new Callback<(name: string) => string>(greet);
        expect(callback.Execute('World')).toBe('Hello, World!');
    });
});