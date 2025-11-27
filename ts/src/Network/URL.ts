/**
 * URL - URL manipulation class similar to U++ URL functionality
 * Uses Node.js built-in URL class for URL operations
 */
export class Url {
    private url: URL;
    
    /**
     * Create a new URL object
     * @param urlStr The URL string to parse
     */
    constructor(urlStr: string) {
        try {
            // If the URL doesn't have a protocol, add a default one for parsing
            if (!urlStr.includes('://')) {
                urlStr = 'http://' + urlStr;
            }
            this.url = new URL(urlStr);
        } catch (error) {
            throw new Error(`Invalid URL: ${error instanceof Error ? error.message : String(error)}`);
        }
    }

    /**
     * Get the protocol (scheme) of the URL
     */
    GetProtocol(): string {
        return this.url.protocol.replace(':', '');
    }

    /**
     * Get the host (hostname:port) of the URL
     */
    GetHost(): string {
        return this.url.host;
    }

    /**
     * Get the hostname of the URL
     */
    GetHostname(): string {
        return this.url.hostname;
    }

    /**
     * Get the port of the URL
     */
    GetPort(): number {
        return this.url.port ? parseInt(this.url.port, 10) : this.getDefaultPort();
    }

    /**
     * Get the path of the URL
     */
    GetPath(): string {
        return this.url.pathname;
    }

    /**
     * Get the query string of the URL (without the '?')
     */
    GetQuery(): string {
        return this.url.search.substring(1);
    }

    /**
     * Get the fragment/anchor of the URL (without the '#')
     */
    GetFragment(): string {
        return this.url.hash.substring(1);
    }

    /**
     * Get a specific query parameter value
     * @param name The parameter name
     */
    GetQueryValue(name: string): string | null {
        return this.url.searchParams.get(name);
    }

    /**
     * Get all query parameters as key-value pairs
     */
    GetQueryValues(): { [key: string]: string } {
        const result: { [key: string]: string } = {};
        for (const [key, value] of this.url.searchParams.entries()) {
            result[key] = value;
        }
        return result;
    }

    /**
     * Get the full URL string
     */
    ToString(): string {
        return this.url.toString();
    }

    /**
     * Get the origin (protocol + host) of the URL
     */
    GetOrigin(): string {
        return this.url.origin;
    }

    /**
     * Create a new URL by resolving a relative URL against this one
     * @param relativeUrl The relative URL to resolve
     */
    Resolve(relativeUrl: string): Url {
        try {
            const resolved = new URL(relativeUrl, this.url);
            return new Url(resolved.toString());
        } catch (error) {
            throw new Error(`Cannot resolve URL: ${error instanceof Error ? error.message : String(error)}`);
        }
    }

    /**
     * Check if the URL has a specific protocol
     * @param protocol The protocol to check for (without ':')
     */
    IsProtocol(protocol: string): boolean {
        return this.GetProtocol() === protocol;
    }

    /**
     * Check if the URL uses a secure protocol (https, wss, etc.)
     */
    IsSecure(): boolean {
        const protocol = this.GetProtocol();
        return protocol === 'https' || protocol === 'wss' || protocol === 'ftps';
    }

    /**
     * Create a URL with a modified protocol
     * @param protocol The new protocol (without ':')
     */
    WithProtocol(protocol: string): Url {
        const newUrl = new URL(this.url.toString());
        newUrl.protocol = protocol + ':';
        return new Url(newUrl.toString());
    }

    /**
     * Create a URL with a modified host
     * @param host The new host
     */
    WithHost(host: string): Url {
        const newUrl = new URL(this.url.toString());
        newUrl.host = host;
        return new Url(newUrl.toString());
    }

    /**
     * Create a URL with a modified path
     * @param path The new path
     */
    WithPath(path: string): Url {
        const newUrl = new URL(this.url.toString());
        newUrl.pathname = path.startsWith('/') ? path : '/' + path;
        return new Url(newUrl.toString());
    }

    /**
     * Create a URL with added query parameters
     * @param params Key-value pairs to add to the query string
     */
    WithQueryParams(params: { [key: string]: string }): Url {
        const newUrl = new URL(this.url.toString());
        Object.entries(params).forEach(([key, value]) => {
            newUrl.searchParams.set(key, value);
        });
        return new Url(newUrl.toString());
    }

    /**
     * Create a URL with a modified fragment
     * @param fragment The new fragment (without '#')
     */
    WithFragment(fragment: string): Url {
        const newUrl = new URL(this.url.toString());
        newUrl.hash = fragment.startsWith('#') ? fragment : '#' + fragment;
        return new Url(newUrl.toString());
    }

    /**
     * Get the default port for the URL's protocol
     */
    private getDefaultPort(): number {
        const protocol = this.GetProtocol();
        switch (protocol) {
            case 'http':
                return 80;
            case 'https':
                return 443;
            case 'ftp':
                return 21;
            case 'ssh':
                return 22;
            case 'telnet':
                return 23;
            default:
                return 0; // unknown protocol
        }
    }
}

/**
 * URL utilities
 */
export class UrlUtil {
    /**
     * Encode a string for use in a URL component
     * @param str The string to encode
     */
    static Encode(str: string): string {
        return encodeURIComponent(str);
    }

    /**
     * Decode a URL-encoded string
     * @param str The string to decode
     */
    static Decode(str: string): string {
        return decodeURIComponent(str);
    }

    /**
     * Encode a string for use in a URL query
     * @param str The string to encode
     */
    static EncodeQuery(str: string): string {
        return encodeURIComponent(str);
    }

    /**
     * Parse a query string into key-value pairs
     * @param queryString The query string to parse (without '?')
     */
    static ParseQuery(queryString: string): { [key: string]: string } {
        const params: { [key: string]: string } = {};
        const pairs = queryString.split('&');
        
        for (const pair of pairs) {
            if (pair) {
                const [key, ...valueParts] = pair.split('=');
                const keyDecoded = UrlUtil.Decode(key);
                const valueDecoded = UrlUtil.Decode(valueParts.join('='));
                params[keyDecoded] = valueDecoded;
            }
        }
        
        return params;
    }

    /**
     * Build a query string from key-value pairs
     * @param params Key-value pairs to convert to query string
     */
    static BuildQuery(params: { [key: string]: string }): string {
        const pairs: string[] = [];
        Object.entries(params).forEach(([key, value]) => {
            if (value !== null && value !== undefined) {
                pairs.push(`${encodeURIComponent(key)}=${encodeURIComponent(value)}`);
            }
        });
        return pairs.join('&');
    }

    /**
     * Normalize a URL by removing extra slashes and resolving '.' and '..' components
     * @param url The URL to normalize
     */
    static Normalize(url: string): string {
        const urlObj = new Url(url);
        const path = urlObj.GetPath();
        const normalizedPath = UrlUtil.normalizePath(path);
        
        return urlObj.WithPath(normalizedPath).ToString();
    }

    /**
     * Normalize a path by resolving '.' and '..' components
     * @param path The path to normalize
     */
    private static normalizePath(path: string): string {
        const parts = path.split('/');
        const result: string[] = [];
        
        for (const part of parts) {
            if (part === '..') {
                if (result.length > 0 && result[result.length - 1] !== '..') {
                    result.pop();
                } else if (!path.startsWith('/')) {
                    // For relative paths, keep the '..' if we can't go further back
                    result.push(part);
                }
            } else if (part !== '.' && part !== '') {
                result.push(part);
            }
        }
        
        let normalizedPath = result.join('/');
        if (path.startsWith('/') && (!normalizedPath || normalizedPath[0] !== '/')) {
            normalizedPath = '/' + normalizedPath;
        }
        if (path.endsWith('/') && normalizedPath !== '/') {
            normalizedPath += '/';
        }
        
        return normalizedPath || (path.startsWith('/') ? '/' : '.');
    }
}