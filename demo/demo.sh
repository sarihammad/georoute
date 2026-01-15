#!/bin/bash
set -e

# GeoRoute Demo Script
# Shows baseline route query, congestion updates, and route changes

BASE_URL="${BASE_URL:-http://localhost:8080}"
GRAPH_FILE="${GRAPH_FILE:-../data/sample_graph.json}"

echo "=========================================="
echo "GeoRoute Demo: Congestion Updates"
echo "=========================================="
echo ""

# Check if server is running
if ! curl -s "${BASE_URL}/health" > /dev/null 2>&1; then
    echo "Error: Server is not running at ${BASE_URL}"
    echo "Please start the server first:"
    echo "  ./build/georoute_server --graph ${GRAPH_FILE}"
    echo "  or"
    echo "  docker compose up"
    exit 1
fi

echo "✓ Server is running"
echo ""

# Baseline route query
echo "1. Baseline Route Query (0 -> 3)"
echo "-----------------------------------"
BASELINE_RESPONSE=$(curl -s "${BASE_URL}/route?src=0&dst=3")
echo "$BASELINE_RESPONSE" | python3 -m json.tool 2>/dev/null || echo "$BASELINE_RESPONSE"
BASELINE_TIME=$(echo "$BASELINE_RESPONSE" | python3 -c "import sys, json; print(json.load(sys.stdin).get('distance', 0))" 2>/dev/null || echo "0")
BASELINE_PATH=$(echo "$BASELINE_RESPONSE" | python3 -c "import sys, json; print(' -> '.join(map(str, json.load(sys.stdin).get('path', []))))" 2>/dev/null || echo "")
echo "Path: $BASELINE_PATH"
echo "Travel time: ${BASELINE_TIME}s"
echo ""

# Apply congestion update
echo "2. Applying Congestion Update"
echo "-----------------------------------"
UPDATE_RESPONSE=$(curl -s -X POST "${BASE_URL}/api/v1/congestion/update" \
    -H "Content-Type: application/json" \
    -d '{"edge_start": 0, "edge_end": 1, "factor": 2.5}')
echo "$UPDATE_RESPONSE" | python3 -m json.tool 2>/dev/null || echo "$UPDATE_RESPONSE"
echo "Applied congestion factor 2.5 to edges [0, 1]"
echo ""

# Query again
echo "3. Route Query After Congestion (0 -> 3)"
echo "-----------------------------------"
CONGESTED_RESPONSE=$(curl -s "${BASE_URL}/route?src=0&dst=3")
echo "$CONGESTED_RESPONSE" | python3 -m json.tool 2>/dev/null || echo "$CONGESTED_RESPONSE"
CONGESTED_TIME=$(echo "$CONGESTED_RESPONSE" | python3 -c "import sys, json; print(json.load(sys.stdin).get('distance', 0))" 2>/dev/null || echo "0")
CONGESTED_PATH=$(echo "$CONGESTED_RESPONSE" | python3 -c "import sys, json; print(' -> '.join(map(str, json.load(sys.stdin).get('path', []))))" 2>/dev/null || echo "")
echo "Path: $CONGESTED_PATH"
echo "Travel time: ${CONGESTED_TIME}s"
echo ""

# Show comparison
echo "4. Comparison"
echo "-----------------------------------"
if [ -n "$BASELINE_TIME" ] && [ -n "$CONGESTED_TIME" ]; then
    DELTA=$(python3 -c "print(${CONGESTED_TIME} - ${BASELINE_TIME})" 2>/dev/null || echo "N/A")
    echo "Baseline time: ${BASELINE_TIME}s"
    echo "Congested time: ${CONGESTED_TIME}s"
    echo "Delta: ${DELTA}s"
    if [ "$BASELINE_PATH" != "$CONGESTED_PATH" ]; then
        echo "✓ Path changed due to congestion!"
    else
        echo "Path unchanged (still optimal)"
    fi
fi
echo ""

# Show metrics
echo "5. Server Metrics"
echo "-----------------------------------"
METRICS=$(curl -s "${BASE_URL}/metrics")
echo "$METRICS" | python3 -m json.tool 2>/dev/null || echo "$METRICS"
echo ""

echo "=========================================="
echo "Demo complete!"
echo "=========================================="

