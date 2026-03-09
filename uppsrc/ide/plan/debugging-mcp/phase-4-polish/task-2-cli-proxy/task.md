# Task 4-2: CLI Proxy Robustness

## Overview

Flesh out `uppsrc/dbg/TcpProxy.cpp` with full robust framing, auto-reconnect, and heartbeat.

## Framing Protocol (Verify Against McpServerCore)

Check `uppsrc/MCP/Server.cpp::ReadFramed` for the exact wire format.
Expected format (to verify):
```
<decimal_length>\n<json_bytes_of_that_length>
```

Implement matching `WriteFramed` and `ReadFramed` in `DbgMcpCli`.

```cpp
void DbgMcpCli::WriteFramed(TcpSocket& sock, const String& json)
{
    String frame = AsString(json.GetCount()) + "\n" + json;
    sock.Put(frame);
}

bool DbgMcpCli::ReadFramed(TcpSocket& sock, String& out_json)
{
    // Read until '\n' to get length
    String len_str;
    for(;;) {
        int c = sock.Get();
        if(c < 0) return false; // disconnected
        if(c == '\n') break;
        len_str.Cat(c);
    }
    int len = StrInt(len_str);
    if(len <= 0 || len > 4 * 1024 * 1024) return false;
    out_json = sock.Get(len);
    return out_json.GetCount() == len;
}
```

## Loop Implementation

```cpp
int DbgMcpCli::Loop()
{
    TcpSocket sock;
    if(!sock.Connect(host, port)) {
        Cerr() << "dbg: cannot connect to " << host << ":" << port << "\n";
        return 1;
    }
    Cerr() << "dbg: connected to " << host << ":" << port << "\n";

    for(;;) {
        // Read one line from stdin
        String line = ReadStdinLine();
        if(line.IsVoid()) break;  // EOF
        line = TrimBoth(line);
        if(line.IsEmpty()) continue;

        // Validate JSON
        Value v = ParseJSON(line);
        if(v.IsError()) {
            // Write error response to stdout
            Cout() << MakeError("null", PARSE_ERROR, "Invalid JSON") << "\n";
            Cout().Flush();
            continue;
        }

        // Forward to server
        WriteFramed(sock, line);

        // Read response
        String resp;
        if(!ReadFramed(sock, resp)) {
            Cerr() << "dbg: server disconnected\n";
            // Attempt reconnect (Phase 4 robustness)
            break;
        }

        // Write response to stdout
        Cout() << resp << "\n";
        Cout().Flush();
    }
    return 0;
}
```

## Stdin Reading

U++ headless: use `Cin()` for stdin.  `Cin().GetLine()` reads one line.
On EOF, `Cin().IsEof()` returns true.

```cpp
String ReadStdinLine() {
    if(Cin().IsEof()) return Null;  // U++ Null = void String
    return Cin().GetLine();
}
```

## Auto-Reconnect

```cpp
bool DbgMcpCli::Connect(const String& h, int p)
{
    host = h; port = p;
    return ReconnectLoop(/*initial=*/true);
}

bool DbgMcpCli::ReconnectLoop(bool initial)
{
    int delay = 1000;
    int attempts = initial ? 1 : 5;
    for(int i = 0; i < attempts; i++) {
        if(sock_.Connect(host, port)) return true;
        if(i < attempts - 1) {
            Sleep(delay);
            delay = min(delay * 2, 30000);
        }
    }
    return false;
}
```

## Heartbeat

Every 30 seconds in `Loop()`, if stdin has been idle:
```cpp
// Send ping
WriteFramed(sock, "{\"jsonrpc\":\"2.0\",\"id\":\"__hb\",\"method\":\"mcp.ping\",\"params\":{}}");
String pong;
if(!ReadFramed(sock, pong)) { /* reconnect */ }
// Discard heartbeat responses (id == "__hb")
```

Simpler: skip heartbeat in Phase 4 if stdin-based loop naturally keeps connection alive.

## OneShot Implementation

```cpp
int DbgMcpCli::OneShot(const String& method, const String& params_json)
{
    TcpSocket sock;
    if(!sock.Connect(host, port)) {
        Cerr() << "dbg: cannot connect\n";
        return 1;
    }
    String req = "{\"jsonrpc\":\"2.0\",\"id\":\"1\",\"method\":\""
               + method + "\",\"params\":" + params_json + "}";
    WriteFramed(sock, req);
    String resp;
    if(!ReadFramed(sock, resp)) {
        Cerr() << "dbg: no response\n";
        return 1;
    }
    Cout() << resp << "\n";
    return 0;
}
```

## Testing the CLI

With TheIDE running:
```bash
# One-shot
bin/dbg debug.state
bin/dbg debug.breakpoint.set '{"file":"main.cpp","line":5}'

# Interactive (pipe JSON-RPC requests)
echo '{"jsonrpc":"2.0","id":"1","method":"debug.state","params":{}}' | bin/dbg

# Verify with mcp_client.sh
uppsrc/ide/MCP/mcp_client.sh
```

## Status: DONE
