/**
 * HttpRequest - HTTP client class similar to U++ Http request functionality
 * Provides both synchronous and asynchronous HTTP operations
 */
export class HttpRequest {
    private url: string = '';
    private method: string = 'GET';
    private headers: { [key: string]: string } = {};
    private body: any = null;
    private timeout: number = 30000; // 30 seconds default timeout
    private followRedirects: boolean = true;
    private maxRedirects: number = 10;

    /**
     * Create a new HTTP request
     */
    constructor() {
        this.headers = {
            'User-Agent': 'uppts-http/0.1.0',
            'Accept': '*/*'
        };
    }

    /**
     * Set the URL for the request
     * @param url The URL to request
     * @returns This request object for chaining
     */
    Url(url: string): HttpRequest {
        this.url = url;
        return this;
    }

    /**
     * Set the HTTP method
     * @param method The HTTP method (GET, POST, PUT, DELETE, etc.)
     * @returns This request object for chaining
     */
    Method(method: string): HttpRequest {
        this.method = method.toUpperCase();
        return this;
    }

    /**
     * Add a header to the request
     * @param name Header name
     * @param value Header value
     * @returns This request object for chaining
     */
    Header(name: string, value: string): HttpRequest {
        this.headers[name] = value;
        return this;
    }

    /**
     * Set the request body
     * @param data The data to send in the request body
     * @returns This request object for chaining
     */
    Body(data: any): HttpRequest {
        this.body = data;
        this.headers['Content-Type'] = 'application/json';
        return this;
    }

    /**
     * Set the timeout for the request
     * @param milliseconds Timeout in milliseconds
     * @returns This request object for chaining
     */
    Timeout(milliseconds: number): HttpRequest {
        this.timeout = milliseconds;
        return this;
    }

    /**
     * Set whether redirects should be followed
     * @param follow Whether to follow redirects
     * @returns This request object for chaining
     */
    FollowRedirects(follow: boolean): HttpRequest {
        this.followRedirects = follow;
        return this;
    }

    /**
     * Execute the HTTP request asynchronously
     * @returns Promise resolving to response
     */
    async ExecuteAsync(): Promise<HttpResponse> {
        // Use fetch for HTTP requests
        const controller = new AbortController();
        const timeoutId = setTimeout(() => controller.abort(), this.timeout);

        try {
            const config: RequestInit = {
                method: this.method,
                headers: this.headers,
                body: this.body ? JSON.stringify(this.body) : undefined,
                redirect: this.followRedirects ? 'follow' : 'manual',
                signal: controller.signal
            };

            // If we have a body but no content-type header set, default to text/plain
            if (this.body && !this.headers['Content-Type']) {
                (config.headers as { [key: string]: string })['Content-Type'] = 'application/json';
            }

            const response = await fetch(this.url, config);
            clearTimeout(timeoutId);

            return new HttpResponse(
                response.status,
                response.statusText,
                Object.fromEntries(response.headers.entries()),
                await response.text()
            );
        } catch (error) {
            clearTimeout(timeoutId);
            if (error instanceof Error && error.name === 'AbortError') {
                throw new Error(`Request timeout after ${this.timeout}ms`);
            }
            throw new Error(`HTTP request failed: ${error instanceof Error ? error.message : String(error)}`);
        }
    }

    /**
     * Execute the HTTP request synchronously using Promise blocking
     * @returns Response object
     */
    Execute(): HttpResponse {
        // Since JavaScript/Node.js doesn't have true sync HTTP requests without third-party libraries,
        // we'll make this a wrapper around the async version but return the result directly
        // In a real synchronous implementation, you'd need a different approach
        throw new Error("Synchronous HTTP requests are not supported in JavaScript/Node.js without special libraries. Use ExecuteAsync() instead.");
    }

    /**
     * Perform a GET request
     * @returns Promise resolving to response
     */
    async GetAsync(): Promise<HttpResponse> {
        this.Method('GET');
        return await this.ExecuteAsync();
    }

    /**
     * Perform a POST request with data
     * @param data The data to send in the request body
     * @returns Promise resolving to response
     */
    async PostAsync(data?: any): Promise<HttpResponse> {
        this.Method('POST');
        if (data) {
            this.Body(data);
        }
        return await this.ExecuteAsync();
    }

    /**
     * Perform a PUT request with data
     * @param data The data to send in the request body
     * @returns Promise resolving to response
     */
    async PutAsync(data?: any): Promise<HttpResponse> {
        this.Method('PUT');
        if (data) {
            this.Body(data);
        }
        return await this.ExecuteAsync();
    }

    /**
     * Perform a DELETE request
     * @returns Promise resolving to response
     */
    async DeleteAsync(): Promise<HttpResponse> {
        this.Method('DELETE');
        return await this.ExecuteAsync();
    }
}

/**
 * HttpResponse - Response from an HTTP request
 */
export class HttpResponse {
    private _status: number;
    private _statusText: string;
    private _headers: { [key: string]: string };
    private _body: string;

    constructor(status: number, statusText: string, headers: { [key: string]: string }, body: string) {
        this._status = status;
        this._statusText = statusText;
        this._headers = headers;
        this._body = body;
    }

    /**
     * Get the HTTP status code
     */
    GetStatus(): number {
        return this._status;
    }

    /**
     * Get the HTTP status text
     */
    GetStatusText(): string {
        return this._statusText;
    }

    /**
     * Get the response headers
     */
    GetHeaders(): { [key: string]: string } {
        return this._headers;
    }

    /**
     * Get a specific header value
     * @param name The header name
     * @returns The header value or undefined if not found
     */
    GetHeader(name: string): string | undefined {
        return this._headers[name.toLowerCase()] || this._headers[name];
    }

    /**
     * Get the response body as string
     */
    GetBody(): string {
        return this._body;
    }

    /**
     * Get the response body as JSON
     */
    GetJson(): any {
        try {
            return JSON.parse(this._body);
        } catch (e) {
            throw new Error(`Failed to parse response as JSON: ${(e as Error).message}`);
        }
    }

    /**
     * Check if the request was successful (status code 200-299)
     */
    IsOK(): boolean {
        return this._status >= 200 && this._status < 300;
    }

    /**
     * Get the response body as a buffer (for binary data)
     * Note: In TypeScript/JavaScript, this returns the string body which can be converted to Buffer if needed
     */
    GetBuffer(): string {
        return this._body;
    }
}

/**
 * Convenience function to make GET requests
 * @param url The URL to request
 * @param headers Optional headers
 * @returns Promise resolving to response
 */
export async function HttpGet(url: string, headers?: { [key: string]: string }): Promise<HttpResponse> {
    const req = new HttpRequest().Url(url).Method('GET');
    if (headers) {
        Object.entries(headers).forEach(([name, value]) => {
            req.Header(name, value);
        });
    }
    return await req.ExecuteAsync();
}

/**
 * Convenience function to make POST requests
 * @param url The URL to request
 * @param data Optional data to send in body
 * @param headers Optional headers
 * @returns Promise resolving to response
 */
export async function HttpPost(url: string, data?: any, headers?: { [key: string]: string }): Promise<HttpResponse> {
    const req = new HttpRequest().Url(url).Method('POST');
    if (headers) {
        Object.entries(headers).forEach(([name, value]) => {
            req.Header(name, value);
        });
    }
    if (data) {
        req.Body(data);
    }
    return await req.ExecuteAsync();
}