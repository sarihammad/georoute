#include "georoute/dijkstra.hpp"

#include <algorithm>
#include <limits>
#include <queue>
#include <stdexcept>
#include <utility>

namespace georoute {

DijkstraRouter::DijkstraRouter(const Graph& graph, const SegmentTree& congestion_tree)
    : graph_(graph), congestion_tree_(congestion_tree) {}

RouteResult DijkstraRouter::shortest_path(node_id source, node_id target) const {
    const auto node_count = graph_.node_count();
    if (source >= node_count || target >= node_count) {
        throw std::out_of_range{"DijkstraRouter::shortest_path node id out of range"};
    }

    if (source == target) {
        return RouteResult{{source}, 0.0F, true};
    }

    constexpr float inf = std::numeric_limits<float>::infinity();
    std::vector<float> distances(node_count, inf);
    std::vector<node_id> predecessors(node_count, std::numeric_limits<node_id>::max());

    distances[source] = 0.0F;

    struct QueueEntry {
        node_id node;
        float cost;
    };
    struct CompareEntry {
        bool operator()(const QueueEntry& lhs, const QueueEntry& rhs) const noexcept {
            return lhs.cost > rhs.cost;
        }
    };

    std::priority_queue<QueueEntry, std::vector<QueueEntry>, CompareEntry> queue;
    queue.push(QueueEntry{source, 0.0F});

    while (!queue.empty()) {
        const auto current = queue.top();
        queue.pop();

        if (current.cost > distances[current.node]) {
            continue;
        }

        if (current.node == target) {
            break;
        }

        for (const auto& edge : graph_.neighbors(current.node)) {
            const float congestion_factor = congestion_tree_.point_query(edge.id);
            const float edge_cost = edge.base_travel_time * congestion_factor;
            const float new_cost = current.cost + edge_cost;

            if (new_cost + std::numeric_limits<float>::epsilon() < distances[edge.to]) {
                distances[edge.to] = new_cost;
                predecessors[edge.to] = current.node;

                queue.push(QueueEntry{edge.to, new_cost});
            }
        }
    }

    if (distances[target] == inf) {
        return RouteResult{};
    }

    std::vector<node_id> path;
    for (node_id current = target; current != std::numeric_limits<node_id>::max(); current = predecessors[current]) {
        path.push_back(current);
        if (current == source) {
            break;
        }
    }

    if (path.empty() || path.back() != source) {
        return RouteResult{};
    }

    std::reverse(path.begin(), path.end());

    RouteResult result;
    result.nodes = std::move(path);
    result.total_travel_time = distances[target];
    result.reachable = true;
    return result;
}

}  // namespace georoute

