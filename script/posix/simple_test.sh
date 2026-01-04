#!/bin/bash

pkill -f "qwen.*server" 2>/dev/null
sleep 2

./script/run_qwen_server.sh > /tmp/simple_test.log 2>&1 &
SERVER_PID=$!
echo "Server PID: $SERVER_PID"
sleep 4

echo "Sending message..."
echo '{"type":"user_input","content":"test"}' | nc localhost 7774

sleep 5

kill $SERVER_PID 2>/dev/null
wait 2>/dev/null

echo ""
echo "=== Checking for NEW BUILD marker ==="
grep "NEW BUILD" /tmp/simple_test.log

echo ""
echo "=== Checking for poll_messages logs ==="
grep "poll_messages" /tmp/simple_test.log
