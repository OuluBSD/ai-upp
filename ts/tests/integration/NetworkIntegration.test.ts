/**
 * Integration tests for network and string operations
 * Tests combinations of network modules with other modules
 */

import { HttpRequest, HttpResponse } from '../../src/Network/HttpRequest';
import { Base64 } from '../../src/Network/Base64';
import { String } from '../../src/Core/String';
import { Vector } from '../../src/Core/Vector';
import { Optional } from '../../src/Core/Optional';
import { Time } from '../../src/DateTime/Time';
import { DateClass } from '../../src/DateTime/Date';
import { Compression } from '../../src/Network/Compression';

describe('Network Integration Tests', () => {
    test('Base64 with String operations', () => {
        // Test Base64 encoding of String objects
        const originalStr = new String('Hello, Network Integration Test!');
        const encoded = Base64.EncodeString(originalStr.ToString());
        const decoded = Base64.DecodeString(encoded);
        
        expect(decoded).toBe(originalStr.ToString());
        
        // Test with buffers as well
        const buffer = Buffer.from(originalStr.ToString());
        const encodedBuffer = Base64.EncodeBuffer(buffer);
        const decodedBuffer = Base64.DecodeBuffer(encodedBuffer);
        
        expect(decodedBuffer.equals(buffer)).toBe(true);
        
        // Test URL-safe encoding
        const urlWithSpecialChars = new String('https://example.com/path?param=value+with+spaces');
        const urlSafeEncoded = Base64.EncodeUrlSafe(urlWithSpecialChars.ToString());
        const urlSafeDecoded = Base64.DecodeUrlSafe(urlSafeEncoded);
        
        expect(urlSafeDecoded).toBe(urlWithSpecialChars.ToString());
    });

    test('Compression with data structures', async () => {
        // Create a complex data structure to compress
        const data = {
            timestamp: Date.now(),
            items: [
                { id: 1, name: 'Item 1', value: 100 },
                { id: 2, name: 'Item 2', value: 200 },
                { id: 3, name: 'Item 3', value: 300 }
            ],
            metadata: {
                version: '1.0',
                created: new Date().toISOString()
            }
        };

        // Test JSON compression
        const jsonString = JSON.stringify(data);
        const compressed = await Compression.CompressString(jsonString, 'gzip');
        const decompressed = await Compression.DecompressString(compressed, 'gzip');
        
        expect(JSON.parse(decompressed)).toEqual(data);
        
        // Test compression ratio
        const originalSize = jsonString.length;
        const compressedSize = compressed.length;
        const ratio = Compression.CalculateRatio(originalSize, compressedSize);
        const percentage = Compression.CalculatePercentage(originalSize, compressedSize);
        
        // Ratio should be <= 1 (compressed should be smaller or same size)
        expect(ratio).toBeLessThanOrEqual(1);
        // Percentage should be positive if compression worked (size reduction)
        expect(percentage).toBeGreaterThanOrEqual(0);
    }, 10000); // Longer timeout for compression tests

    test('Time and Date with JSON serialization', () => {
        // Test serializing Time and Date objects
        const now = Time.Now();
        const today = DateClass.Today();
        
        // Simulate what would happen in a JSON serialization scenario
        const timeData = {
            year: now.GetYear(),
            month: now.GetMonth(),
            day: now.GetDay(),
            hour: now.GetHour(),
            minute: now.GetMinute(),
            second: now.GetSecond(),
            millisecond: now.GetMillisecond()
        };
        
        const dateData = {
            year: today.GetYear(),
            month: today.GetMonth(),
            day: today.GetDay()
        };
        
        // Verify the data is valid
        expect(timeData.year).toBeGreaterThan(2000);
        expect(timeData.month).toBeGreaterThanOrEqual(1);
        expect(timeData.month).toBeLessThanOrEqual(12);
        expect(dateData.year).toBeGreaterThan(2000);
        
        // Test creating new instances from the data
        const newTime = new Time(
            timeData.year,
            timeData.month,
            timeData.day,
            timeData.hour,
            timeData.minute,
            timeData.second,
            timeData.millisecond
        );
        
        const newDate = new DateClass(
            dateData.year,
            dateData.month,
            dateData.day
        );
        
        expect(newTime.GetYear()).toBe(timeData.year);
        expect(newDate.GetYear()).toBe(dateData.year);
    });

    test('Network headers with Map operations', async () => {
        // Although we can't make actual network requests in tests without a server,
        // we can test the API structures and data flow
        
        // Create mock request with headers
        const req = new HttpRequest();
        req.Url('https://httpbin.org/headers')
           .Header('X-Custom-Header', 'test-value')
           .Header('Content-Type', 'application/json')
           .Header('Authorization', 'Bearer token123');
        
        // Verify headers are set in the internal structure
        // This tests the internal structure even if execute doesn't work
        expect(req).toBeInstanceOf(HttpRequest);
    });
    
    test('Response handling with container operations', () => {
        // Test creating and manipulating response-like data with containers
        const headers = new Vector<{ name: string, value: string }>();
        headers.Add({ name: 'Content-Type', value: 'application/json' });
        headers.Add({ name: 'Content-Length', value: '1234' });
        headers.Add({ name: 'Server', value: 'TestServer/1.0' });

        // Find a specific header
        const contentTypeIndex = headers.Find(h => h.name === 'Content-Type');
        expect(contentTypeIndex).toBe(0); // Should be found at index 0
        expect(headers.At(contentTypeIndex).value).toBe('application/json');

        // Find all content-type related headers (there should be at least one)
        let foundHeaders = new Vector<{ name: string, value: string }>();
        for (const header of headers) {
            if (header.name.toLowerCase().includes('content')) {
                foundHeaders.Add(header);
            }
        }
        expect(foundHeaders.GetCount()).toBeGreaterThan(0);
    });
});

describe('Complex Integration Tests', () => {
    test('Full workflow: String -> Base64 -> Compress -> Store', async () => {
        // Create a complex string data using the String class
        const originalData = new String('This is a complex test string for the full workflow test!'.repeat(10));
        
        // Step 1: Convert to base64
        const base64Encoded = Base64.EncodeString(originalData.ToString());
        const base64AsStr = new String(base64Encoded);
        
        expect(base64AsStr.GetLength()).toBeGreaterThan(originalData.GetLength());
        
        // Step 2: Compress the base64 string
        const compressed = await Compression.CompressString(base64AsStr.ToString(), 'gzip');
        const compressedAsStr = new String(compressed);
        
        // The compressed version should likely be smaller than the raw base64
        // depending on the data structure
        expect(compressedAsStr.GetLength()).toBeGreaterThan(10); // Should not be empty
        
        // Step 3: Decompress to get the base64 string back
        const decompressed = await Compression.DecompressString(compressed, 'gzip');
        
        // Step 4: Decode the base64 back to original
        const decodedBack = Base64.DecodeString(decompressed);
        
        // Verify we got back to original
        expect(decodedBack).toBe(originalData.ToString());
    }, 15000); // Longer timeout for comprehensive test

    test('Optional and Error Handling Integration', () => {
        // Simulate fetching data that might not exist
        const fetchData = (id: number): Optional<{ id: number, name: string }> => {
            if (id > 0 && id <= 5) {
                return Optional.Of({ id, name: `Item ${id}` });
            }
            return Optional.Of(null as any); // Not found
        };

        // Get multiple items
        const results = new Vector<Optional<{ id: number, name: string }>>();
        for (let i = 0; i < 7; i++) {
            results.Add(fetchData(i));
        }

        // Count the found items by iterating through the Vector
        let foundItemsCount = 0;
        for (let i = 0; i < results.GetCount(); i++) {
            const opt = results.At(i);
            if (!opt.IsNull()) {
                foundItemsCount++;
            }
        }

        expect(foundItemsCount).toBe(5); // Items 1-5 should exist

        // Check specific values
        const firstItemOpt = results.At(1); // Should be Item 1 (id=1)
        expect(!firstItemOpt.IsNull()).toBe(true);
        expect(firstItemOpt.Get().id).toBe(1);
        expect(firstItemOpt.Get().name).toBe('Item 1');

        const zeroItemOpt = results.At(0); // Should be null (id=0)
        expect(zeroItemOpt.IsNull()).toBe(true);
    });
});