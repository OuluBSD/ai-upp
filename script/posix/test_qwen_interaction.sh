#!/bin/bash
# Test script for Qwen server and nc client interaction

set -e

echo "=== Qwen Server/Client Interaction Test ==="
echo ""

# Cleanup function
cleanup() {
    echo ""
    echo "Cleaning up..."
    if [ ! -z "$SERVER_PID" ]; then
        echo "Stopping server (PID: $SERVER_PID)..."
        kill $SERVER_PID 2>/dev/null || true
        wait $SERVER_PID 2>/dev/null || true
    fi
    rm -f /tmp/qwen_test_output.txt
    echo "Cleanup complete"
}

trap cleanup EXIT INT TERM

# Start the server in background
echo "Starting Qwen server..."
./script/run_qwen_server.sh > /tmp/qwen_server_log.txt 2>&1 &
SERVER_PID=$!
echo "Server started with PID: $SERVER_PID"

# Wait for server to be ready
echo "Waiting for server to be ready..."
for i in {1..30}; do
    if nc -z localhost 7774 2>/dev/null; then
        echo "Server is ready!"
        break
    fi
    if [ $i -eq 30 ]; then
        echo "ERROR: Server failed to start within 30 seconds"
        cat /tmp/qwen_server_log.txt
        exit 1
    fi
    sleep 1
    echo -n "."
done
echo ""

# Give it a moment to fully initialize
sleep 2

# Test 1: Send message and receive response
echo ""
echo "Test 1: Sending test message to server..."
# Use our client script that keeps connection open for async response
./script/test_qwen_client.sh localhost 7774 "Say hello" > /tmp/qwen_test_output.txt 2>&1 &
CLIENT_PID=$!

# Wait for client to complete (give it up to 25 seconds)
echo "Waiting for response (max 25 seconds, AI may take time to respond)..."
for i in {1..25}; do
    if ! kill -0 $CLIENT_PID 2>/dev/null; then
        echo "Client finished!"
        break
    fi
    sleep 1
    echo -n "."
done
echo ""

# If client is still running, kill it
if kill -0 $CLIENT_PID 2>/dev/null; then
    echo "Timeout reached, stopping client..."
    kill $CLIENT_PID 2>/dev/null || true
    wait $CLIENT_PID 2>/dev/null || true
fi

# Check if we got a response
if [ -f /tmp/qwen_test_output.txt ] && [ -s /tmp/qwen_test_output.txt ]; then
    echo ""
    echo "=== Response received ==="
    cat /tmp/qwen_test_output.txt
    echo ""
    echo "=== End of response ==="

    # Check if response contains expected fields
    if grep -q "type" /tmp/qwen_test_output.txt; then
        echo ""
        echo "✓ Test PASSED: Received valid JSON response"
    else
        echo ""
        echo "✗ Test FAILED: Response doesn't look like valid JSON"
        exit 1
    fi
else
    echo ""
    echo "✗ Test FAILED: No response received"
    echo ""
    echo "Server log:"
    cat /tmp/qwen_server_log.txt
    exit 1
fi

echo ""
echo "=== All tests passed! ==="
echo ""
echo "Server log excerpt:"
tail -20 /tmp/qwen_server_log.txt

exit 0
