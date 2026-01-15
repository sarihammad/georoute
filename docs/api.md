# GeoRoute API Reference

## Base URL

By default, the server runs on `http://localhost:8080`.

## Endpoints

### Health Check

#### GET /health

Check if the server is running.

**Response:**
```json
{
  "status": "ok"
}
```

**Status Codes:**
- `200 OK`: Server is healthy

---

### Route Query

#### GET /route?src={source}&dst={target}

Compute the shortest path between two nodes.

**Query Parameters:**
- `src` (required): Source node ID (non-negative integer)
- `dst` (required): Target node ID (non-negative integer)

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

**Fields:**
- `src`: Source node ID (echoed from request)
- `dst`: Target node ID (echoed from request)
- `distance`: Total travel time in seconds (float)
- `eta_ms`: Estimated time of arrival in milliseconds (integer)
- `path`: Array of node IDs representing the route
- `reachable`: Boolean indicating if a path exists
- `stats.compute_us`: Route computation time in microseconds
- `stats.expanded_nodes`: Number of nodes expanded during Dijkstra search (non-zero for non-trivial routes)

**Status Codes:**
- `200 OK`: Request successful
- `400 Bad Request`: Missing or invalid parameters

**Example:**
```bash
curl "http://localhost:8080/route?src=0&dst=3"
```

#### POST /api/v1/route

Alternative route endpoint using JSON body (legacy compatibility).

**Request Body:**
```json
{
  "source": 0,
  "target": 3
}
```

**Response:** Same as GET /route

---

### Congestion Updates

#### POST /api/v1/congestion/update

Apply a congestion multiplier to a range of edges.

**Request Body:**
```json
{
  "edge_start": 0,
  "edge_end": 10,
  "factor": 1.5
}
```

**Fields:**
- `edge_start` (required): Starting edge ID (inclusive, 0-based)
- `edge_end` (required): Ending edge ID (inclusive, 0-based)
- `factor` (required): Multiplier to apply (float, typically > 0)

**Response:**
```json
{
  "status": "ok"
}
```

**Status Codes:**
- `200 OK`: Update applied successfully
- `400 Bad Request`: Invalid JSON or missing fields
- `500 Internal Server Error`: Update failed (e.g., invalid edge range)

**Example:**
```bash
curl -X POST http://localhost:8080/api/v1/congestion/update \
  -H "Content-Type: application/json" \
  -d '{"edge_start": 0, "edge_end": 5, "factor": 2.0}'
```

**Notes:**
- Congestion factors are multiplicative (1.0 = no change, 2.0 = double travel time)
- Updates are applied to the segment tree in O(log n) time
- Edge IDs are assigned sequentially during graph construction (0, 1, 2, ...)

---

### Metrics

#### GET /metrics

Get server statistics and performance metrics.

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

**Fields:**
- `queries_total`: Total number of route queries processed
- `updates_total`: Total number of congestion updates applied
- `compute_time_total_us`: Cumulative route computation time in microseconds
- `compute_time_max_us`: Maximum single-query computation time in microseconds
- `compute_time_avg_us`: Average route computation time in microseconds

**Status Codes:**
- `200 OK`: Metrics retrieved successfully

**Example:**
```bash
curl http://localhost:8080/metrics
```

---

## Error Responses

All error responses follow this format:

```json
{
  "error": "Error message describing what went wrong"
}
```

Common error scenarios:
- Invalid node IDs (out of range)
- Missing required parameters
- Invalid JSON in request body
- Edge range out of bounds for congestion updates
- Unreachable routes (no path exists)

---

## Graph Format

Graphs are loaded from JSON files with the following structure:

```json
{
  "nodes": 4,
  "edges": [
    {
      "from": 0,
      "to": 1,
      "base_travel_time": 10.0
    },
    {
      "from": 1,
      "to": 2,
      "base_travel_time": 5.0
    }
  ]
}
```

**Fields:**
- `nodes`: Total number of nodes in the graph (integer)
- `edges`: Array of edge objects
  - `from`: Source node ID (0-based)
  - `to`: Target node ID (0-based)
  - `base_travel_time`: Base travel time in seconds (float)

Edge IDs are assigned automatically in the order edges appear in the array (0, 1, 2, ...).

