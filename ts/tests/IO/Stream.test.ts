import { Stream } from '../../src/IO/Stream';

// Create a concrete implementation for testing purposes
class TestStream extends Stream {
    private isOpened: boolean = true;
    private content: string = '';
    private position: number = 0;

    IsOpen(): boolean {
        return this.isOpened;
    }

    Close(): void {
        this.isOpened = false;
    }

    Read(buffer: Buffer, size: number): number {
        if (this.position >= this.content.length) {
            return 0; // End of content
        }

        const available = this.content.length - this.position;
        const toRead = Math.min(size, available);
        const subStr = this.content.substring(this.position, this.position + toRead);
        const bytes = Buffer.from(subStr, 'utf-8');
        
        bytes.copy(buffer, 0, 0, Math.min(bytes.length, buffer.length));
        
        const bytesRead = bytes.length;
        this.position += subStr.length;
        
        return bytesRead;
    }

    Write(buffer: Buffer, size: number): number {
        const dataToWrite = buffer.subarray(0, size).toString('utf-8');
        this.content += dataToWrite;
        return dataToWrite.length;
    }

    setContent(content: string): void {
        this.content = content;
        this.position = 0;
    }
}

describe('Stream', () => {
    let testStream: TestStream;

    beforeEach(() => {
        testStream = new TestStream();
    });

    test('IsError returns false initially', () => {
        expect(testStream.IsError()).toBe(false);
    });

    test('GetError returns empty string initially', () => {
        expect(testStream.GetError()).toBe('');
    });

    test('SetError sets error state and message', () => {
        testStream['SetError']('Test error');
        expect(testStream.IsError()).toBe(true);
        expect(testStream.GetError()).toBe('Test error');
    });

    test('ClearError clears error state', () => {
        testStream['SetError']('Test error');
        expect(testStream.IsError()).toBe(true);
        
        testStream.ClearError();
        expect(testStream.IsError()).toBe(false);
        expect(testStream.GetError()).toBe('');
    });

    test('WriteString writes a string to the stream', () => {
        const bytesWritten = testStream.WriteString('Hello, World!');
        expect(bytesWritten).toBeGreaterThan(0);
    });

    test('ReadLine reads a line from the stream', () => {
        testStream.setContent('Hello\nWorld\nTest');
        const line1 = testStream.ReadLine();
        expect(line1).toBe('Hello');
    });

    test('ReadAll reads entire content of the stream', () => {
        testStream.setContent('Hello, World!');
        const content = testStream.ReadAll();
        expect(content).toBe('Hello, World!');
    });

    test('ReadAsync reads data asynchronously', async () => {
        testStream.setContent('Hello');
        const buffer = Buffer.alloc(5);
        const bytesRead = await testStream.ReadAsync(buffer, 5);
        expect(bytesRead).toBe(5);
        expect(buffer.toString()).toBe('Hello');
    });

    test('WriteAsync writes data asynchronously', async () => {
        const buffer = Buffer.from('Hello');
        const bytesWritten = await testStream.WriteAsync(buffer, 5);
        expect(bytesWritten).toBe(5);
    });

    test('ReadAllAsync reads entire content asynchronously', async () => {
        testStream.setContent('Hello, World!');
        const content = await testStream.ReadAllAsync();
        expect(content).toBe('Hello, World!');
    });

    test('WriteStringAsync writes a string asynchronously', async () => {
        const bytesWritten = await testStream.WriteStringAsync('Hello, World!');
        expect(bytesWritten).toBeGreaterThan(0);
    });
});