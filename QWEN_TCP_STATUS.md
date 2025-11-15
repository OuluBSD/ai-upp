# Qwen TCP Server Status

## Current Status

The Qwen TCP server integration is **partially working**:

✅ **Working**:
- TCP server accepts connections on port 7774
- Messages are received from TCP clients
- Messages are forwarded to qwen-code subprocess
- qwen-code subprocess generates AI responses
- Responses are logged by qwen-code (see "[ServerMode] Sent conversation message to client")

❌ **Not Working**:
- Responses are NOT forwarded back to TCP clients
- TCP clients receive nothing

## Root Cause

The issue is in `QwenTCPServer::server_thread()` (uppsrc/Qwen/QwenTCPServer.cpp):

```cpp
// Line 297-303
if (qwen_client_ && qwen_client_->is_running()) {
    int msg_count = qwen_client_->poll_messages(0); // Non-blocking poll
    // ...
}
```

**Problem**: `poll_messages(0)` is called with timeout=0 (non-blocking), meaning it returns immediately if no data is available RIGHT NOW. Since AI responses arrive asynchronously (several seconds after the request), the poll call happens between responses and misses them.

## Evidence from Logs

When testing with `nc localhost 7774`, server logs show:

1. ✅ Message received: `[QwenTCPServer] Processing message from client 6: {"type":"user_input","content":"hello"}`
2. ✅ Sent to qwen-code: `[QwenClient] Sending: {"type":"user_input","content":"hello"}`
3. ✅ qwen-code generates response: `[ServerMode] Content event received. Length: 5 Content: Hello`
4. ✅ qwen-code logs sending: `[ServerMode] Sent conversation message to client`
5. ❌ **But QwenClient never reads these messages from subprocess stdout**
6. ❌ Therefore handlers are never called
7. ❌ Therefore nothing is sent to TCP client

## Solutions

### Option 1: Call poll_messages() in a loop
```cpp
// Keep polling until no more messages
while (qwen_client_->poll_messages(0) > 0) {
    // Messages processed via handlers
}
```

### Option 2: Use blocking poll with timeout
```cpp
// Wait up to 10ms for messages
qwen_client_->poll_messages(10);
```

### Option 3: Poll more aggressively
Call `poll_messages()` multiple times per server loop iteration, or reduce the server poll timeout from 100ms to something smaller.

## Test Procedure

### Manual Test (works, server responds):
```bash
# Terminal 1
./script/run_qwen_server.sh

# Terminal 2
nc localhost 7774
{"type":"user_input","content":"hello"}
# Type the above and press Enter
# Wait 5-10 seconds
# Should see JSON responses (but currently doesn't)
```

### Automated Test:
```bash
./script/test_qwen_interaction.sh
```

## Files Modified

- `uppsrc/Qwen/QwenTCPServer.cpp` - Added POLLHUP fix (line 257), debug logging
- `uppsrc/Qwen/QwenClient.cpp` - Added debug logging in dispatch_message()
- `script/test_qwen_interaction.sh` - Test script for server/client
- `script/test_qwen_client.sh` - Client helper script
- `script/test_nc_interactive.sh` - Interactive test script

## Next Steps

1. Implement one of the solutions above (Option 1 recommended)
2. Test with `./script/test_qwen_interaction.sh`
3. Verify TCP clients receive responses
4. Remove debug logging once working
5. Test with actual Claude Code client

## Architecture Diagram

```
TCP Client (nc)
    |
    | JSON over TCP
    ↓
QwenTCPServer ←→ QwenClient ←→ qwen-code subprocess
    |              |  (stdin/stdout)      |
    |              |                      |
    |              ↓                      ↓
    |         poll_messages()        Gemini AI
    |         [BROKEN HERE]
    |              |
    |              ↓
    |         on_conversation
    |         handler
    |              |
    ←──────────────┘
    |
    ↓
TCP Client (receives response)
```

The break is at the `poll_messages()` call - it's not reading messages from the subprocess because it's non-blocking and the messages arrive later.
