#!/bin/bash

echo "=== Final Clean Test - Qwen TCP Server ==="
echo ""

pkill -f "qwen.*server" 2>/dev/null
sleep 2

echo "Starting server..."
./script/run_qwen_server.sh > /tmp/clean_test.log 2>&1 &
SERVER_PID=$!
echo "Server PID: $SERVER_PID"
sleep 5

echo "Sending test message..."
echo '{"type":"user_input","content":"say hello world"}' | timeout 15 nc localhost 7774

echo ""
echo "Test complete!"
echo ""
echo "Cleaning up..."
sleep 2
kill $SERVER_PID 2>/dev/null
wait 2>/dev/null

echo ""
echo "=== Summary ==="
grep "closed write side" /tmp/clean_test.log >/dev/null && echo "✓ Client kept alive after EOF" || echo "✗ Client disconnected on EOF"
echo ""
echo "Server log: /tmp/clean_test.log"
