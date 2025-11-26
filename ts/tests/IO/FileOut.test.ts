import { FileOut } from '../../src/IO/FileOut';
import * as fs from 'fs';
import * as path from 'path';

describe('FileOut', () => {
    const testDir = path.join(__dirname, 'test_output');
    const testFilePath = path.join(testDir, 'test_output.txt');
    const testAppendPath = path.join(testDir, 'test_append.txt');

    beforeAll(() => {
        // Create test directory
        if (!fs.existsSync(testDir)) {
            fs.mkdirSync(testDir, { recursive: true });
        }
    });

    afterAll(() => {
        // Clean up test files
        [testFilePath, testAppendPath].forEach(filePath => {
            if (fs.existsSync(filePath)) {
                fs.unlinkSync(filePath);
            }
        });
        
        if (fs.existsSync(testDir)) {
            fs.rmdirSync(testDir, { recursive: true });
        }
    });

    test('Open creates a new file for writing', () => {
        const fileOut = new FileOut();
        const result = fileOut.Open(testFilePath);
        expect(result).toBe(true);
        expect(fileOut.IsOpen()).toBe(true);
        
        const bytesWritten = fileOut.WriteString('Hello, World!');
        expect(bytesWritten).toBeGreaterThan(0);
        
        fileOut.Close();
        
        // Verify the file was created with content
        const content = fs.readFileSync(testFilePath, 'utf-8');
        expect(content).toBe('Hello, World!');
    });

    test('Open with append mode appends to existing file', () => {
        // First, create a file with some content
        fs.writeFileSync(testAppendPath, 'Initial content', 'utf-8');
        
        const fileOut = new FileOut();
        const result = fileOut.Open(testAppendPath, true); // append mode
        expect(result).toBe(true);
        
        const bytesWritten = fileOut.WriteString('\nAppended content');
        expect(bytesWritten).toBeGreaterThan(0);
        
        fileOut.Close();
        
        // Verify content was appended
        const content = fs.readFileSync(testAppendPath, 'utf-8');
        expect(content).toBe('Initial content\nAppended content');
    });

    test('Write writes data to file', () => {
        const fileOut = new FileOut(testFilePath);
        const buffer = Buffer.from('Test data');
        const bytesWritten = fileOut.Write(buffer, buffer.length);
        expect(bytesWritten).toBe(buffer.length);
        fileOut.Close();
        
        // Verify content
        const content = fs.readFileSync(testFilePath, 'utf-8');
        expect(content).toBe('Test data');
    });

    test('Flush flushes buffered data', () => {
        const fileOut = new FileOut(testFilePath);
        fileOut.WriteString('Buffered data');
        fileOut.Flush(); // This should flush the data
        fileOut.Close();
        
        // Verify content
        const content = fs.readFileSync(testFilePath, 'utf-8');
        expect(content).toBe('Buffered data');
    });

    test('GetSize returns file size', () => {
        const fileOut = new FileOut(testFilePath);
        fileOut.WriteString('Test data');
        const size = fileOut.GetSize();
        expect(size).toBeGreaterThan(0);
        fileOut.Close();
    });

    test('Read is not supported on FileOut', () => {
        const fileOut = new FileOut(testFilePath);
        const buffer = Buffer.alloc(10);
        const bytesRead = fileOut.Read(buffer, 10);
        expect(bytesRead).toBe(0);
        expect(fileOut.IsError()).toBe(true);
        fileOut.Close();
    });

    test('OpenAsync creates a new file for writing asynchronously', async () => {
        const fileOut = new FileOut();
        const result = await fileOut.OpenAsync(testFilePath);
        expect(result).toBe(true);
        expect(fileOut.IsOpen()).toBe(true);
        
        const bytesWritten = await fileOut.WriteAsync(Buffer.from('Async content'), 13);
        expect(bytesWritten).toBe(13);
        
        await new Promise(resolve => setTimeout(resolve, 10)); // Allow time for write
        fileOut.Close();
        
        // Verify the file was created with content
        const content = fs.readFileSync(testFilePath, 'utf-8');
        expect(content).toBe('Async content');
    });

    test('WriteAsync writes data asynchronously', async () => {
        const fileOut = new FileOut(testFilePath);
        
        const buffer = Buffer.from('Async test data');
        const bytesWritten = await fileOut.WriteAsync(buffer, buffer.length);
        expect(bytesWritten).toBe(buffer.length);
        
        fileOut.Close();
        
        // Verify content
        const content = fs.readFileSync(testFilePath, 'utf-8');
        expect(content).toBe('Async test data');
    });

    test('FlushAsync flushes data asynchronously', async () => {
        const fileOut = new FileOut(testFilePath);
        fileOut.WriteString('Async buffered data');
        
        await fileOut.FlushAsync();
        fileOut.Close();
        
        // Verify content
        const content = fs.readFileSync(testFilePath, 'utf-8');
        expect(content).toBe('Async buffered data');
    });
});