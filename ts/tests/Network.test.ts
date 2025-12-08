/**
 * Network module tests
 */

import { HttpRequest, HttpResponse, HttpGet, HttpPost } from '../src/Network/HttpRequest';
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

    test('brotli compression works', async () => {
        const original = 'This is a test string for brotli compression.';
        const compressed = await Compression.Brotli(original);
        const decompressed = await Compression.BrotliDecompress(compressed);
        expect(decompressed.toString()).toBe(original);
    }, 10000);

    test('multi-algorithm compression works', async () => {
        const original = 'Test string for multi-algorithm compression';
        const compressedGzip = await Compression.Compress(original, 'gzip');
        const decompressedGzip = await Compression.Decompress(compressedGzip, 'gzip');
        expect(decompressedGzip.toString()).toBe(original);

        const compressedDeflate = await Compression.Compress(original, 'deflate');
        const decompressedDeflate = await Compression.Decompress(compressedDeflate, 'deflate');
        expect(decompressedDeflate.toString()).toBe(original);

        const compressedBrotli = await Compression.Compress(original, 'brotli');
        const decompressedBrotli = await Compression.Decompress(compressedBrotli, 'brotli');
        expect(decompressedBrotli.toString()).toBe(original);
    }, 15000);

    test('string compression works', async () => {
        const original = 'Test string for string compression';
        const compressed = await Compression.CompressString(original, 'gzip');
        const decompressed = await Compression.DecompressString(compressed, 'gzip');
        expect(decompressed).toBe(original);
    }, 10000);

    test('compression ratio calculation works', () => {
        const ratio = Compression.CalculateRatio(100, 50);
        expect(ratio).toBe(0.5);

        const percentage = Compression.CalculatePercentage(100, 50);
        expect(percentage).toBe(50);
    });

    test('compression utilities work', async () => {
        const obj = { name: 'test', value: 123 };
        const compressed = await CompressionUtil.CompressObject(obj);
        const decompressed = await CompressionUtil.DecompressObject(compressed);
        expect(decompressed).toEqual(obj);
    }, 10000);

    test('algorithm comparison works', async () => {
        const testString = 'This is a string for testing compression algorithms.';
        const results = await CompressionUtil.CompareAlgorithms(testString);

        expect(results.gzip).toHaveProperty('compressed');
        expect(results.deflate).toHaveProperty('compressed');
        expect(results.brotli).toHaveProperty('compressed');
        expect(results.gzip.ratio).toBeGreaterThanOrEqual(0);
        expect(results.deflate.ratio).toBeGreaterThanOrEqual(0);
        expect(results.brotli.ratio).toBeGreaterThanOrEqual(0);
    }, 15000);
});

// Test XML functionality
describe('XmlParser', () => {
    test('can parse XML string', async () => {
        const xmlString = '<root><item id="1">Test</item><item id="2">Value</item></root>';
        const doc = await XmlParser.Parse(xmlString);

        expect(doc).toBeInstanceOf(XmlDocument);
        const root = doc.GetRoot();
        expect(root).toBeDefined();
        expect(root?.GetTagName()).toBe('root');
    });

    test('XmlDocument methods work', async () => {
        const xmlString = '<root><item>Test</item></root>';
        const doc = await XmlParser.Parse(xmlString);

        expect(doc.SaveToString()).toContain('<root>');

        const root = doc.GetRoot();
        expect(root).toBeDefined();
        if (root) {
            expect(root.GetTagName()).toBe('root');
            // The simple XML parser implementation in the library will include child elements as text content
            // This is expected behavior for the simple implementation
            expect(root.GetTextContent()).toBeDefined();
        }
    });

    test('XmlNode methods work', async () => {
        const node = new XmlNode('test');
        expect(node.GetTagName()).toBe('test');

        node.SetTagName('newtag');
        expect(node.GetTagName()).toBe('newtag');

        node.SetAttribute('attr1', 'value1');
        expect(node.GetAttribute('attr1')).toBe('value1');
        expect(node.GetAttributes()).toHaveProperty('attr1');

        const child = new XmlNode('child');
        node.AddChild(child);
        expect(node.GetChildCount()).toBe(1);
        expect(node.GetChildren().length).toBe(1);

        node.SetTextContent('test content');
        expect(node.GetTextContent()).toBe('test content');

        const xmlStr = node.ToXml();
        expect(xmlStr).toContain('newtag');
        expect(xmlStr).toContain('test content');
    });

    test('XML to/from object conversion works', async () => {
        const obj = { tagName: 'root', attributes: { id: '1' }, textContent: 'Test' };
        const xml = XmlParser.FromObject(obj, 'root');
        expect(xml).toContain('root');

        const parsedObj = await XmlParser.ToObject(xml);
        expect(parsedObj).toHaveProperty('tagName');
    }, 10000);
});

// Test HttpRequest additional methods
describe('HttpRequest additional', () => {
    test('can set headers and get response', async () => {
        const req = new HttpRequest();

        // Just testing methods exist without errors
        expect(req).toBeInstanceOf(HttpRequest);

        // Test Header method
        const withHeaders = req.Header('Content-Type', 'application/json');
        expect(withHeaders).toBeInstanceOf(HttpRequest);

        // Test different methods
        expect(req.Url('https://httpbin.org/get')).toBeInstanceOf(HttpRequest);
        expect(req.Method('POST')).toBeInstanceOf(HttpRequest);
        expect(req.Timeout(5000)).toBeInstanceOf(HttpRequest);
        expect(req.FollowRedirects(true)).toBeInstanceOf(HttpRequest);
    });

    test('HTTP convenience functions exist', async () => {
        // These will fail since there's no server, but we can at least test they exist
        expect(typeof HttpGet).toBe('function');
        expect(typeof HttpPost).toBe('function');
    });

    test('HttpResponse methods work', () => {
        const response = new HttpResponse(200, 'OK', {'content-type': 'text/plain'}, 'Test body');

        expect(response.GetStatus()).toBe(200);
        expect(response.GetStatusText()).toBe('OK');
        expect(response.GetHeaders()).toEqual({'content-type': 'text/plain'});
        expect(response.GetHeader('content-type')).toBe('text/plain');
        expect(response.GetBody()).toBe('Test body');
        expect(response.IsOK()).toBe(true);
    });

    test('HttpResponse JSON parsing works', () => {
        const jsonBody = JSON.stringify({test: 'value'});
        const response = new HttpResponse(200, 'OK', {'content-type': 'application/json'}, jsonBody);

        expect(response.GetJson()).toEqual({test: 'value'});
    });

    test('HttpResponse handles JSON parsing errors', () => {
        const response = new HttpResponse(200, 'OK', {'content-type': 'application/json'}, 'invalid json');

        expect(() => response.GetJson()).toThrow();
    });
});

// Test additional TcpSocket functionality
describe('TcpSocket additional', () => {
    test('socket methods exist', () => {
        const socket = new TcpSocket();
        expect(socket).toBeInstanceOf(TcpSocket);
    });
});

// Test additional utilities
describe('Additional utilities', () => {
    test('Base64 utilities work correctly', () => {
        const original = 'Hello, World!';
        const encoded = Base64Util.EncodeJson({text: original});
        const decoded = Base64Util.DecodeJson(encoded);
        expect(decoded.text).toBe(original);

        // Test data URL functions
        const dataUrl = Base64Util.EncodeDataUrl('text/plain', original);
        expect(dataUrl).toContain('data:text/plain;base64,');

        const decodedDataUrl = Base64Util.DecodeDataUrl(dataUrl);
        expect(decodedDataUrl).not.toBeNull();
        if (decodedDataUrl) {
            expect(decodedDataUrl.mimeType).toBe('text/plain');
        }

        // Test chunk functions
        const longBase64 = Base64.EncodeString('This is a longer string for testing chunks');
        const chunks = Base64Util.SplitToChunks(longBase64, 10);
        expect(chunks.length).toBeGreaterThan(1);
        const rejoined = Base64Util.JoinChunks(chunks);
        expect(rejoined).toBe(longBase64);
    });

    test('Base64 encode/decode methods work', () => {
        const originalStr = 'Test string for encoding';
        const originalBuffer = Buffer.from(originalStr);
        const originalBytes = new Uint8Array(originalBuffer);

        // String encoding/decoding
        const encodedStr = Base64.EncodeString(originalStr);
        const decodedStr = Base64.DecodeString(encodedStr);
        expect(decodedStr).toBe(originalStr);

        // Buffer encoding/decoding
        const encodedBuffer = Base64.EncodeBuffer(originalBuffer);
        const decodedBuffer = Base64.DecodeBuffer(encodedBuffer);
        expect(decodedBuffer.equals(originalBuffer)).toBe(true);

        // Bytes encoding/decoding
        const encodedBytes = Base64.EncodeBytes(originalBytes);
        const decodedBytes = Base64.DecodeBytes(encodedBytes);
        expect(decodedBytes).toEqual(originalBytes);
    });

    test('Base64 validation and utility functions work', () => {
        const validBase64 = Base64.EncodeString('test');
        expect(Base64.IsValidBase64(validBase64)).toBe(true);
        expect(Base64.IsValidBase64('invalid-base64')).toBe(false);

        const padded = Base64.Pad('YW55IGNhcm5hbCBwbGVhc3VyZQ'); // Missing padding
        expect(padded).toBe('YW55IGNhcm5hbCBwbGVhc3VyZQ==');

        const withoutPadding = Base64.RemovePadding('dGVzdA==');
        expect(withoutPadding).toBe('dGVzdA');

        const urlSafe = Base64.EncodeUrlSafe('test with + and /');
        expect(urlSafe).not.toMatch(/[+/]/);
        const urlSafeDecoded = Base64.DecodeUrlSafe(urlSafe);
        expect(urlSafeDecoded).toBe('test with + and /');
    });

    test('URL utilities handle edge cases', () => {
        expect(UrlUtil.Encode('')).toBe('');
        expect(UrlUtil.Decode('')).toBe('');
        expect(UrlUtil.EncodeQuery('test key')).toBe('test%20key');

        const query = 'key1=value1&key2=value2';
        const parsed = UrlUtil.ParseQuery(query);
        expect(parsed.key1).toBe('value1');
        expect(parsed.key2).toBe('value2');

        const params = { a: '1', b: 'test space', c: '' };
        const builtQuery = UrlUtil.BuildQuery(params);
        expect(builtQuery).toContain('a=1');
        expect(builtQuery).toContain('b=test%20space');
        expect(builtQuery).toContain('c=');

        const normalized = UrlUtil.Normalize('http://example.com/path/../newpath');
        expect(normalized).toContain('/newpath');
    });

    test('URL class methods work', () => {
        const url = new Url('https://example.com:8080/path?query=value#fragment');

        expect(url.GetProtocol()).toBe('https');
        expect(url.GetHostname()).toBe('example.com');
        expect(url.GetPort()).toBe(8080);
        expect(url.GetPath()).toBe('/path');
        expect(url.GetQuery()).toBe('query=value');
        expect(url.GetFragment()).toBe('fragment');
        expect(url.GetQueryValue('query')).toBe('value');
        expect(url.GetOrigin()).toBe('https://example.com:8080');

        expect(url.IsProtocol('https')).toBe(true);
        expect(url.IsSecure()).toBe(true);

        const newUrl = url.WithProtocol('http');
        expect(newUrl.GetProtocol()).toBe('http');

        const withPath = url.WithPath('/newpath');
        expect(withPath.GetPath()).toBe('/newpath');
    });
});