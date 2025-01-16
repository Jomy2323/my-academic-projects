#!/bin/bash

# Paths
SERVER_EXEC=../server/kvs
JOB_DIR=../server/jobs
CLIENT_EXEC=./client
FIFO_PATH=/tmp/reg
CLIENT_TESTS_DIR=./client_tests

# Clean up any existing FIFOs
rm -f $FIFO_PATH

# Start the server
gnome-terminal -- bash -c "echo 'Starting server...'; $SERVER_EXEC $JOB_DIR 4 2 $FIFO_PATH; exec bash"

# Wait for the server to start
sleep 2

# Launch clients
for i in {1..6}; do
    CLIENT_ID=$i
    CLIENT_TEST_FILE="$CLIENT_TESTS_DIR/client${CLIENT_ID}.txt"
    if [[ -f $CLIENT_TEST_FILE ]]; then
        gnome-terminal -- bash -c "echo 'Starting client $CLIENT_ID...'; $CLIENT_EXEC $CLIENT_ID $FIFO_PATH < $CLIENT_TEST_FILE; exec bash"
    else
        echo "Client test file not found: $CLIENT_TEST_FILE"
    fi
done
