/**
 * Network module tests
 */

import { HttpRequest, HttpResponse } from '../src/Network/HttpRequest';
import { TcpSocket, TcpServer } from '../src/Network/TcpSocket';
import { WsSocket, WsServer } from '../src/Network/WebSocket';
import { Url, UrlUtil } from '../src/Network/URL';
import { JsonSerializer, JsonValidator, JsonPathQuery } from '../src/Network/JsonSerializer';
import { XmlDocument, XmlParser, XmlNode } from '../src/Network/XmlParser';
import { Base64, Base64Util } from '../src/Network/Base64';
import { Compression, CompressionUtil } from '../src/Network/Compression';

// Test HttpRequest
describe('HttpRequest', () => {
    test('can create HttpRequest instance', () => {
        const req = new HttpRequest();
        expect(req).toBeInstanceOf(HttpRequest);
    });

    test('can set URL and method', () => {
        const req = new HttpRequest();
        req.Url('https://httpbin.org/get').Method('GET');
        // Note: Actual execution would require a real server, so we're testing setup
        expect(req).toBeInstanceOf(HttpRequest);
    });
});

// Test URL functionality
describe('URL', () => {
    test('can parse a URL', () => {
        const url = new Url('https://example.com:8080/path?query=value#fragment');
        expect(url.GetProtocol()).toBe('https');
        expect(url.GetHostname()).toBe('example.com');
        expect(url.GetPort()).toBe(8080);
        expect(url.GetPath()).toBe('/path');
        expect(url.GetQuery()).toBe('query=value');
        expect(url.GetFragment()).toBe('fragment');
    });

    test('URL utility functions work', () => {
        expect(UrlUtil.Encode('hello world')).toBe('hello%20world');
        expect(UrlUtil.Decode('hello%20world')).toBe('hello world');
        expect(UrlUtil.BuildQuery({ key: 'value', foo: 'bar' })).toBe('key=value&foo=bar');
    });
});

// Test Base64 functionality
describe('Base64', () => {
    test('can encode and decode strings', () => {
        const original = 'Hello, World!';
        const encoded = Base64.EncodeString(original);
        const decoded = Base64.DecodeString(encoded);
        expect(decoded).toBe(original);
    });

    test('can encode and decode buffers', () => {
        const original = Buffer.from('Hello, World!');
        const encoded = Base64.EncodeBuffer(original);
        const decoded = Base64.DecodeBuffer(encoded);
        expect(decoded.equals(original)).toBe(true);
    });
});

// Test JSON functionality
describe('JsonSerializer', () => {
    test('can serialize and deserialize objects', () => {
        const obj = { name: 'Test', value: 123 };
        const json = JsonSerializer.Serialize(obj);
        const parsed = JsonSerializer.Deserialize(json);
        expect(parsed).toEqual(obj);
    });

    test('JSON path query works', () => {
        const obj = { user: { name: 'John', age: 30 } };
        const query = new JsonPathQuery(obj);
        expect(query.Get('user.name')).toBe('John');
        expect(query.Get('user.age')).toBe(30);
    });
});

// Test Compression functionality
describe('Compression', () => {
    test('gzip compression works', async () => {
        const original = 'This is a test string for compression.';
        const compressed = await Compression.Gzip(original);
        const decompressed = await Compression.GzipDecompress(compressed);
        expect(decompressed.toString()).toBe(original);
    }, 10000); // Increase timeout for compression tests

    test('deflate compression works', async () => {
        const original = 'This is a test string for deflate compression.';
        const compressed = await Compression.Deflate(original);
        const decompressed = await Compression.DeflateDecompress(compressed);
        expect(decompressed.toString()).toBe(original);
    }, 10000);
});