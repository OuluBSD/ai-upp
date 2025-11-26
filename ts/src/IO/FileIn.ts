import { Stream } from './Stream';
import * as fs from 'fs';
import * as path from 'path';

/**
 * Class for reading files, similar to U++ FileIn.
 * Implements the Stream interface for file input operations.
 */
export class FileIn extends Stream {
    private fd: number | null = null;
    private filePath: string = '';
    private position: number = 0;

    /**
     * Creates a new FileIn instance for reading a file.
     * @param filename Path to the file to read
     */
    constructor(filename?: string) {
        super();
        if (filename) {
            this.Open(filename);
        }
    }

    /**
     * Opens a file for reading.
     * @param filename Path to the file to read
     * @returns True if the file was opened successfully, false otherwise
     */
    Open(filename: string): boolean {
        try {
            // Check if file exists
            if (!fs.existsSync(filename)) {
                this.SetError(`File does not exist: ${filename}`);
                return false;
            }

            // Check if it's a file (not a directory)
            const stats = fs.statSync(filename);
            if (!stats.isFile()) {
                this.SetError(`Path is not a file: ${filename}`);
                return false;
            }

            this.fd = fs.openSync(filename, 'r');
            this.filePath = filename;
            this.position = 0;
            this.ClearError();
            return true;
        } catch (error) {
            this.SetError(`Failed to open file: ${(error as Error).message}`);
            return false;
        }
    }

    /**
     * Checks if the file is currently open.
     * @returns True if the file is open, false otherwise
     */
    IsOpen(): boolean {
        return this.fd !== null;
    }

    /**
     * Closes the file if it's open.
     */
    Close(): void {
        if (this.fd !== null) {
            try {
                fs.closeSync(this.fd);
                this.fd = null;
            } catch (error) {
                this.SetError(`Failed to close file: ${(error as Error).message}`);
            }
        }
    }

    /**
     * Reads data from the file into a buffer.
     * @param buffer Buffer to read data into
     * @param size Number of bytes to read
     * @returns Number of bytes actually read
     */
    Read(buffer: Buffer, size: number): number {
        if (this.fd === null) {
            this.SetError('File is not open');
            return 0;
        }

        try {
            // Using type assertion since TypeScript can't guarantee fd is not null here
            const bytesRead = fs.readSync(this.fd!, buffer, 0, size, this.position);
            this.position += bytesRead;
            return bytesRead;
        } catch (error) {
            this.SetError(`Read error: ${(error as Error).message}`);
            return 0;
        }
    }

    /**
     * Writes are not supported for FileIn.
     * @param buffer Buffer containing data to write
     * @param size Number of bytes to write
     * @returns 0, as writes are not supported
     */
    Write(buffer: Buffer, size: number): number {
        this.SetError('Write operation not supported on FileIn');
        return 0;
    }

    /**
     * Gets the current position in the file.
     * @returns Current position in the file, or -1 if error
     */
    GetPosition(): number {
        if (this.fd === null) {
            return -1;
        }
        return this.position;
    }

    /**
     * Sets the position in the file.
     * @param pos Position to seek to
     * @returns True if successful, false otherwise
     */
    Seek(pos: number): boolean {
        if (this.fd === null) {
            this.SetError('File is not open');
            return false;
        }

        try {
            fs.readSync(this.fd, Buffer.alloc(0), 0, 0, pos);
            this.position = pos;
            return true;
        } catch (error) {
            this.SetError(`Seek error: ${(error as Error).message}`);
            return false;
        }
    }

    /**
     * Gets the size of the file.
     * @returns Size of the file in bytes, or -1 if error
     */
    GetSize(): number {
        if (this.filePath === '' || !fs.existsSync(this.filePath)) {
            return -1;
        }

        try {
            const stats = fs.statSync(this.filePath);
            return stats.size;
        } catch (error) {
            this.SetError(`GetSize error: ${(error as Error).message}`);
            return -1;
        }
    }

    /**
     * Checks if the current position is at the end of the file.
     * @returns True if at end of file, false otherwise
     */
    IsEof(): boolean {
        if (this.fd === null) {
            return true;
        }

        try {
            const size = this.GetSize();
            return size !== -1 && this.position >= size;
        } catch {
            return true;
        }
    }

    /**
     * Reads the entire file content as a string.
     * @returns The file content as a string
     */
    ReadAll(): string {
        if (this.filePath === '') {
            this.SetError('No file path specified');
            return '';
        }

        try {
            // Use fs.readFileSync to get the content
            const content = fs.readFileSync(this.filePath, 'utf-8');
            // Update position to end of file to maintain consistency
            const stats = fs.statSync(this.filePath);
            this.position = stats.size;
            return content;
        } catch (error) {
            this.SetError(`ReadAll error: ${(error as Error).message}`);
            return '';
        }
    }

    /**
     * Asynchronously opens a file for reading.
     * @param filename Path to the file to read
     * @returns Promise resolving to true if the file was opened successfully, false otherwise
     */
    async OpenAsync(filename: string): Promise<boolean> {
        try {
            // Check if file exists
            if (!fs.existsSync(filename)) {
                this.SetError(`File does not exist: ${filename}`);
                return false;
            }

            // Check if it's a file (not a directory)
            const stats = await fs.promises.stat(filename);
            if (!stats.isFile()) {
                this.SetError(`Path is not a file: ${filename}`);
                return false;
            }

            this.fd = fs.openSync(filename, 'r'); // Note: Using sync open for consistency
            this.filePath = filename;
            this.position = 0;
            this.ClearError();
            return true;
        } catch (error) {
            this.SetError(`Failed to open file: ${(error as Error).message}`);
            return false;
        }
    }

    /**
     * Asynchronously reads data from the file into a buffer.
     * @param buffer Buffer to read data into
     * @param size Number of bytes to read
     * @returns Promise resolving to the number of bytes actually read
     */
    async ReadAsync(buffer: Buffer, size: number): Promise<number> {
        if (this.fd === null) {
            this.SetError('File is not open');
            return 0;
        }

        return new Promise((resolve, reject) => {
            try {
                // Using sync approach to maintain consistency with class design
                // but in a real implementation we could use async file operations
                const bytesRead = fs.readSync(this.fd!, buffer, 0, size, this.position);
                this.position += bytesRead;
                resolve(bytesRead);
            } catch (error) {
                this.SetError(`Read error: ${(error as Error).message}`);
                resolve(0);
            }
        });
    }

    /**
     * Asynchronously reads the entire file content as a string.
     * @returns Promise resolving to the file content as a string
     */
    async ReadAllAsync(): Promise<string> {
        if (this.filePath === '') {
            this.SetError('No file path specified');
            return '';
        }

        try {
            const content = await fs.promises.readFile(this.filePath, 'utf-8');
            return content;
        } catch (error) {
            this.SetError(`ReadAllAsync error: ${(error as Error).message}`);
            return '';
        }
    }
}