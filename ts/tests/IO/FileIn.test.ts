import { FileIn } from '../../src/IO/FileIn';
import * as fs from 'fs';
import * as path from 'path';

describe('FileIn', () => {
    const testFilePath = path.join(__dirname, 'test_file.txt');
    const testDirPath = path.dirname(testFilePath);

    beforeAll(() => {
        // Create test directory if it doesn't exist
        if (!fs.existsSync(testDirPath)) {
            fs.mkdirSync(testDirPath, { recursive: true });
        }
        
        // Create a test file
        fs.writeFileSync(testFilePath, 'Hello, World!\nThis is a test file.\nWith multiple lines.', 'utf-8');
    });

    afterAll(() => {
        // Clean up test file
        if (fs.existsSync(testFilePath)) {
            fs.unlinkSync(testFilePath);
        }
    });

    test('Open successfully opens an existing file', () => {
        const fileIn = new FileIn();
        const result = fileIn.Open(testFilePath);
        expect(result).toBe(true);
        expect(fileIn.IsOpen()).toBe(true);
        fileIn.Close();
    });

    test('Open returns false for non-existent file', () => {
        const fileIn = new FileIn();
        const result = fileIn.Open('nonexistent.txt');
        expect(result).toBe(false);
        expect(fileIn.IsOpen()).toBe(false);
        expect(fileIn.IsError()).toBe(true);
    });

    test('IsOpen returns correct state', () => {
        const fileIn = new FileIn();
        expect(fileIn.IsOpen()).toBe(false);
        
        fileIn.Open(testFilePath);
        expect(fileIn.IsOpen()).toBe(true);
        
        fileIn.Close();
        expect(fileIn.IsOpen()).toBe(false);
    });

    test('Read reads data from file', () => {
        const fileIn = new FileIn(testFilePath);
        const buffer = Buffer.alloc(5);
        const bytesRead = fileIn.Read(buffer, 5);
        expect(bytesRead).toBe(5);
        expect(buffer.toString()).toBe('Hello');
        fileIn.Close();
    });

    test('GetSize returns file size', () => {
        const fileIn = new FileIn(testFilePath);
        const size = fileIn.GetSize();
        expect(size).toBeGreaterThan(0);
        fileIn.Close();
    });

    test('ReadAll reads entire file content', () => {
        const fileIn = new FileIn(testFilePath);
        const content = fileIn.ReadAll();
        expect(content).toBe('Hello, World!\nThis is a test file.\nWith multiple lines.');
        fileIn.Close();
    });

    test('IsEof works correctly', () => {
        const fileIn = new FileIn(testFilePath);
        const content = fileIn.ReadAll();
        expect(fileIn.IsEof()).toBe(true);
        fileIn.Close();
    });

    test('Write is not supported on FileIn', () => {
        const fileIn = new FileIn(testFilePath);
        const buffer = Buffer.from('test');
        const bytesWritten = fileIn.Write(buffer, 4);
        expect(bytesWritten).toBe(0);
        expect(fileIn.IsError()).toBe(true);
        fileIn.Close();
    });

    test('ReadLine reads a single line', () => {
        const fileIn = new FileIn(testFilePath);
        const line = fileIn.ReadLine();
        expect(line).toBe('Hello, World!');
        fileIn.Close();
    });

    test('OpenAsync opens file asynchronously', async () => {
        const fileIn = new FileIn();
        const result = await fileIn.OpenAsync(testFilePath);
        expect(result).toBe(true);
        expect(fileIn.IsOpen()).toBe(true);
        fileIn.Close();
    });

    test('ReadAllAsync reads file content asynchronously', async () => {
        const fileIn = new FileIn();
        await fileIn.OpenAsync(testFilePath);
        const content = await fileIn.ReadAllAsync();
        expect(content).toBe('Hello, World!\nThis is a test file.\nWith multiple lines.');
        fileIn.Close();
    });
});