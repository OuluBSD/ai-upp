import * as fs from 'fs';
import * as path from 'path';

/**
 * A class similar to U++ FindFile for directory traversal.
 */
export class FindFile {
    private dirPath: string;
    private entries: fs.Dirent[];
    private index: number;

    constructor(dirPath: string) {
        this.dirPath = dirPath;
        try {
            this.entries = fs.readdirSync(dirPath, { withFileTypes: true });
            this.index = 0;
        } catch (error) {
            this.entries = [];
            this.index = 0;
        }
    }

    /**
     * Moves to the next entry in the directory.
     * @returns True if there's a next entry, false otherwise
     */
    Next(): boolean {
        if (this.index < this.entries.length) {
            this.index++;
            return true;
        }
        return false;
    }

    /**
     * Checks if the current entry is a file.
     * @returns True if the current entry is a file, false otherwise
     */
    IsFile(): boolean {
        if (this.index <= 0 || this.index > this.entries.length) {
            return false;
        }
        return this.entries[this.index - 1].isFile();
    }

    /**
     * Checks if the current entry is a directory.
     * @returns True if the current entry is a directory, false otherwise
     */
    IsDir(): boolean {
        if (this.index <= 0 || this.index > this.entries.length) {
            return false;
        }
        return this.entries[this.index - 1].isDirectory();
    }

    /**
     * Gets the name of the current entry.
     * @returns The name of the current entry
     */
    GetName(): string {
        if (this.index <= 0 || this.index > this.entries.length) {
            return '';
        }
        return this.entries[this.index - 1].name;
    }

    /**
     * Gets the full path of the current entry.
     * @returns The full path of the current entry
     */
    GetPath(): string {
        if (this.index <= 0 || this.index > this.entries.length) {
            return '';
        }
        return path.join(this.dirPath, this.entries[this.index - 1].name);
    }
}

/**
 * File system utility functions, similar to U++ FileSystem operations.
 */
export class FileSystem {
    /**
     * Checks if a file exists.
     * @param filePath The path to the file
     * @returns True if the file exists, false otherwise
     */
    static FileExists(filePath: string): boolean {
        try {
            return fs.existsSync(filePath) && fs.statSync(filePath).isFile();
        } catch {
            return false;
        }
    }

    /**
     * Checks if a directory exists.
     * @param dirPath The path to the directory
     * @returns True if the directory exists, false otherwise
     */
    static DirectoryExists(dirPath: string): boolean {
        try {
            return fs.existsSync(dirPath) && fs.statSync(dirPath).isDirectory();
        } catch {
            return false;
        }
    }

    /**
     * Creates a directory and any necessary parent directories.
     * @param dirPath The path to the directory to create
     * @returns True if successful, false otherwise
     */
    static CreateDirectory(dirPath: string): boolean {
        try {
            fs.mkdirSync(dirPath, { recursive: true });
            return true;
        } catch (error) {
            console.error(`Error creating directory ${dirPath}: ${(error as Error).message}`);
            return false;
        }
    }

    /**
     * Removes a file.
     * @param filePath The path to the file to remove
     * @returns True if successful, false otherwise
     */
    static RemoveFile(filePath: string): boolean {
        try {
            fs.unlinkSync(filePath);
            return true;
        } catch (error) {
            console.error(`Error removing file ${filePath}: ${(error as Error).message}`);
            return false;
        }
    }

    /**
     * Removes a directory (must be empty).
     * @param dirPath The path to the directory to remove
     * @returns True if successful, false otherwise
     */
    static RemoveDirectory(dirPath: string): boolean {
        try {
            fs.rmdirSync(dirPath);
            return true;
        } catch (error) {
            console.error(`Error removing directory ${dirPath}: ${(error as Error).message}`);
            return false;
        }
    }

    /**
     * Copies a file from source to destination.
     * @param sourcePath The path to the source file
     * @param destPath The path to the destination file
     * @returns True if successful, false otherwise
     */
    static CopyFile(sourcePath: string, destPath: string): boolean {
        try {
            // Ensure destination directory exists
            const destDir = path.dirname(destPath);
            if (!this.DirectoryExists(destDir)) {
                this.CreateDirectory(destDir);
            }
            
            fs.copyFileSync(sourcePath, destPath);
            return true;
        } catch (error) {
            console.error(`Error copying file from ${sourcePath} to ${destPath}: ${(error as Error).message}`);
            return false;
        }
    }

    /**
     * Moves or renames a file.
     * @param oldPath The current path to the file
     * @param newPath The new path for the file
     * @returns True if successful, false otherwise
     */
    static MoveFile(oldPath: string, newPath: string): boolean {
        try {
            fs.renameSync(oldPath, newPath);
            return true;
        } catch (error) {
            console.error(`Error moving file from ${oldPath} to ${newPath}: ${(error as Error).message}`);
            return false;
        }
    }

    /**
     * Gets the size of a file in bytes.
     * @param filePath The path to the file
     * @returns The size of the file in bytes, or -1 if error
     */
    static GetFileSize(filePath: string): number {
        try {
            const stats = fs.statSync(filePath);
            return stats.size;
        } catch (error) {
            console.error(`Error getting file size for ${filePath}: ${(error as Error).message}`);
            return -1;
        }
    }

    /**
     * Gets the last modification time of a file.
     * @param filePath The path to the file
     * @returns The last modification time as a Date object, or null if error
     */
    static GetFileTime(filePath: string): Date | null {
        try {
            const stats = fs.statSync(filePath);
            return stats.mtime;
        } catch (error) {
            console.error(`Error getting file time for ${filePath}: ${(error as Error).message}`);
            return null;
        }
    }
}