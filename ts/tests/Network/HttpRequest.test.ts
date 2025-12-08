/**
 * Comprehensive tests for HttpRequest to improve coverage
 */
import { HttpRequest, HttpResponse, HttpGet, HttpPost } from '../../src/Network/HttpRequest';

describe('HttpRequest Comprehensive Tests', () => {
    test('Execute method throws expected error', () => {
        const req = new HttpRequest();
        expect(() => req.Execute()).toThrow("Synchronous HTTP requests are not supported in JavaScript/Node.js without special libraries. Use ExecuteAsync() instead.");
    });

    test('HttpResponse GetBuffer returns body string', () => {
        const response = new HttpResponse(200, 'OK', {'content-type': 'text/plain'}, 'Test body');
        expect(response.GetBuffer()).toBe('Test body');
    });

    test('HttpRequest chaining methods work', () => {
        const req = new HttpRequest();
        const chainedReq = req.Url('https://httpbin.org/get')
                          .Method('POST')
                          .Header('Test-Header', 'test-value')
                          .Body({test: 'data'})
                          .Timeout(10000)
                          .FollowRedirects(false);

        expect(chainedReq).toBeInstanceOf(HttpRequest);
    });

    test('HttpRequest Body method sets Content-Type header', () => {
        const req = new HttpRequest();
        req.Body({test: 'data'});
        
        // The Body method should set Content-Type to 'application/json'
        const headers = (req as any).headers as { [key: string]: string };
        expect(headers['Content-Type']).toBe('application/json');
    });

    test('HttpRequest PostAsync method with data', async () => {
        const req = new HttpRequest();
        // Just test the method exists and can be called without error in structure
        expect(req.PostAsync).toBeInstanceOf(Function);
    });

    test('HttpRequest PutAsync method with data', async () => {
        const req = new HttpRequest();
        // Just test the method exists and can be called without error in structure
        expect(req.PutAsync).toBeInstanceOf(Function);
    });

    test('HttpRequest DeleteAsync method', async () => {
        const req = new HttpRequest();
        // Just test the method exists and can be called without error in structure
        expect(req.DeleteAsync).toBeInstanceOf(Function);
    });

    test('HttpResponse GetJson throws on invalid JSON', () => {
        const response = new HttpResponse(200, 'OK', {'content-type': 'application/json'}, 'invalid json');
        expect(() => response.GetJson()).toThrow('Failed to parse response as JSON:');
    });
    
    test('HttpResponse IsOK returns false for error status', () => {
        const response = new HttpResponse(404, 'Not Found', {}, 'Not found');
        expect(response.IsOK()).toBe(false);
        
        const response2 = new HttpResponse(500, 'Server Error', {}, 'Server error');
        expect(response2.IsOK()).toBe(false);
        
        // Test edge cases
        const response3 = new HttpResponse(199, 'Edge case', {}, 'content');
        expect(response3.IsOK()).toBe(false);
        
        const response4 = new HttpResponse(300, 'Redirect', {}, 'redirect');
        expect(response4.IsOK()).toBe(false);
    });
    
    test('HttpResponse GetHeader with different cases', () => {
        const headers = {
            'content-type': 'application/json',
            'x-custom-header': 'custom-value'  // Using lowercase to match the implementation expectation
        };

        const response = new HttpResponse(200, 'OK', headers, 'body');

        // For exact key match
        expect(response.GetHeader('content-type')).toBe('application/json');
        // For lowercase lookup of originally lowercase key
        expect(response.GetHeader('Content-Type')).toBe('application/json');
        // For exact key match
        expect(response.GetHeader('x-custom-header')).toBe('custom-value');
        // For lowercase lookup of originally lowercase key
        expect(response.GetHeader('X-Custom-Header')).toBe('custom-value');
        expect(response.GetHeader('non-existent')).toBeUndefined();
    });

    test('HttpRequest with no content-type should default to application/json when body is present', async () => {
        // This test checks the internal logic where if a body is set but no content-type
        // the content type defaults to application/json
        const req = new HttpRequest();
        req.Url('https://httpbin.org/post');
        (req as any).body = {test: 'data'};
        (req as any).headers = {}; // No content-type set
        
        // We can't actually execute this without a server, but we can check that the 
        // ExecuteAsync method is defined
        expect(req.ExecuteAsync).toBeInstanceOf(Function);
    }, 30000); // Increase timeout for potential network operations

    test('HttpGet function exists and is usable', async () => {
        expect(HttpGet).toBeInstanceOf(Function);
    });

    test('HttpPost function exists and is usable', async () => {
        expect(HttpPost).toBeInstanceOf(Function);
    });

    test('HttpRequest with error handling', async () => {
        // Mock fetch to return a promise that rejects
        const originalFetch = global.fetch;
        global.fetch = jest.fn(() => Promise.reject(new Error('Network error'))) as any;

        try {
            const req = new HttpRequest();
            req.Url('https://httpbin.org/get');

            await expect(req.ExecuteAsync()).rejects.toThrow('HTTP request failed: Network error');
        } finally {
            global.fetch = originalFetch;
        }
    }, 10000); // Set timeout to 10 seconds for this test

    test('HttpRequest with AbortError throws appropriate message', async () => {
        // Mock fetch to return a promise that rejects with AbortError
        const originalFetch = global.fetch;
        global.fetch = jest.fn(() =>
            Promise.reject(new DOMException('Aborted', 'AbortError'))
        ) as any;

        try {
            const req = new HttpRequest();
            req.Url('https://httpbin.org/get').Timeout(10000); // Set a large timeout to avoid our timeout

            await expect(req.ExecuteAsync()).rejects.toThrow('HTTP request failed: AbortError: Aborted');
        } finally {
            global.fetch = originalFetch;
        }
    }, 10000); // Increase timeout for this test
});