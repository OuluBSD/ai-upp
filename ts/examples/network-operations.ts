/**
 * Example 2: HTTP client operations
 * Demonstrates network functionality: HttpRequest, HttpResponse
 */

import { HttpRequest, HttpResponse } from '../src/Network/HttpRequest';
import { Base64 } from '../src/Network/Base64';
import { Compression } from '../src/Network/Compression';

async function httpClientExample() {
    console.log('=== HTTP Client Example ===');
    
    try {
        // Example 1: Simple GET request
        console.log('Making GET request to httpbin.org...');
        const request = new HttpRequest();
        
        // For this example, we'll use a mock approach since we don't want to make real requests in examples
        // In a real application, you would do:
        // const response = await request.Url('https://httpbin.org/get').Method('GET').ExecuteAsync();
        
        console.log('HttpRequest object created successfully');
        console.log('Available methods: Url(), Method(), Header(), Body(), ExecuteAsync()');
        
        // Example 2: Base64 encoding/decoding
        console.log('\n--- Base64 Encoding/Decoding ---');
        const originalText = 'Hello, this is a test string for Base64 encoding!';
        console.log(`Original text: ${originalText}`);
        
        const encoded = Base64.EncodeString(originalText);
        console.log(`Base64 encoded: ${encoded}`);
        
        const decoded = Base64.DecodeString(encoded);
        console.log(`Decoded back: ${decoded}`);
        
        // Example 3: Data compression
        console.log('\n--- Data Compression ---');
        const textToCompress = 'This is a longer text that we will compress and decompress using gzip. '.repeat(10);
        console.log(`Original length: ${textToCompress.length} characters`);
        
        const compressed = await Compression.Gzip(textToCompress);
        console.log(`Compressed length: ${compressed.length} bytes`);
        console.log(`Compression ratio: ${(compressed.length / textToCompress.length * 100).toFixed(2)}% of original`);
        
        const decompressedBuffer = await Compression.GzipDecompress(compressed);
        const decompressed = decompressedBuffer.toString('utf8');
        console.log(`Decompressed matches original: ${decompressed === textToCompress}`);
        
        console.log('HTTP client example completed.\n');
    } catch (error) {
        console.error('Error in HTTP client example:', error);
    }
}

// Example: JSON handling
async function jsonHandlingExample() {
    console.log('=== JSON Handling Example ===');
    
    // Create a complex data structure
    const userData = {
        id: 12345,
        name: 'John Doe',
        email: 'john.doe@example.com',
        preferences: {
            theme: 'dark',
            notifications: true,
            language: 'en'
        },
        items: [
            { id: 1, name: 'Item 1', value: 100 },
            { id: 2, name: 'Item 2', value: 200 },
            { id: 3, name: 'Item 3', value: 300 }
        ]
    };
    
    console.log('Original user data:', JSON.stringify(userData, null, 2));
    
    // Simulate JSON serialization through Base64
    const jsonString = JSON.stringify(userData);
    const encodedJson = Base64.EncodeString(jsonString);
    console.log(`JSON encoded as Base64 (${encodedJson.length} chars): ${encodedJson.substring(0, 60)}...`);
    
    // Decode back
    const decodedJson = Base64.DecodeString(encodedJson);
    const decodedData = JSON.parse(decodedJson);
    console.log(`Decoded data id matches: ${decodedData.id === userData.id}`);
    
    console.log('JSON handling example completed.\n');
}

// Run examples
async function runNetworkExamples() {
    await httpClientExample();
    await jsonHandlingExample();
    console.log('All network examples completed!');
}

runNetworkExamples().catch(console.error);