import { Stream } from './Stream';
import * as fs from 'fs';
import * as path from 'path';

/**
 * Class for writing files, similar to U++ FileOut.
 * Implements the Stream interface for file output operations.
 */
export class FileOut extends Stream {
    private fd: number | null = null;
    private filePath: string = '';
    private position: number = 0;
    private append: boolean = false;

    /**
     * Creates a new FileOut instance for writing a file.
     * @param filename Path to the file to write
     * @param append Whether to append to the file (default: false)
     */
    constructor(filename?: string, append: boolean = false) {
        super();
        if (filename) {
            this.Open(filename, append);
        }
    }

    /**
     * Opens a file for writing.
     * @param filename Path to the file to write
     * @param append Whether to append to the file (default: false)
     * @returns True if the file was opened successfully, false otherwise
     */
    Open(filename: string, append: boolean = false): boolean {
        try {
            // Create directory if it doesn't exist
            const dir = path.dirname(filename);
            if (!fs.existsSync(dir)) {
                fs.mkdirSync(dir, { recursive: true });
            }

            // Determine flags based on append mode
            const flags = append ? 'a' : 'w';
            this.fd = fs.openSync(filename, flags);
            this.filePath = filename;
            this.append = append;
            
            // If not appending, position starts at 0
            // If appending, position starts at end of file
            if (!append) {
                this.position = 0;
            } else {
                const stats = fs.statSync(filename);
                this.position = stats.size;
            }
            
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
     * Reads are not supported for FileOut.
     * @param buffer Buffer to read data into
     * @param size Number of bytes to read
     * @returns 0, as reads are not supported
     */
    Read(buffer: Buffer, size: number): number {
        this.SetError('Read operation not supported on FileOut');
        return 0;
    }

    /**
     * Writes data from a buffer to the file.
     * @param buffer Buffer containing data to write
     * @param size Number of bytes to write
     * @returns Number of bytes actually written
     */
    Write(buffer: Buffer, size: number): number {
        if (this.fd === null) {
            this.SetError('File is not open');
            return 0;
        }

        try {
            // Ensure we only write the requested size
            const actualBuffer = buffer.subarray(0, size);
            const bytesWritten = fs.writeSync(this.fd, actualBuffer, 0, actualBuffer.length, this.position);
            this.position += bytesWritten;
            return bytesWritten;
        } catch (error) {
            this.SetError(`Write error: ${(error as Error).message}`);
            return 0;
        }
    }

    /**
     * Flushes any buffered data to the file.
     */
    Flush(): void {
        if (this.fd !== null) {
            try {
                fs.fsyncSync(this.fd);
            } catch (error) {
                this.SetError(`Flush error: ${(error as Error).message}`);
            }
        }
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
     * Note: This doesn't work well with append mode.
     * @param pos Position to seek to
     * @returns True if successful, false otherwise
     */
    Seek(pos: number): boolean {
        if (this.fd === null) {
            this.SetError('File is not open');
            return false;
        }

        // If in append mode, warn user that seeking may not work as expected
        if (this.append) {
            console.warn('Seeking in append mode may not behave as expected');
        }

        try {
            fs.writeSync(this.fd, Buffer.alloc(0), 0, 0, pos);
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
     * Asynchronously opens a file for writing.
     * @param filename Path to the file to write
     * @param append Whether to append to the file (default: false)
     * @returns Promise resolving to true if the file was opened successfully, false otherwise
     */
    async OpenAsync(filename: string, append: boolean = false): Promise<boolean> {
        try {
            // Create directory if it doesn't exist
            const dir = path.dirname(filename);
            if (!fs.existsSync(dir)) {
                await fs.promises.mkdir(dir, { recursive: true });
            }

            // Determine flags based on append mode
            const flags = append ? 'a' : 'w';
            this.fd = fs.openSync(filename, flags); // Using sync for consistency
            this.filePath = filename;
            this.append = append;

            // If not appending, position starts at 0
            // If appending, position starts at end of file
            if (!append) {
                this.position = 0;
            } else {
                const stats = fs.statSync(filename);
                this.position = stats.size;
            }

            this.ClearError();
            return true;
        } catch (error) {
            this.SetError(`Failed to open file: ${(error as Error).message}`);
            return false;
        }
    }

    /**
     * Asynchronously writes data from a buffer to the file.
     * @param buffer Buffer containing data to write
     * @param size Number of bytes to write
     * @returns Promise resolving to the number of bytes actually written
     */
    async WriteAsync(buffer: Buffer, size: number): Promise<number> {
        if (this.fd === null) {
            this.SetError('File is not open');
            return 0;
        }

        return new Promise((resolve, reject) => {
            try {
                // Using sync approach to maintain consistency with class design
                // but in a real implementation we could use async file operations
                const actualBuffer = buffer.subarray(0, size);
                const bytesWritten = fs.writeSync(this.fd!, actualBuffer, 0, actualBuffer.length, this.position);
                this.position += bytesWritten;
                resolve(bytesWritten);
            } catch (error) {
                this.SetError(`Write error: ${(error as Error).message}`);
                resolve(0);
            }
        });
    }

    /**
     * Asynchronously flushes any buffered data to the file.
     * @returns Promise that resolves when the flush is complete
     */
    async FlushAsync(): Promise<void> {
        if (this.fd !== null) {
            return new Promise((resolve, reject) => {
                try {
                    fs.fsyncSync(this.fd!); // Using sync for consistency
                    resolve();
                } catch (error) {
                    this.SetError(`Flush error: ${(error as Error).message}`);
                    reject(error);
                }
            });
        }
    }
}