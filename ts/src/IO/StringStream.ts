import { Stream } from './Stream';

/**
 * Class for string-based stream operations, similar to U++ StringStream.
 * Implements the Stream interface for in-memory string operations.
 */
export class StringStream extends Stream {
    private data: string;
    private position: number = 0;

    /**
     * Creates a new StringStream instance.
     * @param initialData Optional initial string data
     */
    constructor(initialData: string = '') {
        super();
        this.data = initialData;
    }

    /**
     * Checks if the stream is open (always true for StringStream).
     * @returns Always returns true
     */
    IsOpen(): boolean {
        return true;
    }

    /**
     * Closes the stream (no-op for StringStream).
     */
    Close(): void {
        // No operation needed for StringStream
    }

    /**
     * Reads data from the string into a buffer.
     * @param buffer Buffer to read data into
     * @param size Number of bytes to read
     * @returns Number of bytes actually read
     */
    Read(buffer: Buffer, size: number): number {
        if (this.position >= this.data.length) {
            return 0; // End of string
        }

        // Calculate how many characters we can read
        const available = this.data.length - this.position;
        const toRead = Math.min(size, available);

        // Convert the substring to bytes and copy to buffer
        const subStr = this.data.substring(this.position, this.position + toRead);
        const bytes = Buffer.from(subStr, 'utf-8');
        
        // Copy to the provided buffer
        bytes.copy(buffer, 0, 0, Math.min(bytes.length, buffer.length));
        
        // Update position - advance by the number of UTF-8 bytes
        const bytesRead = bytes.length;
        this.position += subStr.length; // Move position by character count
        
        return bytesRead;
    }

    /**
     * Writes data from a buffer to the string.
     * @param buffer Buffer containing data to write
     * @param size Number of bytes to write
     * @returns Number of bytes actually written
     */
    Write(buffer: Buffer, size: number): number {
        try {
            // Convert the buffer to string and append to our data
            const dataToWrite = buffer.subarray(0, size).toString('utf-8');
            this.data += dataToWrite;
            return dataToWrite.length;
        } catch (error) {
            this.SetError(`Write error: ${(error as Error).message}`);
            return 0;
        }
    }

    /**
     * Gets the current position in the string.
     * @returns Current position in the string
     */
    GetPosition(): number {
        return this.position;
    }

    /**
     * Sets the position in the string.
     * @param pos Position to seek to
     * @returns True if successful (always true for valid positions)
     */
    Seek(pos: number): boolean {
        if (pos < 0 || pos > this.data.length) {
            this.SetError(`Invalid position: ${pos}`);
            return false;
        }
        
        this.position = pos;
        return true;
    }

    /**
     * Gets the size of the string.
     * @returns Size of the string in characters
     */
    GetSize(): number {
        return this.data.length;
    }

    /**
     * Checks if the current position is at the end of the string.
     * @returns True if at end of string, false otherwise
     */
    IsEof(): boolean {
        return this.position >= this.data.length;
    }

    /**
     * Gets the content of the string stream.
     * @returns The current string content
     */
    GetString(): string {
        return this.data;
    }

    /**
     * Sets the content of the string stream, resetting the position to 0.
     * @param str The new string content
     */
    SetString(str: string): void {
        this.data = str;
        this.position = 0; // Reset position when setting new string
        this.ClearError();
    }

    /**
     * Clears the content of the string stream, resetting the position to 0.
     */
    Clear(): void {
        this.data = '';
        this.position = 0;
        this.ClearError();
    }

    /**
     * Reads the remaining content of the string stream.
     * @returns The remaining string content from the current position
     */
    ReadAll(): string {
        const remaining = this.data.substring(this.position);
        this.position = this.data.length; // Move to end
        return remaining;
    }
}