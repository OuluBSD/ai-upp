# Network Module

The Network module provides U++-like networking functionality implemented in TypeScript.

## Components

### HttpRequest
HTTP client class for making HTTP requests with familiar U++ patterns.

```typescript
import { HttpRequest } from '@uppts/core';

const request = new HttpRequest();
const response = await request
    .Url('https://api.example.com/data')
    .Method('GET')
    .Header('Authorization', 'Bearer token')
    .ExecuteAsync();

if (response.IsOK()) {
    console.log(response.GetBody());
}
```

### TcpSocket
TCP socket class for low-level network communication.

```typescript
import { TcpSocket } from '@uppts/core';

const socket = new TcpSocket();
await socket.ConnectAsync('example.com', 80);
await socket.SendAsync('Hello, server!');
const data = await socket.ReceiveAsync(1024);
socket.Close();
```

### WebSocket (WsSocket and WsServer)
WebSocket classes for real-time bidirectional communication.

```typescript
import { WsSocket } from '@uppts/core';

const ws = new WsSocket();
await ws.ConnectAsync('ws://localhost:8080');
ws.OnMessage((data) => {
    console.log('Received:', data);
});
await ws.SendAsync('Hello, WebSocket!');
```

### URL
URL manipulation utilities.

```typescript
import { Url, UrlUtil } from '@uppts/core';

const url = new Url('https://example.com/path?query=value');
console.log(url.GetHostname()); // example.com
console.log(url.GetQueryValue('query')); // value
```

### JSON Utilities
JSON serialization and validation.

```typescript
import { JsonSerializer, JsonValidator } from '@uppts/core';

const obj = { name: 'Example', count: 42 };
const json = JsonSerializer.Serialize(obj);
const parsed = JsonSerializer.Deserialize(json);
```

### Base64 Utilities
Base64 encoding and decoding.

```typescript
import { Base64 } from '@uppts/core';

const encoded = Base64.EncodeString('Hello, World!');
const decoded = Base64.DecodeString(encoded);
```

### Compression Utilities
Gzip, deflate, and brotli compression.

```typescript
import { Compression } from '@uppts/core';

const original = 'This is a test string for compression.';
const compressed = await Compression.Gzip(original);
const decompressed = await Compression.GzipDecompress(compressed);
```