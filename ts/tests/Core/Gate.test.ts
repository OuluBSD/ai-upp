import { Gate } from '../../src/Core/Gate';

describe('Gate', () => {
    test('constructor creates closed gate by default', () => {
        const gate = new Gate();
        expect(gate.IsClosed()).toBe(true);
        expect(gate.IsOpen()).toBe(false);
    });

    test('constructor creates gate with specified initial state', () => {
        const openGate = new Gate(true);
        expect(openGate.IsOpen()).toBe(true);
        expect(openGate.IsClosed()).toBe(false);

        const closedGate = new Gate(false);
        expect(closedGate.IsClosed()).toBe(true);
        expect(closedGate.IsOpen()).toBe(false);
    });

    test('Open method opens the gate', () => {
        const gate = new Gate();
        expect(gate.IsClosed()).toBe(true);

        gate.Open();
        expect(gate.IsOpen()).toBe(true);
    });

    test('Close method closes the gate', () => {
        const gate = new Gate(true); // Start open
        expect(gate.IsOpen()).toBe(true);

        gate.Close();
        expect(gate.IsClosed()).toBe(true);
    });

    test('IsOpen and IsClosed return correct values', () => {
        const gate = new Gate();

        expect(gate.IsClosed()).toBe(true);
        expect(gate.IsOpen()).toBe(false);

        gate.Open();
        expect(gate.IsOpen()).toBe(true);
        expect(gate.IsClosed()).toBe(false);

        gate.Close();
        expect(gate.IsClosed()).toBe(true);
        expect(gate.IsOpen()).toBe(false);
    });

    test('Toggle switches the gate state', () => {
        const gate = new Gate();

        expect(gate.IsClosed()).toBe(true);
        gate.Toggle();
        expect(gate.IsOpen()).toBe(true);
        gate.Toggle();
        expect(gate.IsClosed()).toBe(true);
    });

    test('Set sets the gate to specified state', () => {
        const gate = new Gate();

        gate.Set(true);
        expect(gate.IsOpen()).toBe(true);

        gate.Set(false);
        expect(gate.IsClosed()).toBe(true);
    });

    test('method chaining works correctly', () => {
        const gate = new Gate();

        expect(gate.Open()).toBe(gate);
        expect(gate.Close()).toBe(gate);
        expect(gate.Toggle()).toBe(gate);
        expect(gate.Set(true)).toBe(gate);
        expect(gate.Set(false)).toBe(gate);
    });
});