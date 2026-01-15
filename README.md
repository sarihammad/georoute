# GeoRoute

**Low-latency geo routing & real-time congestion updates in modern C++.**

GeoRoute is a demonstration of building low-latency geo systems in modern C++. Features real-time congestion updates, fast route queries, and measurable performance.

## 60-Second Demo

```bash
# Start the server
docker compose up --build

# In another terminal, query a route
curl "http://localhost:8080/route?src=0&dst=3"

# Apply a congestion update
curl -X POST http://localhost:8080/api/v1/congestion/update \
  -H "Content-Type: application/json" \
  -d '{"edge_start": 0, "edge_end": 1, "factor": 2.5}'

# Query again - observe path change
curl "http://localhost:8080/route?src=0&dst=3"

# Check metrics
curl http://localhost:8080/metrics
```

Or run the automated demo:

```bash
./demo/demo.sh
```

## Core Capabilities

- **Fast Route Queries**: Dijkstra-based shortest path with sub-millisecond p50 latency on 25K node grids (see benchmarks)
- **Incremental Congestion Updates**: Segment-tree-backed range updates without graph reconstruction
- **Thread-Safe Queries**: Concurrent reads with `std::shared_mutex` for high-throughput scenarios
- **Metrics & Observability**: Built-in latency tracking and query statistics

## Architecture

```
┌─────────────────────────────────────────┐
│         HTTP Server (cpp-httplib)      │
│  /route, /congestion/update, /metrics   │
└─────────────────┬───────────────────────┘
                  │
┌─────────────────▼───────────────────────┐
│      GeoRouteEngine (Orchestrator)      │
│  - Route computation with timing        │
│  - Stats aggregation                    │
└─────────────────┬───────────────────────┘
                  │
┌─────────────────▼───────────────────────┐
│           Router (Thread-Safe)          │
│  - Graph + SegmentTree coordination     │
└───────┬───────────────────────┬─────────┘
        │                       │
┌───────▼────────┐    ┌─────────▼──────────┐
│     Graph      │    │   SegmentTree      │
│  Adjacency List │    │  Lazy Propagation │
└────────────────┘    └────────────────────┘
        │                       │
┌───────▼───────────────────────▼─────────┐
│         DijkstraRouter                   │
│  Shortest path with congestion factors   │
└──────────────────────────────────────────┘
```

## Performance

Sample benchmark output (160×160 grid, 25,600 nodes, modern laptop):

```
ROUTE_BENCH
  queries=10000
  p50_us=245.3
  p95_us=512.7
  p99_us=892.1
  max_us=1234.5
  mean_us=267.8

UPDATE_BENCH
  queries=1000
  p50_us=12.3
  p95_us=18.7
  p99_us=24.2
  max_us=31.5
  mean_us=13.1
  throughput_updates_per_sec=76335.9
```

**Note**: Results vary by hardware and graph topology. Grid graphs represent a worst-case scenario for Dijkstra. Real-world road networks typically perform better due to hierarchical structure.

Run benchmarks:

```bash
./build/georoute_bench_main --mode=mixed --queries=10000 --updates=1000
```

See [docs/perf.md](docs/perf.md) for detailed methodology and hardware notes.

## API

### GET /health
Health check endpoint.

**Response:**
```json
{"status": "ok"}
```

### GET /route?src={source}&dst={target}
Compute shortest path between two nodes.

**Response:**
```json
{
  "src": 0,
  "dst": 3,
  "distance": 12.4,
  "eta_ms": 12400,
  "path": [0, 1, 3],
  "reachable": true,
  "stats": {
    "compute_us": 210.5,
    "expanded_nodes": 0
  }
}
```

### POST /api/v1/congestion/update
Apply congestion multiplier to a range of edges.

**Request:**
```json
{
  "edge_start": 0,
  "edge_end": 10,
  "factor": 1.5
}
```

**Response:**
```json
{"status": "ok"}
```

### GET /metrics
Get server statistics.

**Response:**
```json
{
  "queries_total": 1234,
  "updates_total": 56,
  "compute_time_total_us": 345678.9,
  "compute_time_max_us": 1234.5,
  "compute_time_avg_us": 280.1
}
```

Full API documentation: [docs/api.md](docs/api.md)

## Systems Story

GeoRoute demonstrates key systems engineering tradeoffs in low-latency geo routing:

- **Segment Tree for Range Updates**: O(log n) congestion updates vs O(n) graph rebuild
- **Dijkstra for Exact Paths**: Exact shortest path vs A* heuristics (simpler, deterministic)
- **Thread-Safe Reads**: Shared mutex allows concurrent queries during updates
- **In-Memory Graph**: Fast queries, bounded by RAM (intentional tradeoff)

What we intentionally did NOT build:
- Map matching from GPS traces
- Live GPS ingestion pipelines
- Distributed sharding across regions
- Multi-region replication
- Time-dependent travel times

See [docs/systems-story.md](docs/systems-story.md) for detailed design decisions.

## Project Structure

```
georoute/
├── include/georoute/      # Public API headers
│   ├── engine.hpp         # GeoRouteEngine (main API)
│   ├── router.hpp          # Router (thread-safe wrapper)
│   ├── graph.hpp           # Graph data structure
│   ├── dijkstra.hpp        # Dijkstra algorithm
│   └── segment_tree.hpp    # Segment tree for congestion
├── src/                    # Implementation
├── apps/                   # Application entry points
│   └── server/             # HTTP server app
├── tests/                  # Unit tests (Catch2)
├── benchmarks/             # Performance benchmarks
├── demo/                   # Demo scripts
├── docs/                   # Documentation
│   ├── api.md              # API reference
│   ├── systems-story.md   # Design decisions
│   └── perf.md             # Performance methodology
└── docker/                 # Docker files
```

## Build & Run

### Prerequisites
- C++20 compiler (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.23+
- Docker (optional, for containerized runs)

### Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

**Note**: GeoRoute builds offline by default using vendored header-only dependencies. See [docs/build.md](docs/build.md) for dependency options.

### Run Server

```bash
./build/georoute_server --graph ../data/sample_graph.json
```

Or with Docker:

```bash
docker compose up --build
```

### Run Tests

```bash
cd build
ctest
```

### Run Benchmarks

```bash
./build/georoute_bench_main --mode=mixed --queries=10000
```

## Roadmap

- [ ] Add expanded node tracking in Dijkstra for better stats
- [ ] Support query parameters for alternative routes (k-shortest paths)
- [ ] Add graph validation and cycle detection
- [ ] Implement graph serialization/deserialization optimizations
- [ ] Add request tracing and structured logging
- [ ] Support weighted multi-criteria routing (time + cost)
- [ ] Add integration tests with real-world graph datasets
