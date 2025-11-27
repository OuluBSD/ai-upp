/**
 * Compression - Compression and decompression utilities similar to U++ compression functionality
 * Uses Node.js built-in zlib for compression/decompression
 */
import * as zlib from 'zlib';

export class Compression {
    /**
     * Compress data using gzip
     * @param data The data to compress (string or Buffer)
     * @returns Promise resolving to compressed Buffer
     */
    static async Gzip(data: string | Buffer): Promise<Buffer> {
        const buffer = typeof data === 'string' ? Buffer.from(data, 'utf8') : data;
        return new Promise((resolve, reject) => {
            zlib.gzip(buffer, (error, result) => {
                if (error) {
                    reject(new Error(`Gzip compression error: ${error.message}`));
                } else {
                    resolve(result);
                }
            });
        });
    }

    /**
     * Decompress gzip-compressed data
     * @param data The compressed data to decompress
     * @returns Promise resolving to decompressed Buffer
     */
    static async GzipDecompress(data: Buffer): Promise<Buffer> {
        return new Promise((resolve, reject) => {
            zlib.gunzip(data, (error, result) => {
                if (error) {
                    reject(new Error(`Gzip decompression error: ${error.message}`));
                } else {
                    resolve(result);
                }
            });
        });
    }

    /**
     * Compress data using deflate
     * @param data The data to compress (string or Buffer)
     * @returns Promise resolving to compressed Buffer
     */
    static async Deflate(data: string | Buffer): Promise<Buffer> {
        const buffer = typeof data === 'string' ? Buffer.from(data, 'utf8') : data;
        return new Promise((resolve, reject) => {
            zlib.deflate(buffer, (error, result) => {
                if (error) {
                    reject(new Error(`Deflate compression error: ${error.message}`));
                } else {
                    resolve(result);
                }
            });
        });
    }

    /**
     * Decompress deflate-compressed data
     * @param data The compressed data to decompress
     * @returns Promise resolving to decompressed Buffer
     */
    static async DeflateDecompress(data: Buffer): Promise<Buffer> {
        return new Promise((resolve, reject) => {
            zlib.inflate(data, (error, result) => {
                if (error) {
                    reject(new Error(`Deflate decompression error: ${error.message}`));
                } else {
                    resolve(result);
                }
            });
        });
    }

    /**
     * Compress data using brotli
     * @param data The data to compress (string or Buffer)
     * @returns Promise resolving to compressed Buffer
     */
    static async Brotli(data: string | Buffer): Promise<Buffer> {
        const buffer = typeof data === 'string' ? Buffer.from(data, 'utf8') : data;
        return new Promise((resolve, reject) => {
            zlib.brotliCompress(buffer, (error, result) => {
                if (error) {
                    reject(new Error(`Brotli compression error: ${error.message}`));
                } else {
                    resolve(result);
                }
            });
        });
    }

    /**
     * Decompress brotli-compressed data
     * @param data The compressed data to decompress
     * @returns Promise resolving to decompressed Buffer
     */
    static async BrotliDecompress(data: Buffer): Promise<Buffer> {
        return new Promise((resolve, reject) => {
            zlib.brotliDecompress(data, (error, result) => {
                if (error) {
                    reject(new Error(`Brotli decompression error: ${error.message}`));
                } else {
                    resolve(result);
                }
            });
        });
    }

    /**
     * Compress data using multiple compression algorithms
     * @param data The data to compress (string or Buffer)
     * @param algorithm The compression algorithm to use ('gzip', 'deflate', or 'brotli')
     * @returns Promise resolving to compressed Buffer
     */
    static async Compress(data: string | Buffer, algorithm: 'gzip' | 'deflate' | 'brotli' = 'gzip'): Promise<Buffer> {
        switch (algorithm) {
            case 'gzip':
                return Compression.Gzip(data);
            case 'deflate':
                return Compression.Deflate(data);
            case 'brotli':
                return Compression.Brotli(data);
            default:
                throw new Error(`Unknown compression algorithm: ${algorithm}`);
        }
    }

    /**
     * Decompress data using the appropriate algorithm based on content
     * @param data The compressed data to decompress
     * @param algorithm The compression algorithm that was used ('gzip', 'deflate', or 'brotli')
     * @returns Promise resolving to decompressed Buffer
     */
    static async Decompress(data: Buffer, algorithm: 'gzip' | 'deflate' | 'brotli' = 'gzip'): Promise<Buffer> {
        switch (algorithm) {
            case 'gzip':
                return Compression.GzipDecompress(data);
            case 'deflate':
                return Compression.DeflateDecompress(data);
            case 'brotli':
                return Compression.BrotliDecompress(data);
            default:
                throw new Error(`Unknown compression algorithm: ${algorithm}`);
        }
    }

    /**
     * Compress a string to a compressed string using Base64 encoding
     * @param str The string to compress
     * @param algorithm The compression algorithm to use
     * @returns Promise resolving to Base64 encoded compressed string
     */
    static async CompressString(str: string, algorithm: 'gzip' | 'deflate' | 'brotli' = 'gzip'): Promise<string> {
        const compressed = await Compression.Compress(str, algorithm);
        return compressed.toString('base64');
    }

    /**
     * Decompress a Base64 encoded compressed string back to original string
     * @param compressedStr The Base64 encoded compressed string
     * @param algorithm The compression algorithm that was used
     * @returns Promise resolving to decompressed string
     */
    static async DecompressString(compressedStr: string, algorithm: 'gzip' | 'deflate' | 'brotli' = 'gzip'): Promise<string> {
        const compressed = Buffer.from(compressedStr, 'base64');
        const decompressed = await Compression.Decompress(compressed, algorithm);
        return decompressed.toString('utf8');
    }

    /**
     * Calculate compression ratio
     * @param originalSize Original size in bytes
     * @param compressedSize Compressed size in bytes
     * @returns Compression ratio as a number (e.g., 0.5 means 50% of original size)
     */
    static CalculateRatio(originalSize: number, compressedSize: number): number {
        if (originalSize === 0) return 1; // Unchanged if original size is 0
        return compressedSize / originalSize;
    }

    /**
     * Calculate compression percentage
     * @param originalSize Original size in bytes
     * @param compressedSize Compressed size in bytes
     * @returns Compression percentage (e.g., 50 means 50% size reduction)
     */
    static CalculatePercentage(originalSize: number, compressedSize: number): number {
        if (originalSize === 0) return 0; // No compression if original size is 0
        return 100 * (1 - compressedSize / originalSize);
    }
}

/**
 * Compression utilities for common operations
 */
export class CompressionUtil {
    /**
     * Compress an object to a compressed string representation
     * @param obj The object to compress
     * @param algorithm The compression algorithm to use
     * @returns Promise resolving to Base64 encoded compressed string
     */
    static async CompressObject(obj: any, algorithm: 'gzip' | 'deflate' | 'brotli' = 'gzip'): Promise<string> {
        const jsonString = JSON.stringify(obj);
        return Compression.CompressString(jsonString, algorithm);
    }

    /**
     * Decompress a compressed string representation back to an object
     * @param compressedStr The Base64 encoded compressed string
     * @param algorithm The compression algorithm that was used
     * @returns Promise resolving to the decompressed object
     */
    static async DecompressObject(compressedStr: string, algorithm: 'gzip' | 'deflate' | 'brotli' = 'gzip'): Promise<any> {
        const jsonString = await Compression.DecompressString(compressedStr, algorithm);
        return JSON.parse(jsonString);
    }

    /**
     * Compress a file using the specified algorithm
     * @param inputPath Path to input file
     * @param outputPath Path to output compressed file
     * @param algorithm The compression algorithm to use
     * @returns Promise resolving when compression is complete
     */
    static async CompressFile(
        inputPath: string,
        outputPath: string,
        algorithm: 'gzip' | 'deflate' | 'brotli' = 'gzip'
    ): Promise<void> {
        const fs = await import('fs');
        const input = fs.createReadStream(inputPath);
        let output: any;

        // Create appropriate compression stream based on algorithm
        switch (algorithm) {
            case 'gzip':
                output = input.pipe(zlib.createGzip());
                break;
            case 'deflate':
                output = input.pipe(zlib.createDeflate());
                break;
            case 'brotli':
                output = input.pipe(zlib.createBrotliCompress());
                break;
            default:
                throw new Error(`Unknown compression algorithm: ${algorithm}`);
        }

        const outStream = fs.createWriteStream(outputPath);
        return new Promise((resolve, reject) => {
            output.pipe(outStream);
            output.on('end', () => resolve());
            output.on('error', (error: Error) => reject(error));
        });
    }

    /**
     * Decompress a file using the specified algorithm
     * @param inputPath Path to input compressed file
     * @param outputPath Path to output decompressed file
     * @param algorithm The compression algorithm that was used
     * @returns Promise resolving when decompression is complete
     */
    static async DecompressFile(
        inputPath: string,
        outputPath: string,
        algorithm: 'gzip' | 'deflate' | 'brotli' = 'gzip'
    ): Promise<void> {
        const fs = await import('fs');
        const input = fs.createReadStream(inputPath);
        let output: any;

        // Create appropriate decompression stream based on algorithm
        switch (algorithm) {
            case 'gzip':
                output = input.pipe(zlib.createGunzip());
                break;
            case 'deflate':
                output = input.pipe(zlib.createInflate());
                break;
            case 'brotli':
                output = input.pipe(zlib.createBrotliDecompress());
                break;
            default:
                throw new Error(`Unknown compression algorithm: ${algorithm}`);
        }

        const outStream = fs.createWriteStream(outputPath);
        return new Promise((resolve, reject) => {
            output.pipe(outStream);
            output.on('end', () => resolve());
            output.on('error', (error: Error) => reject(error));
        });
    }

    /**
     * Compare compression algorithms for a given data
     * @param data The data to compress with different algorithms
     * @returns Promise resolving to compression results for each algorithm
     */
    static async CompareAlgorithms(data: string | Buffer): Promise<{
        gzip: { compressed: Buffer, ratio: number, percentage: number },
        deflate: { compressed: Buffer, ratio: number, percentage: number },
        brotli: { compressed: Buffer, ratio: number, percentage: number }
    }> {
        const originalSize = typeof data === 'string' ? Buffer.from(data, 'utf8').length : data.length;
        
        const gzipCompressed = await Compression.Gzip(data);
        const deflateCompressed = await Compression.Deflate(data);
        const brotliCompressed = await Compression.Brotli(data);
        
        return {
            gzip: {
                compressed: gzipCompressed,
                ratio: Compression.CalculateRatio(originalSize, gzipCompressed.length),
                percentage: Compression.CalculatePercentage(originalSize, gzipCompressed.length)
            },
            deflate: {
                compressed: deflateCompressed,
                ratio: Compression.CalculateRatio(originalSize, deflateCompressed.length),
                percentage: Compression.CalculatePercentage(originalSize, deflateCompressed.length)
            },
            brotli: {
                compressed: brotliCompressed,
                ratio: Compression.CalculateRatio(originalSize, brotliCompressed.length),
                percentage: Compression.CalculatePercentage(originalSize, brotliCompressed.length)
            }
        };
    }
}