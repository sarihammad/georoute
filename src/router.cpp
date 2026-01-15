#include "georoute/router.hpp"

#include <shared_mutex>
#include <stdexcept>
#include <utility>

#include <nlohmann/json.hpp>

namespace georoute {

Router::Router(Graph graph, SegmentTree segment_tree)
    : graph_(std::move(graph)), congestion_tree_(std::move(segment_tree)) {}

Router::~Router() = default;

void Router::apply_congestion_update(std::size_t edge_start, std::size_t edge_end, float factor) {
    std::unique_lock lock{mutex_};
    if (edge_start > edge_end) {
        throw std::invalid_argument{"Router::apply_congestion_update invalid range"};
    }
    if (edge_end >= congestion_tree_.size()) {
        throw std::out_of_range{"Router::apply_congestion_update range exceeds edge count"};
    }
    congestion_tree_.range_multiply(edge_start, edge_end, factor);
}

RouteComputation Router::compute_route(node_id source, node_id target) const {
    std::shared_lock lock{mutex_};
    DijkstraRouter router{graph_, congestion_tree_};
    return router.shortest_path(source, target);
}

Router Router::from_json(const nlohmann::json& config) {
    if (!config.contains("nodes")) {
        throw std::invalid_argument{"Router::from_json missing 'nodes' field"};
    }
    if (!config.contains("edges") || !config["edges"].is_array()) {
        throw std::invalid_argument{"Router::from_json missing 'edges' array"};
    }

    const auto node_count = config.at("nodes").get<std::size_t>();
    Graph graph{node_count};

    const auto& edges = config.at("edges");
    for (const auto& edge : edges) {
        if (!edge.contains("from") || !edge.contains("to") || !edge.contains("base_travel_time")) {
            throw std::invalid_argument{"Router::from_json edge missing required fields"};
        }
        const auto from = edge.at("from").get<node_id>();
        const auto to = edge.at("to").get<node_id>();
        const auto base_time = edge.at("base_travel_time").get<float>();
        graph.add_edge(from, to, base_time);
    }

    SegmentTree tree{graph.edge_count()};
    return Router{std::move(graph), std::move(tree)};
}

}  // namespace georoute

