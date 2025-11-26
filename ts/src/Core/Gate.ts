/**
 * A Gate class similar to U++ Gate, providing a simple boolean-based gate mechanism.
 * The gate can be opened or closed and can be tested to see if it's open.
 * 
 * @example
 * ```typescript
 * const gate = new Gate();
 * console.log(gate.IsOpen()); // false
 * 
 * gate.Open();
 * console.log(gate.IsOpen()); // true
 * 
 * gate.Close();
 * console.log(gate.IsOpen()); // false
 * ```
 */
export class Gate {
    private isOpen: boolean;

    /**
     * Creates a new Gate instance.
     * @param initialState Optional initial state of the gate (default: false)
     */
    constructor(initialState: boolean = false) {
        this.isOpen = initialState;
    }

    /**
     * Opens the gate.
     * @returns This Gate instance for method chaining
     */
    Open(): this {
        this.isOpen = true;
        return this;
    }

    /**
     * Closes the gate.
     * @returns This Gate instance for method chaining
     */
    Close(): this {
        this.isOpen = false;
        return this;
    }

    /**
     * Checks if the gate is open.
     * @returns True if the gate is open, false otherwise
     */
    IsOpen(): boolean {
        return this.isOpen;
    }

    /**
     * Checks if the gate is closed.
     * @returns True if the gate is closed, false otherwise
     */
    IsClosed(): boolean {
        return !this.isOpen;
    }

    /**
     * Toggles the state of the gate.
     * @returns This Gate instance for method chaining
     */
    Toggle(): this {
        this.isOpen = !this.isOpen;
        return this;
    }

    /**
     * Sets the state of the gate.
     * @param state The state to set the gate to
     * @returns This Gate instance for method chaining
     */
    Set(state: boolean): this {
        this.isOpen = state;
        return this;
    }
}