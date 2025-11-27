/**
 * WebSocket - WebSocket client class similar to U++ WebSocket functionality
 * Uses Node.js ws library for WebSocket operations
 */
import WebSocket from 'ws';
import { Server as WebSocketServer, ServerOptions as WebSocketServerOptions } from 'ws';
import { IncomingMessage } from 'http';

export class WsSocket {
    private ws: WebSocket | null = null;
    private connected: boolean = false;
    private url: string = '';
    private options: WebSocket.ClientOptions;

    /**
     * Create a new WebSocket connection
     * @param options WebSocket client options
     */
    constructor(options?: WebSocket.ClientOptions) {
        this.options = options || {};
    }

    /**
     * Connect to a WebSocket server
     * @param url The WebSocket URL to connect to
     * @returns Promise resolving when connected
     */
    async ConnectAsync(url: string): Promise<void> {
        return new Promise((resolve, reject) => {
            this.url = url;
            
            // Create the WebSocket connection
            this.ws = new WebSocket(url, this.options);
            
            this.ws.on('open', () => {
                this.connected = true;
                resolve();
            });
            
            this.ws.on('error', (error) => {
                this.connected = false;
                reject(new Error(`WebSocket connection error: ${error.message}`));
            });
            
            this.ws.on('close', () => {
                this.connected = false;
            });
        });
    }

    /**
     * Disconnect from the server
     */
    Close(code?: number, reason?: string): void {
        if (this.ws) {
            this.ws.close(code, reason);
            this.connected = false;
        }
    }

    /**
     * Send data through the WebSocket
     * @param data The data to send
     * @returns Promise resolving when data is sent
     */
    async SendAsync(data: string | Buffer): Promise<void> {
        if (!this.connected || !this.ws) {
            throw new Error('WebSocket is not connected');
        }

        return new Promise((resolve, reject) => {
            this.ws?.send(data, (error) => {
                if (error) {
                    reject(new Error(`WebSocket send error: ${error.message}`));
                } else {
                    resolve();
                }
            });
        });
    }

    /**
     * Receive data from the WebSocket
     * This method sets up a callback to handle incoming messages
     * @param callback Function to handle received messages
     */
    OnMessage(callback: (data: WebSocket.Data) => void): void {
        if (this.ws) {
            this.ws.on('message', callback);
        }
    }

    /**
     * Check if the WebSocket is connected
     */
    IsConnected(): boolean {
        return this.connected && this.ws !== null && this.ws.readyState === WebSocket.OPEN;
    }

    /**
     * Get the ready state of the WebSocket
     */
    GetReadyState(): number {
        if (!this.ws) {
            return WebSocket.CLOSED;
        }
        return this.ws.readyState;
    }

    /**
     * Set up error handling
     * @param callback Function to handle errors
     */
    OnError(callback: (error: Error) => void): void {
        if (this.ws) {
            this.ws.on('error', (error: Error) => callback(error));
        }
    }

    /**
     * Set up close event handling
     * @param callback Function to handle close events
     */
    OnClose(callback: (code: number, reason: string) => void): void {
        if (this.ws) {
            this.ws.on('close', (code: number, reason: string) => callback(code, reason));
        }
    }

    /**
     * Set up open event handling
     * @param callback Function to handle open events
     */
    OnOpen(callback: () => void): void {
        if (this.ws) {
            this.ws.on('open', callback);
        }
    }
}

/**
 * WsServer - WebSocket server class to handle incoming WebSocket connections
 */
export class WsServer {
    private server: WebSocketServer | null = null;
    private listening: boolean = false;
    private port: number = 0;
    private host: string = 'localhost';

    /**
     * Create a new WebSocket server
     * @param options WebSocket server options
     */
    constructor(options?: WebSocketServerOptions) {
        this.server = new WebSocketServer(options || {});
    }

    /**
     * Listen for incoming WebSocket connections
     * @param port Port to listen on
     * @param host Host to listen on (default 'localhost')
     * @returns Promise resolving when server is listening
     */
    async ListenAsync(port: number, host: string = 'localhost'): Promise<void> {
        return new Promise((resolve, reject) => {
            this.server = new WebSocketServer({ port, host });

            this.server.on('listening', () => {
                this.listening = true;
                this.port = port;
                this.host = host;
                resolve();
            });

            this.server.on('error', (error: Error) => {
                reject(new Error(`WebSocket server error: ${error.message}`));
            });

            this.server.on('connection', (ws: WebSocket) => {
                // Handle new connection
            });
        });
    }

    /**
     * Accept an incoming connection
     * This method sets up a callback to handle new connections
     * @param callback Function to handle new connections
     */
    OnConnection(callback: (socket: WsSocket) => void): void {
        if (this.server) {
            this.server.on('connection', (ws: WebSocket) => {
                // Create a WsSocket wrapper for the raw WebSocket
                const wsSocket = new WsSocket();
                wsSocket['ws'] = ws;
                wsSocket['connected'] = true;

                callback(wsSocket);
            });
        }
    }

    /**
     * Close the server
     * @returns Promise resolving when server is closed
     */
    async CloseAsync(): Promise<void> {
        if (!this.server) {
            return Promise.resolve();
        }

        return new Promise((resolve, reject) => {
            this.server?.close((error: Error | undefined) => {
                if (error) {
                    reject(new Error(`WebSocket server close error: ${error.message}`));
                } else {
                    this.listening = false;
                    resolve();
                }
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

    /**
     * Broadcast a message to all connected clients
     * @param data The data to broadcast
     */
    Broadcast(data: string | Buffer): void {
        if (this.server) {
            this.server.clients.forEach((client: WebSocket) => {
                if (client.readyState === WebSocket.OPEN) {
                    client.send(data);
                }
            });
        }
    }
}