#!/bin/bash
# Simple client script to test Qwen TCP server
# Keeps connection open to receive async responses

HOST="${1:-localhost}"
PORT="${2:-7774}"
MESSAGE="${3:-Say hello}"

echo "Connecting to $HOST:$PORT..."
echo "Sending: $MESSAGE"
echo ""

# Use a named pipe to keep connection open
PIPE="/tmp/qwen_client_pipe_$$"
mkfifo "$PIPE" 2>/dev/null || { echo "Failed to create pipe"; exit 1; }

# Cleanup on exit
cleanup() {
    rm -f "$PIPE"
    # Kill background jobs
    jobs -p | xargs -r kill 2>/dev/null
}
trap cleanup EXIT INT TERM

# Create output file
OUTPUT="/tmp/qwen_response_$$"

# Start nc in background, reading from the named pipe and writing to output
nc "$HOST" "$PORT" < "$PIPE" > "$OUTPUT" 2>&1 &
NC_PID=$!

# Give nc a moment to connect
sleep 0.5

# Open pipe for writing (keeps it open)
exec 3>"$PIPE"

# Send the message
echo "{\"type\":\"user_input\",\"content\":\"$MESSAGE\"}" >&3

# Keep pipe open and wait for response (timeout after 15 seconds)
echo "Waiting for response (timeout 15s)..."
for i in {1..15}; do
    sleep 1
    # Check if we got any response
    if [ -s "$OUTPUT" ]; then
        echo "Response received!"
        break
    fi
done

# Close the pipe
exec 3>&-

# Wait a bit more for any trailing data
sleep 1

# Wait for nc to finish
wait $NC_PID 2>/dev/null || true

# Show the response
if [ -s "$OUTPUT" ]; then
    echo "=== Server Response ==="
    cat "$OUTPUT"
    echo "=== End Response ==="
else
    echo "No response received"
fi

# Cleanup output file
rm -f "$OUTPUT"

echo ""
echo "Done"
