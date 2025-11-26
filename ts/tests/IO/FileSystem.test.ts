import { FileSystem, FindFile } from '../../src/IO/FileSystem';
import * as fs from 'fs';
import * as path from 'path';

describe('FileSystem', () => {
    const testDir = path.join(__dirname, 'test_fs');
    const testFile = path.join(testDir, 'test_file.txt');
    const testSubDir = path.join(testDir, 'subdir');

    beforeAll(() => {
        // Create test directory structure
        if (!fs.existsSync(testDir)) {
            fs.mkdirSync(testDir, { recursive: true });
        }
        
        // Create a test file
        fs.writeFileSync(testFile, 'Test content', 'utf-8');
        
        // Create a subdirectory
        if (!fs.existsSync(testSubDir)) {
            fs.mkdirSync(testSubDir);
        }
    });

    afterAll(() => {
        // Clean up
        if (fs.existsSync(testFile)) {
            fs.unlinkSync(testFile);
        }
        
        if (fs.existsSync(testSubDir)) {
            fs.rmdirSync(testSubDir);
        }
        
        if (fs.existsSync(testDir)) {
            fs.rmdirSync(testDir);
        }
    });

    test('FileExists returns true for existing file', () => {
        expect(FileSystem.FileExists(testFile)).toBe(true);
    });

    test('FileExists returns false for non-existing file', () => {
        expect(FileSystem.FileExists(path.join(testDir, 'nonexistent.txt'))).toBe(false);
    });

    test('DirectoryExists returns true for existing directory', () => {
        expect(FileSystem.DirectoryExists(testDir)).toBe(true);
    });

    test('DirectoryExists returns false for non-existing directory', () => {
        expect(FileSystem.DirectoryExists(path.join(testDir, 'nonexistent'))).toBe(false);
    });

    test('CreateDirectory creates a directory', () => {
        const newDir = path.join(testDir, 'new_dir');
        const result = FileSystem.CreateDirectory(newDir);
        expect(result).toBe(true);
        expect(fs.existsSync(newDir)).toBe(true);
        
        // Clean up
        fs.rmdirSync(newDir);
    });

    test('RemoveFile removes a file', () => {
        const tempFile = path.join(testDir, 'temp.txt');
        fs.writeFileSync(tempFile, 'temp content', 'utf-8');
        
        const result = FileSystem.RemoveFile(tempFile);
        expect(result).toBe(true);
        expect(fs.existsSync(tempFile)).toBe(false);
    });

    test('RemoveDirectory removes a directory', () => {
        const tempDir = path.join(testDir, 'temp_dir');
        fs.mkdirSync(tempDir);
        
        const result = FileSystem.RemoveDirectory(tempDir);
        expect(result).toBe(true);
        expect(fs.existsSync(tempDir)).toBe(false);
    });

    test('CopyFile copies a file', () => {
        const sourceFile = path.join(testDir, 'source.txt');
        const destFile = path.join(testDir, 'dest.txt');
        
        fs.writeFileSync(sourceFile, 'source content', 'utf-8');
        
        const result = FileSystem.CopyFile(sourceFile, destFile);
        expect(result).toBe(true);
        expect(fs.existsSync(destFile)).toBe(true);
        expect(fs.readFileSync(destFile, 'utf-8')).toBe('source content');
        
        // Clean up
        fs.unlinkSync(sourceFile);
        fs.unlinkSync(destFile);
    });

    test('MoveFile moves a file', () => {
        const sourceFile = path.join(testDir, 'move_source.txt');
        const destFile = path.join(testDir, 'move_dest.txt');
        
        fs.writeFileSync(sourceFile, 'move content', 'utf-8');
        
        const result = FileSystem.MoveFile(sourceFile, destFile);
        expect(result).toBe(true);
        expect(fs.existsSync(sourceFile)).toBe(false);
        expect(fs.existsSync(destFile)).toBe(true);
        expect(fs.readFileSync(destFile, 'utf-8')).toBe('move content');
        
        // Clean up
        fs.unlinkSync(destFile);
    });

    test('GetFileSize returns correct size', () => {
        fs.writeFileSync(testFile, 'Test content', 'utf-8');
        const size = FileSystem.GetFileSize(testFile);
        expect(size).toBe(12); // Length of "Test content"
    });

    test('GetFileTime returns a Date object or null', () => {
        const time = FileSystem.GetFileTime(testFile);
        // The result should either be a Date object or null
        // Using more reliable date checking
        const isDate = time !== null && time !== undefined &&
                      (typeof time.getMonth === 'function' && typeof time.getFullYear === 'function');
        expect(isDate || time === null).toBe(true);
    });
});

describe('FindFile', () => {
    const testDir = path.join(__dirname, 'findfile_test');
    const testFile = path.join(testDir, 'test.txt');
    const testSubDir = path.join(testDir, 'subdir');

    beforeAll(() => {
        // Create test directory structure
        if (!fs.existsSync(testDir)) {
            fs.mkdirSync(testDir, { recursive: true });
        }
        
        // Create a test file
        fs.writeFileSync(testFile, 'Test content', 'utf-8');
        
        // Create a subdirectory
        if (!fs.existsSync(testSubDir)) {
            fs.mkdirSync(testSubDir);
        }
    });

    afterAll(() => {
        // Clean up
        if (fs.existsSync(testFile)) {
            fs.unlinkSync(testFile);
        }
        
        if (fs.existsSync(testSubDir)) {
            fs.rmdirSync(testSubDir);
        }
        
        if (fs.existsSync(testDir)) {
            fs.rmdirSync(testDir);
        }
    });

    test('FindFile iterates through directory entries', () => {
        const finder = new FindFile(testDir);
        
        // First entry
        expect(finder.Next()).toBe(true);
        const firstEntry = finder.GetName();
        expect(firstEntry).toMatch(/^(test\.txt|subdir)$/);
        
        // Check if it's a file or directory
        if (firstEntry === 'test.txt') {
            expect(finder.IsFile()).toBe(true);
            expect(finder.IsDir()).toBe(false);
        } else {
            expect(finder.IsDir()).toBe(true);
            expect(finder.IsFile()).toBe(false);
        }
        
        // Second entry
        expect(finder.Next()).toBe(true);
        const secondEntry = finder.GetName();
        expect(secondEntry).toMatch(/^(test\.txt|subdir)$/);
        expect(secondEntry).not.toBe(firstEntry);
        
        // Check that Next() returns false when no more entries
        const nextResult = finder.Next(); // Should return false now
        expect(nextResult).toBe(false);

        // Reset finder to go through again and count entries
        const countFinder = new FindFile(testDir);
        let count = 0;
        while (countFinder.Next()) {
            count++;
        }
        // Should have gone through all entries
        expect(count).toBe(2); // We have 2 entries: test.txt and subdir

        // Reset finder to go through again and collect entries
        const entryFinder = new FindFile(testDir);
        const entries = [];
        while (entryFinder.Next()) {
            entries.push({
                name: entryFinder.GetName(),
                isFile: entryFinder.IsFile(),
                isDir: entryFinder.IsDir()
            });
        }

        // Should have both the file and directory
        expect(entries.length).toBe(2);
        const hasFile = entries.some(entry => entry.name === 'test.txt' && entry.isFile);
        const hasDir = entries.some(entry => entry.name === 'subdir' && entry.isDir);
        expect(hasFile).toBe(true);
        expect(hasDir).toBe(true);
    });

    test('GetPath returns full path', () => {
        const finder = new FindFile(testDir);
        if (finder.Next()) {
            const fullPath = finder.GetPath();
            expect(fullPath).toContain(testDir);
        }
    });
});