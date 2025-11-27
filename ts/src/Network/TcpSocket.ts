/**
 * TcpSocket - TCP socket class providing U++-like API for TCP networking
 * Uses Node.js built-in net module for TCP operations
 */
import * as net from 'net';

export class TcpSocket {
    private socket: net.Socket | null = null;
    private connected: boolean = false;
    private host: string = '';
    private port: number = 0;
    private timeout: number = 30000; // 30 seconds default timeout

    /**
     * Create a new TCP socket
     */
    constructor() {
        this.socket = new net.Socket();
    }

    /**
     * Connect to a TCP server
     * @param host The host to connect to
     * @param port The port to connect to
     * @returns Promise resolving when connected
     */
    async ConnectAsync(host: string, port: number): Promise<void> {
        return new Promise((resolve, reject) => {
            this.host = host;
            this.port = port;

            // Set up timeout for connection
            const timeoutId = setTimeout(() => {
                this.socket?.destroy();
                reject(new Error(`Connection to ${host}:${port} timed out after ${this.timeout}ms`));
            }, this.timeout);

            this.socket = new net.Socket();
            
            this.socket.setTimeout(this.timeout);
            
            this.socket.on('connect', () => {
                clearTimeout(timeoutId);
                this.connected = true;
                resolve();
            });
            
            this.socket.on('error', (error) => {
                clearTimeout(timeoutId);
                reject(new Error(`Connection error to ${host}:${port}: ${error.message}`));
            });
            
            this.socket.on('timeout', () => {
                this.socket?.destroy();
                clearTimeout(timeoutId);
                reject(new Error(`Connection to ${host}:${port} timed out after ${this.timeout}ms`));
            });

            this.socket.connect(port, host);
        });
    }

    /**
     * Disconnect from the server
     */
    Close(): void {
        if (this.socket) {
            this.socket.destroy();
            this.connected = false;
        }
    }

    /**
     * Send data to the connected server
     * @param data The data to send
     * @returns Promise resolving when data is sent
     */
    async SendAsync(data: string | Buffer): Promise<void> {
        if (!this.connected || !this.socket) {
            throw new Error('Socket is not connected');
        }

        return new Promise((resolve, reject) => {
            const result = this.socket!.write(data, (error) => {
                if (error) {
                    reject(new Error(`Send error: ${error.message}`));
                } else {
                    resolve();
                }
            });
            
            if (!result) {
                // Handle backpressure
                this.socket!.once('drain', () => resolve());
            }
        });
    }

    /**
     * Receive data from the server
     * @param size Maximum number of bytes to receive
     * @returns Promise resolving to received data
     */
    async ReceiveAsync(size?: number): Promise<Buffer> {
        if (!this.connected || !this.socket) {
            throw new Error('Socket is not connected');
        }

        return new Promise((resolve, reject) => {
            let receivedData = Buffer.alloc(0);
            
            const dataHandler = (chunk: Buffer) => {
                receivedData = Buffer.concat([receivedData, chunk]);
                
                // If we've received enough data or no specific size was requested, resolve
                if (size === undefined || receivedData.length >= size) {
                    this.socket?.removeListener('data', dataHandler);
                    this.socket?.removeListener('error', errorHandler);
                    this.socket?.removeListener('close', closeHandler);
                    
                    if (size !== undefined) {
                        resolve(receivedData.subarray(0, size));
                    } else {
                        resolve(receivedData);
                    }
                }
            };
            
            const errorHandler = (error: Error) => {
                this.socket?.removeListener('data', dataHandler);
                reject(new Error(`Receive error: ${error.message}`));
            };
            
            const closeHandler = () => {
                this.socket?.removeListener('data', dataHandler);
                this.socket?.removeListener('error', errorHandler);
                
                // Resolve with whatever data we have so far
                resolve(receivedData);
            };
            
            this.socket?.on('data', dataHandler);
            this.socket?.on('error', errorHandler);
            this.socket?.on('close', closeHandler);

            // If no specific size was requested, return immediately with first chunk
            if (size === undefined) {
                this.socket?.removeListener('data', dataHandler);
            }
        });
    }

    /**
     * Check if the socket is connected
     */
    IsConnected(): boolean {
        return this.connected && this.socket !== null && !this.socket.destroyed;
    }

    /**
     * Get the local address of the socket
     */
    GetLocalAddress(): string | null {
        if (this.socket && this.connected) {
            return this.socket.localAddress || null;
        }
        return null;
    }

    /**
     * Get the local port of the socket
     */
    GetLocalPort(): number | null {
        if (this.socket && this.connected) {
            return this.socket.localPort || null;
        }
        return null;
    }

    /**
     * Get the remote address of the socket
     */
    GetRemoteAddress(): string | null {
        if (this.socket && this.connected) {
            return this.socket.remoteAddress || null;
        }
        return null;
    }

    /**
     * Get the remote port of the socket
     */
    GetRemotePort(): number | null {
        if (this.socket && this.connected) {
            return this.socket.remotePort || null;
        }
        return null;
    }

    /**
     * Set the socket timeout
     * @param milliseconds Timeout in milliseconds
     */
    SetTimeout(milliseconds: number): void {
        this.timeout = milliseconds;
        if (this.socket) {
            this.socket.setTimeout(milliseconds);
        }
    }

    /**
     * Get the socket timeout
     */
    GetTimeout(): number {
        return this.timeout;
    }

    /**
     * Enable/disable Nagle's algorithm
     * @param noDelay Whether to disable Nagle's algorithm
     */
    SetNoDelay(noDelay: boolean = true): void {
        if (this.socket) {
            this.socket.setNoDelay(noDelay);
        }
    }

    /**
     * Set keep-alive on the socket
     * @param enable Whether to enable keep-alive
     * @param initialDelay Initial delay in milliseconds
     */
    SetKeepAlive(enable: boolean = true, initialDelay?: number): void {
        if (this.socket) {
            this.socket.setKeepAlive(enable, initialDelay);
        }
    }
}

/**
 * TcpServer - TCP server class to accept incoming connections
 */
export class TcpServer {
    private server: net.Server | null = null;
    private listening: boolean = false;
    private port: number = 0;
    private host: string = 'localhost';

    /**
     * Create a new TCP server
     */
    constructor() {
        this.server = net.createServer();
    }

    /**
     * Listen for incoming connections
     * @param port Port to listen on
     * @param host Host to listen on (default 'localhost')
     * @returns Promise resolving when server is listening
     */
    async ListenAsync(port: number, host: string = 'localhost'): Promise<void> {
        return new Promise((resolve, reject) => {
            this.port = port;
            this.host = host;
            
            this.server?.on('error', (error) => {
                reject(new Error(`Server error: ${error.message}`));
            });
            
            this.server?.listen(port, host, () => {
                this.listening = true;
                resolve();
            });
        });
    }

    /**
     * Accept an incoming connection
     * @returns Promise resolving to accepted socket
     */
    async AcceptAsync(): Promise<TcpSocket> {
        if (!this.server) {
            throw new Error('Server not created');
        }

        return new Promise((resolve, reject) => {
            this.server?.once('connection', (socket) => {
                const tcpSocket = new TcpSocket();
                tcpSocket['socket'] = socket;
                tcpSocket['connected'] = true;
                tcpSocket['host'] = socket.remoteAddress || '';
                tcpSocket['port'] = socket.remotePort || 0;
                
                resolve(tcpSocket);
            });
            
            this.server?.on('error', (error) => {
                reject(new Error(`Accept error: ${error.message}`));
            });
        });
    }

    /**
     * Close the server
     * @returns Promise resolving when server is closed
     */
    async CloseAsync(): Promise<void> {
        if (!this.server) {
            return Promise.resolve();
        }
        
        return new Promise((resolve) => {
            this.server?.close(() => {
                this.listening = false;
                resolve();
            });
        });
    }

    /**
     * Check if the server is listening
     */
    IsListening(): boolean {
        return this.listening;
    }

    /**
     * Get the listening port
     */
    GetPort(): number {
        return this.port;
    }

    /**
     * Get the listening host
     */
    GetHost(): string {
        return this.host;
    }
}