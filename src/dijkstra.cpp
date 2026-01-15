#include "georoute/dijkstra.hpp"

#include <algorithm>
#include <limits>
#include <queue>
#include <stdexcept>
#include <utility>

namespace georoute {

DijkstraRouter::DijkstraRouter(const Graph& graph, const SegmentTree& congestion_tree)
    : graph_(graph), congestion_tree_(congestion_tree) {}

RouteComputation DijkstraRouter::shortest_path(node_id source, node_id target) const {
    const auto node_count = graph_.node_count();
    if (source >= node_count || target >= node_count) {
        throw std::out_of_range{"DijkstraRouter::shortest_path node id out of range"};
    }

    RouteStats stats{};
    RouteResult result{};

    if (source == target) {
        result.nodes = {source};
        result.total_travel_time = 0.0F;
        result.reachable = true;
        stats.expanded_nodes = 1;
        stats.visited_nodes = 1;
        return RouteComputation{result, stats};
    }

    constexpr double inf = std::numeric_limits<double>::infinity();
    std::vector<double> distances(node_count, inf);
    std::vector<node_id> predecessors(node_count, std::numeric_limits<node_id>::max());
    std::vector<bool> visited(node_count, false);

    distances[source] = 0.0;

    struct QueueEntry {
        node_id node;
        double cost;
    };
    struct CompareEntry {
        bool operator()(const QueueEntry& lhs, const QueueEntry& rhs) const noexcept {
            return lhs.cost > rhs.cost;
        }
    };

    std::priority_queue<QueueEntry, std::vector<QueueEntry>, CompareEntry> queue;
    queue.push(QueueEntry{source, 0.0});

    while (!queue.empty()) {
        const auto current = queue.top();
        queue.pop();

        // Skip stale entries
        if (current.cost > distances[current.node]) {
            continue;
        }

        // Count expanded nodes (non-stale queue pops)
        stats.expanded_nodes++;

        // Mark as visited
        if (!visited[current.node]) {
            visited[current.node] = true;
            stats.visited_nodes++;
        }

        if (current.node == target) {
            break;
        }

        for (const auto& edge : graph_.neighbors(current.node)) {
            const float congestion_factor = congestion_tree_.point_query(edge.id);
            const double edge_cost = static_cast<double>(edge.base_travel_time) * static_cast<double>(congestion_factor);
            const double new_cost = current.cost + edge_cost;

            if (new_cost < distances[edge.to]) {
                distances[edge.to] = new_cost;
                predecessors[edge.to] = current.node;
                stats.relaxed_edges++;

                queue.push(QueueEntry{edge.to, new_cost});
            }
        }
    }

    if (distances[target] == inf) {
        return RouteComputation{result, stats};
    }

    std::vector<node_id> path;
    for (node_id current = target; current != std::numeric_limits<node_id>::max(); current = predecessors[current]) {
        path.push_back(current);
        if (current == source) {
            break;
        }
    }

    if (path.empty() || path.back() != source) {
        return RouteComputation{result, stats};
    }

    std::reverse(path.begin(), path.end());

    result.nodes = std::move(path);
    result.total_travel_time = static_cast<float>(distances[target]);
    result.reachable = true;
    return RouteComputation{result, stats};
}

}  // namespace georoute

