#!/bin/bash
set -e

# GeoRoute Local Run Script
# Builds and runs the server locally (without Docker)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
GRAPH_FILE="${1:-${PROJECT_ROOT}/data/sample_graph.json}"

echo "GeoRoute Local Runner"
echo "===================="
echo ""

# Build if needed
if [ ! -f "${BUILD_DIR}/georoute_server" ]; then
    echo "Building GeoRoute..."
    mkdir -p "${BUILD_DIR}"
    cd "${BUILD_DIR}"
    cmake ..
    cmake --build .
    echo ""
fi

# Check if graph file exists
if [ ! -f "${GRAPH_FILE}" ]; then
    echo "Error: Graph file not found: ${GRAPH_FILE}"
    echo "Usage: $0 [graph_file.json]"
    exit 1
fi

echo "Starting server with graph: ${GRAPH_FILE}"
echo "Server will be available at http://localhost:8080"
echo "Press Ctrl+C to stop"
echo ""

cd "${BUILD_DIR}"
./georoute_server --graph "${GRAPH_FILE}"

