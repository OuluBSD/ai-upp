/**
 * Base class for stream operations, similar to U++ Stream.
 * Provides a common interface for different types of streams.
 */
export abstract class Stream {
    protected error: boolean = false;
    protected errorMsg: string = '';

    /**
     * Checks if the stream is in an error state.
     * @returns True if the stream has encountered an error, false otherwise
     */
    IsError(): boolean {
        return this.error;
    }

    /**
     * Gets the error message if the stream is in an error state.
     * @returns The error message
     */
    GetError(): string {
        return this.errorMsg;
    }

    /**
     * Sets the error state and message.
     * @param msg The error message
     */
    protected SetError(msg: string): void {
        this.error = true;
        this.errorMsg = msg;
    }

    /**
     * Clears the error state.
     */
    ClearError(): void {
        this.error = false;
        this.errorMsg = '';
    }

    /**
     * Abstract method to check if the stream is open.
     */
    abstract IsOpen(): boolean;

    /**
     * Abstract method to close the stream.
     */
    abstract Close(): void;

    /**
     * Abstract method to read data from the stream.
     * @param buffer Buffer to read data into
     * @param size Number of bytes to read
     * @returns Number of bytes actually read
     */
    abstract Read(buffer: Buffer, size: number): number;

    /**
     * Abstract method to write data to the stream.
     * @param buffer Buffer containing data to write
     * @param size Number of bytes to write
     * @returns Number of bytes actually written
     */
    abstract Write(buffer: Buffer, size: number): number;

    /**
     * Reads a line from the stream.
     * This is a default implementation that can be overridden by derived classes.
     * @returns The line read as a string, or null if end of stream
     */
    ReadLine(): string | null {
        let line = '';
        const buffer = Buffer.alloc(1);

        while (this.Read(buffer, 1) === 1) {
            const char = String.fromCharCode(buffer[0]);
            if (char === '\n') {
                // Remove potential carriage return
                if (line.endsWith('\r')) {
                    line = line.slice(0, -1);
                }
                return line;
            }
            line += char;
        }

        // Return the line if we have one, or null if EOF and no data
        return line.length > 0 ? line : null;
    }

    /**
     * Reads the entire stream content as a string.
     * This is a default implementation that can be overridden by derived classes.
     * @returns The entire content as a string
     */
    ReadAll(): string {
        const chunks: Buffer[] = [];
        const buffer = Buffer.alloc(1024);
        let bytesRead: number;

        while ((bytesRead = this.Read(buffer, buffer.length)) > 0) {
            chunks.push(Buffer.from(buffer.subarray(0, bytesRead)));
        }

        return Buffer.concat(chunks).toString('utf-8');
    }

    /**
     * Writes a string to the stream.
     * This is a default implementation that can be overridden by derived classes.
     * @param str The string to write
     * @returns Number of bytes written
     */
    WriteString(str: string): number {
        const buffer = Buffer.from(str, 'utf-8');
        return this.Write(buffer, buffer.length);
    }

    /**
     * Asynchronously reads data from the stream.
     * @param buffer Buffer to read data into
     * @param size Number of bytes to read
     * @returns Promise resolving to the number of bytes actually read
     */
    async ReadAsync(buffer: Buffer, size: number): Promise<number> {
        return new Promise((resolve, reject) => {
            try {
                const bytesRead = this.Read(buffer, size);
                resolve(bytesRead);
            } catch (error) {
                reject(error);
            }
        });
    }

    /**
     * Asynchronously writes data to the stream.
     * @param buffer Buffer containing data to write
     * @param size Number of bytes to write
     * @returns Promise resolving to the number of bytes actually written
     */
    async WriteAsync(buffer: Buffer, size: number): Promise<number> {
        return new Promise((resolve, reject) => {
            try {
                const bytesWritten = this.Write(buffer, size);
                resolve(bytesWritten);
            } catch (error) {
                reject(error);
            }
        });
    }

    /**
     * Asynchronously reads the entire stream content as a string.
     * @returns Promise resolving to the entire content as a string
     */
    async ReadAllAsync(): Promise<string> {
        return new Promise((resolve, reject) => {
            try {
                const content = this.ReadAll();
                resolve(content);
            } catch (error) {
                reject(error);
            }
        });
    }

    /**
     * Asynchronously writes a string to the stream.
     * @param str The string to write
     * @returns Promise resolving to the number of bytes written
     */
    async WriteStringAsync(str: string): Promise<number> {
        return new Promise((resolve, reject) => {
            try {
                const bytesWritten = this.WriteString(str);
                resolve(bytesWritten);
            } catch (error) {
                reject(error);
            }
        });
    }
}