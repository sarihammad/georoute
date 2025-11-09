#include "georoute/graph.hpp"

#include <stdexcept>

namespace georoute {

Graph::Graph(std::size_t node_count)
    : adjacency_(node_count), next_edge_id_(0) {}

void Graph::add_edge(node_id from, node_id to, float base_travel_time) {
    if (from >= adjacency_.size() || to >= adjacency_.size()) {
        throw std::out_of_range{"Graph::add_edge node id out of range"};
    }

    adjacency_[from].push_back(Edge{to, base_travel_time, next_edge_id_});
    ++next_edge_id_;
}

const std::vector<Edge>& Graph::neighbors(node_id u) const noexcept {
    static const std::vector<Edge> empty{};
    if (u >= adjacency_.size()) {
        return empty;
    }
    return adjacency_[u];
}

std::size_t Graph::node_count() const noexcept {
    return adjacency_.size();
}

std::size_t Graph::edge_count() const noexcept {
    return next_edge_id_;
}

}  // namespace georoute

