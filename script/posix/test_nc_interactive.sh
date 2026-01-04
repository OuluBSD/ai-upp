#!/bin/bash

echo "Killing any existing servers..."
pkill -f "run_qwen_server" 2>/dev/null || true
pkill -f "Qwen.*server" 2>/dev/null || true
sleep 2

echo "Starting server..."
./script/run_qwen_server.sh > /tmp/test_server.log 2>&1 &
SERVER_PID=$!
echo "Server PID: $SERVER_PID"
sleep 4

echo "Sending message via nc..."
echo '{"type":"user_input","content":"say hi"}' | nc localhost 7774 > /tmp/test_nc_output.txt 2>&1 &
NC_PID=$!
echo "NC PID: $NC_PID"

# Wait for response
sleep 10

# Cleanup
echo "Cleaning up..."
kill $NC_PID 2>/dev/null || true
kill $SERVER_PID 2>/dev/null || true
wait 2>/dev/null || true

echo ""
echo "====== What NC received ======"
cat /tmp/test_nc_output.txt
echo ""
echo "====== End of NC output ======"
echo ""
echo "====== Server dispatch logs ======"
grep "\[QwenClient::dispatch\]" /tmp/test_server.log | head -10
echo ""
echo "====== Server broadcast logs ======"
grep "Broadcasting\|Sending.*bytes" /tmp/test_server.log | head -10
