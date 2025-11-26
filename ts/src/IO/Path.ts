import * as path from 'path';

/**
 * Path manipulation utilities, similar to U++ Path operations.
 */
export class Path {
    private pathValue: string;

    constructor(pathStr: string = '') {
        this.pathValue = pathStr;
    }

    /**
     * Gets the string representation of the path.
     * @returns The path string
     */
    ToString(): string {
        return this.pathValue;
    }

    /**
     * Sets the path value.
     * @param pathStr The new path string
     * @returns This Path instance for method chaining
     */
    Set(pathStr: string): Path {
        this.pathValue = pathStr;
        return this;
    }

    /**
     * Gets the directory name of the path.
     * @returns The directory name
     */
    GetDirectory(): string {
        return path.dirname(this.pathValue);
    }

    /**
     * Gets the file name of the path.
     * @returns The file name with extension
     */
    GetFileName(): string {
        return path.basename(this.pathValue);
    }

    /**
     * Gets the file name without extension.
     * @returns The file name without extension
     */
    GetFileNameWithoutExtension(): string {
        const baseName = path.basename(this.pathValue);
        const ext = path.extname(baseName);
        return baseName.slice(0, -ext.length);
    }

    /**
     * Gets the file extension.
     * @returns The file extension (including the dot)
     */
    GetExtension(): string {
        return path.extname(this.pathValue);
    }

    /**
     * Changes the file extension.
     * @param newExtension The new extension (should include the dot)
     * @returns A new Path instance with the changed extension
     */
    ChangeExtension(newExtension: string): Path {
        const dir = path.dirname(this.pathValue);
        const nameWithoutExt = this.GetFileNameWithoutExtension();
        return new Path(path.join(dir, nameWithoutExt + newExtension));
    }

    /**
     * Appends a subpath to the current path.
     * @param subPath The subpath to append
     * @returns A new Path instance with the appended path
     */
    Append(subPath: string): Path {
        return new Path(path.join(this.pathValue, subPath));
    }

    /**
     * Normalizes the path (resolves '..' and '.' segments).
     * @returns A new Path instance with the normalized path
     */
    Normalize(): Path {
        return new Path(path.normalize(this.pathValue));
    }

    /**
     * Checks if the path is absolute.
     * @returns True if the path is absolute, false otherwise
     */
    IsAbsolute(): boolean {
        return path.isAbsolute(this.pathValue);
    }

    /**
     * Makes the path absolute.
     * @param basePath The base path to use if the path is relative (defaults to current working directory)
     * @returns A new Path instance with the absolute path
     */
    MakeAbsolute(basePath: string = process.cwd()): Path {
        if (this.IsAbsolute()) {
            return new Path(this.pathValue);
        }
        return new Path(path.resolve(basePath, this.pathValue));
    }

    /**
     * Makes the path relative to a base path.
     * @param basePath The base path to make relative to
     * @returns A new Path instance with the relative path
     */
    MakeRelative(basePath: string): Path {
        return new Path(path.relative(basePath, this.pathValue));
    }

    /**
     * Checks if the path represents the same file or directory as another path.
     * @param otherPath The other path to compare to
     * @returns True if the paths represent the same file or directory, false otherwise
     */
    IsSameAs(otherPath: Path): boolean {
        try {
            const resolvedThis = path.resolve(this.pathValue);
            const resolvedOther = path.resolve(otherPath.pathValue);
            return resolvedThis === resolvedOther;
        } catch {
            return false;
        }
    }

    /**
     * Combines two paths.
     * @param path1 First path
     * @param path2 Second path
     * @returns A new Path instance with the combined paths
     */
    static Combine(path1: string, path2: string): Path {
        return new Path(path.join(path1, path2));
    }

    /**
     * Gets the path separator for the current platform.
     * @returns The path separator string
     */
    static GetSeparator(): string {
        return path.sep;
    }

    /**
     * Gets the path delimiter for the current platform.
     * @returns The path delimiter string
     */
    static GetDelimiter(): string {
        return path.delimiter;
    }

    /**
     * Checks if a string is a valid path.
     * @param pathStr The string to check
     * @returns True if the string is a valid path, false otherwise
     */
    static IsValid(pathStr: string): boolean {
        try {
            // Basic validation: path.normalize should not throw and should result in a reasonable path
            const normalized = path.normalize(pathStr);
            return normalized === pathStr || normalized === path.join(process.cwd(), pathStr);
        } catch {
            return false;
        }
    }
}