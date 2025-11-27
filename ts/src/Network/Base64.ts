/**
 * Base64 - Base64 encoding and decoding utilities similar to U++ Base64 functionality
 * Uses Node.js built-in Buffer for Base64 operations
 */

export class Base64 {
    /**
     * Encode a string to Base64
     * @param str The string to encode
     * @returns Base64 encoded string
     */
    static EncodeString(str: string): string {
        // Create a buffer from the string and encode to base64
        return Buffer.from(str, 'utf8').toString('base64');
    }

    /**
     * Decode a Base64 encoded string
     * @param base64Str The Base64 string to decode
     * @returns Decoded string
     */
    static DecodeString(base64Str: string): string {
        // Create a buffer from the base64 string and decode to utf8
        return Buffer.from(base64Str, 'base64').toString('utf8');
    }

    /**
     * Encode a buffer to Base64
     * @param buffer The buffer to encode
     * @returns Base64 encoded string
     */
    static EncodeBuffer(buffer: Buffer): string {
        return buffer.toString('base64');
    }

    /**
     * Decode a Base64 string to a buffer
     * @param base64Str The Base64 string to decode
     * @returns Buffer containing the decoded data
     */
    static DecodeBuffer(base64Str: string): Buffer {
        return Buffer.from(base64Str, 'base64');
    }

    /**
     * Encode bytes (Uint8Array) to Base64
     * @param bytes The bytes to encode
     * @returns Base64 encoded string
     */
    static EncodeBytes(bytes: Uint8Array): string {
        return Buffer.from(bytes).toString('base64');
    }

    /**
     * Decode a Base64 string to bytes (Uint8Array)
     * @param base64Str The Base64 string to decode
     * @returns Uint8Array containing the decoded bytes
     */
    static DecodeBytes(base64Str: string): Uint8Array {
        return new Uint8Array(Buffer.from(base64Str, 'base64'));
    }

    /**
     * Check if a string is valid Base64
     * @param str The string to check
     * @returns True if the string is valid Base64, false otherwise
     */
    static IsValidBase64(str: string): boolean {
        if (typeof str !== 'string') {
            return false;
        }

        // Check if the string contains only valid Base64 characters
        if (!/^[A-Za-z0-9+/]*={0,2}$/.test(str)) {
            return false;
        }

        // Check if the length is valid (multiple of 4)
        if (str.length % 4 !== 0) {
            return false;
        }

        try {
            // Try to decode and re-encode to verify
            const buffer = Buffer.from(str, 'base64');
            const reencoded = buffer.toString('base64');
            return reencoded === str;
        } catch (error) {
            return false;
        }
    }

    /**
     * Pad a Base64 string to make its length a multiple of 4
     * @param base64Str The Base64 string to pad
     * @returns Padded Base64 string
     */
    static Pad(base64Str: string): string {
        const padding = base64Str.length % 4;
        if (padding === 0) return base64Str;
        return base64Str + '='.repeat(4 - padding);
    }

    /**
     * Remove padding from a Base64 string (URL-safe Base64)
     * @param base64Str The Base64 string to remove padding from
     * @returns Base64 string without padding
     */
    static RemovePadding(base64Str: string): string {
        return base64Str.replace(/=+$/, '');
    }

    /**
     * Encode a string to URL-safe Base64
     * @param str The string to encode
     * @returns URL-safe Base64 encoded string (no padding)
     */
    static EncodeUrlSafe(str: string): string {
        return Base64.EncodeString(str)
            .replace(/\+/g, '-')
            .replace(/\//g, '_')
            .replace(/=/g, '');
    }

    /**
     * Decode a URL-safe Base64 string
     * @param base64Str The URL-safe Base64 string to decode
     * @returns Decoded string
     */
    static DecodeUrlSafe(base64Str: string): string {
        // Restore URL-safe Base64 to regular Base64 format
        let regularBase64 = base64Str
            .replace(/-/g, '+')
            .replace(/_/g, '/');
        
        // Add padding if needed
        const padding = regularBase64.length % 4;
        if (padding !== 0) {
            regularBase64 += '='.repeat(4 - padding);
        }
        
        return Base64.DecodeString(regularBase64);
    }
}

/**
 * Base64 utilities for common operations
 */
export class Base64Util {
    /**
     * Encode a JSON object to Base64
     * @param obj The JSON object to encode
     * @returns Base64 encoded string of the JSON
     */
    static EncodeJson(obj: any): string {
        const jsonStr = JSON.stringify(obj);
        return Base64.EncodeString(jsonStr);
    }

    /**
     * Decode a Base64 string to a JSON object
     * @param base64Str The Base64 string to decode
     * @returns The decoded JSON object
     */
    static DecodeJson(base64Str: string): any {
        const jsonStr = Base64.DecodeString(base64Str);
        return JSON.parse(jsonStr);
    }

    /**
     * Encode binary data in a data URL format
     * @param mimeType The MIME type of the data
     * @param data The binary data to encode
     * @returns Data URL string
     */
    static EncodeDataUrl(mimeType: string, data: Uint8Array | Buffer | string): string {
        let base64Data: string;
        
        if (typeof data === 'string') {
            base64Data = Base64.EncodeString(data);
        } else if (data instanceof Buffer) {
            base64Data = Base64.EncodeBuffer(data);
        } else {
            base64Data = Base64.EncodeBytes(data);
        }
        
        return `data:${mimeType};base64,${base64Data}`;
    }

    /**
     * Decode a data URL to get the MIME type and binary data
     * @param dataUrl The data URL to decode
     * @returns Object containing mimeType and data
     */
    static DecodeDataUrl(dataUrl: string): { mimeType: string, data: Buffer } | null {
        // Match data URLs: data:[<media type>][;base64],<data>
        const match = dataUrl.match(/^data:([^,]+),(.*)$/);
        if (!match) {
            return null;
        }
        
        const fullMimeType = match[1];
        const isBase64 = fullMimeType.includes(';base64');
        const mimeType = isBase64 ? fullMimeType.replace(';base64', '') : fullMimeType;
        const dataPart = match[2];
        
        let buffer: Buffer;
        if (isBase64) {
            buffer = Base64.DecodeBuffer(dataPart);
        } else {
            // URL-safe decoding for non-base64 data
            buffer = Buffer.from(decodeURIComponent(dataPart), 'ascii');
        }
        
        return { mimeType, data: buffer };
    }

    /**
     * Split a long Base64 string into chunks
     * @param base64Str The Base64 string to split
     * @param chunkSize The size of each chunk (default 76 characters)
     * @returns Array of Base64 string chunks
     */
    static SplitToChunks(base64Str: string, chunkSize: number = 76): string[] {
        const chunks: string[] = [];
        for (let i = 0; i < base64Str.length; i += chunkSize) {
            chunks.push(base64Str.substring(i, i + chunkSize));
        }
        return chunks;
    }

    /**
     * Join Base64 chunks back together
     * @param chunks Array of Base64 string chunks
     * @returns Combined Base64 string
     */
    static JoinChunks(chunks: string[]): string {
        return chunks.join('');
    }
}