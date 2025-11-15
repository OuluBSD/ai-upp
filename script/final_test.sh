#!/bin/bash

echo "=== Final Test with Poll Loop Fix ==="
echo ""

pkill -f "qwen.*server" 2>/dev/null
sleep 2

echo "Starting server..."
./script/run_qwen_server.sh > /tmp/final_test.log 2>&1 &
SERVER_PID=$!
echo "Server PID: $SERVER_PID"
sleep 4

echo "Sending message and waiting for response..."
echo ""
echo '{"type":"user_input","content":"hello"}' | timeout 15 nc localhost 7774 &
NC_PID=$!
echo ""

# Wait for nc to finish or timeout
wait $NC_PID 2>/dev/null
echo "NC finished, waiting a bit more..."
sleep 3

kill $SERVER_PID 2>/dev/null
wait 2>/dev/null

echo ""
echo "=== Server Logs ==="
echo ""
echo "Messages processed:"
grep "Processed.*messages" /tmp/final_test.log || echo "  (none found)"
echo ""
echo "Handler calls:"
grep "on_conversation handler\|Broadcasting" /tmp/final_test.log | head -5 || echo "  (none found)"
echo ""
echo "Dispatch calls:"
grep "\[QwenClient::dispatch\]" /tmp/final_test.log | head -3 || echo "  (none found)"
