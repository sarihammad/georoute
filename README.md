# GeoRoute

GeoRoute is a real-time routing engine written in modern C++20. It combines graph algorithms and range-based congestion models to deliver low-latency routes for large transportation hubs such as airports.

## Architecture Overview

GeoRoute layers a pure C++ routing core with multiple entry points:

- **Graph**: adjacency-list representation of up to 100K+ nodes and edges.
- **Segment Tree**: lazy-propagated congestion multipliers for dynamic edge updates.
- **Dijkstra Router**: shortest-path routing that blends base travel times with congestion factors.
- **Router Service**: thread-safe façade over graph + congestion state.
- **Interfaces**: CLI demo, REST server (cpp-httplib), and AWS Lambda-style entrypoint.

```
Graph + SegmentTree + Dijkstra -> Router -> (CLI | HTTP API | Lambda)
```

## Key Features

- High-performance Dijkstra search tuned for 100K+ node networks.
- Segment tree supporting range congestion updates without rebuilding the graph.
- Thread-safe queries with `std::shared_mutex` allowing concurrent reads.
- REST API built with cpp-httplib and JSON via nlohmann::json.
- CLI tool for scripted experiments and a synthetic benchmark harness.
- Docker files for local development container and Lambda-style packaging.

## Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Running Unit Tests

```bash
ctest
```

## Running the CLI Demo

```bash
./build/georoute_cli --graph data/sample_graph.json --route 0 3
./build/georoute_cli --graph data/sample_graph.json --congestion 0 1 1.5 --route 0 3
```

## Running the HTTP Server

```bash
./build/georoute_server --graph data/sample_graph.json
```

Example requests:

```bash
curl http://localhost:8080/api/v1/health
curl -X POST http://localhost:8080/api/v1/route \
     -H "Content-Type: application/json" \
     -d '{"source":0,"target":3}'

curl -X POST http://localhost:8080/api/v1/congestion/update \
     -H "Content-Type: application/json" \
     -d '{"edge_start":0,"edge_end":10,"factor":1.2}'
```

## Benchmarks

```bash
./build/georoute_bench
```

Outputs average and max route latency across randomized congestion scenarios on a synthetic grid graph.

## Docker

- `docker/Dockerfile.dev` builds the CLI/server/benchmark inside Ubuntu 22.04 and runs the HTTP server on port 8080.
- `docker/Dockerfile.lambda` sketches how to package GeoRoute for AWS Lambda with a placeholder bootstrap entry point.

## Future Work

- Time-dependent travel times and historical data ingestion.
- Multi-criteria routing (time + cost + carbon).
- Real integration with cloud storage and IAM-secured configuration loading.
- C++ modules for fine-grained build acceleration once compiler support stabilizes.

