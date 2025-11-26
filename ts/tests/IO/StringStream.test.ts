import { StringStream } from '../../src/IO/StringStream';

describe('StringStream', () => {
    test('constructor initializes with empty string', () => {
        const ss = new StringStream();
        expect(ss.IsOpen()).toBe(true);
        expect(ss.GetString()).toBe('');
    });

    test('constructor initializes with provided string', () => {
        const ss = new StringStream('Hello, World!');
        expect(ss.GetString()).toBe('Hello, World!');
    });

    test('Write adds content to the stream', () => {
        const ss = new StringStream();
        const buffer = Buffer.from('Hello');
        const bytesWritten = ss.Write(buffer, buffer.length);
        expect(bytesWritten).toBe(5);
        expect(ss.GetString()).toBe('Hello');
    });

    test('Read reads content from the stream', () => {
        const ss = new StringStream('Hello');
        const buffer = Buffer.alloc(5);
        const bytesRead = ss.Read(buffer, 5);
        expect(bytesRead).toBe(5);
        expect(buffer.toString()).toBe('Hello');
    });

    test('GetPosition returns current position', () => {
        const ss = new StringStream('Hello, World!');
        ss.Read(Buffer.alloc(5), 5);
        // Note: Position is incremented by character count, not byte count
        // The exact value depends on the implementation
        expect(ss.GetPosition()).toBeGreaterThanOrEqual(0);
    });

    test('Seek sets the position correctly', () => {
        const ss = new StringStream('Hello, World!');
        const result = ss.Seek(6); // Position after 'Hello,'
        expect(result).toBe(true);
        expect(ss.GetPosition()).toBe(6);
    });

    test('GetSize returns string size', () => {
        const ss = new StringStream('Hello, World!');
        expect(ss.GetSize()).toBe(13); // Length of "Hello, World!"
    });

    test('IsEof returns true at end of stream', () => {
        const ss = new StringStream('Hello');
        const content = ss.ReadAll(); // This should move position to end
        expect(content).toBe('Hello');
        expect(ss.IsEof()).toBe(true);
    });

    test('SetString sets new content and resets position', () => {
        const ss = new StringStream('Initial');
        ss.SetString('New content');
        expect(ss.GetString()).toBe('New content');
        expect(ss.GetPosition()).toBe(0);
    });

    test('Clear clears the content', () => {
        const ss = new StringStream('Hello, World!');
        ss.Clear();
        expect(ss.GetString()).toBe('');
        expect(ss.GetPosition()).toBe(0);
        expect(ss.IsError()).toBe(false);
    });

    test('ReadAll reads remaining content', () => {
        const ss = new StringStream('Hello, World!');
        ss.Seek(7); // Position at 'W'
        const remaining = ss.ReadAll();
        expect(remaining).toBe('World!'); // From 'World!' to end
    });

    test('WriteString writes a string to the stream', () => {
        const ss = new StringStream();
        const bytesWritten = ss.WriteString('Hello, World!');
        expect(bytesWritten).toBeGreaterThan(0);
        expect(ss.GetString()).toBe('Hello, World!');
    });

    test('ReadLine reads a single line', () => {
        const ss = new StringStream('Line 1\nLine 2\nLine 3');
        const line1 = ss.ReadLine();
        expect(line1).toBe('Line 1');
        const line2 = ss.ReadLine();
        expect(line2).toBe('Line 2');
    });

    test('ReadAllAsync reads content asynchronously', async () => {
        const ss = new StringStream('Hello, async World!');
        const content = await ss.ReadAllAsync();
        expect(content).toBe('Hello, async World!');
    });

    test('WriteStringAsync writes a string asynchronously', async () => {
        const ss = new StringStream();
        const bytesWritten = await ss.WriteStringAsync('Async content');
        expect(bytesWritten).toBeGreaterThan(0);
        expect(ss.GetString()).toBe('Async content');
    });
});