import { Function } from '../../src/Core/Function';

describe('Function', () => {
    test('constructor creates Function with provided function', () => {
        const add = (a: number, b: number): number => a + b;
        const func = new Function<(a: number, b: number) => number>(add);
        
        expect(func.Execute(2, 3)).toBe(5);
    });

    test('IsNull returns correct value', () => {
        const add = (a: number, b: number): number => a + b;
        const func = new Function<(a: number, b: number) => number>(add);
        expect(func.IsNull()).toBe(false);

        const nullFunc = new Function<(a: number, b: number) => number>();
        expect(nullFunc.IsNull()).toBe(true);
    });

    test('Execute calls the stored function', () => {
        const multiply = (a: number, b: number): number => a * b;
        const func = new Function<(a: number, b: number) => number>(multiply);
        
        expect(func.Execute(3, 4)).toBe(12);
    });

    test('Set updates the stored function', () => {
        const add = (a: number, b: number): number => a + b;
        const multiply = (a: number, b: number): number => a * b;
        
        const func = new Function<(a: number, b: number) => number>(add);
        expect(func.Execute(2, 3)).toBe(5);

        func.Set(multiply);
        expect(func.Execute(2, 3)).toBe(6);
    });

    test('Clear makes the Function null', () => {
        const add = (a: number, b: number): number => a + b;
        const func = new Function<(a: number, b: number) => number>(add);
        
        func.Clear();
        expect(func.IsNull()).toBe(true);
    });

    test('Static Create method creates Function', () => {
        const add = (a: number, b: number): number => a + b;
        const func = Function.Create(add);
        
        expect(func.Execute(2, 3)).toBe(5);
    });

    test('Execute throws error for null function', () => {
        const func = new Function<(a: number, b: number) => number>();
        expect(() => func.Execute(2, 3)).toThrow();
    });

    test('Pick transfers ownership', () => {
        const add = (a: number, b: number): number => a + b;
        const func = new Function<(a: number, b: number) => number>(add);
        
        const storedFunc = func.Pick();
        expect(storedFunc(2, 3)).toBe(5);
        expect(func.IsNull()).toBe(true);
    });

    test('Pick throws error on null function', () => {
        const func = new Function<(a: number, b: number) => number>();
        expect(() => func.Pick()).toThrow();
    });
});