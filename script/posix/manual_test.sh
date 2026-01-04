#!/bin/bash
# Manual test to verify server sends responses

echo "Starting server..."
./script/run_qwen_server.sh > /tmp/manual_server.log 2>&1 &
SERVER_PID=$!
echo "Server PID: $SERVER_PID"

sleep 3
echo "Server should be ready"
echo ""

echo "Sending message and waiting for response..."
{
    echo '{"type":"user_input","content":"hello"}'
    sleep 12
} | nc localhost 7774

echo ""
echo "Done with nc"

sleep 2

echo "Killing server..."
kill $SERVER_PID 2>/dev/null || true
wait $SERVER_PID 2>/dev/null || true

echo ""
echo "=== Server log (last 30 lines) ==="
tail -30 /tmp/manual_server.log
