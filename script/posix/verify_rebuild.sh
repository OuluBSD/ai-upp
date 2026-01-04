#!/bin/bash

echo "=== Testing EOF Fix ==="
echo ""

pkill -f "qwen.*server" 2>/dev/null
sleep 2

echo "Starting server..."
./script/run_qwen_server.sh > /tmp/eof_test.log 2>&1 &
SERVER_PID=$!
echo "Server PID: $SERVER_PID"
sleep 5

echo "Sending message with nc (which will close write side)..."
echo '{"type":"user_input","content":"Say hi"}' | timeout 20 nc localhost 7774 &
NC_PID=$!
echo "NC PID: $NC_PID"

# Wait for AI response
sleep 12

# Kill server
kill $SERVER_PID 2>/dev/null
wait 2>/dev/null

echo ""
echo "=== Client Connection Status ==="
grep "closed write side\|Client disconnected\|Broadcasting to" /tmp/eof_test.log | head -10 || echo "(not found)"

echo ""
echo "=== Handler Calls ==="
grep "on_conversation handler" /tmp/eof_test.log | head -5 || echo "(not found)"

echo ""
echo "=== Send Response Calls ==="
grep "Successfully sent response\|Failed to send" /tmp/eof_test.log | head -10 || echo "(not found)"

echo ""
echo "=== Full log available at /tmp/eof_test.log ==="
