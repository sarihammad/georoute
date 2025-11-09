#pragma once

#include <shared_mutex>

#include <nlohmann/json_fwd.hpp>

#include "georoute/dijkstra.hpp"
#include "georoute/graph.hpp"
#include "georoute/segment_tree.hpp"
#include "georoute/types.hpp"

namespace georoute {

class Router {
public:
    Router(Graph graph, SegmentTree segment_tree);
    Router(const Router&) = delete;
    Router& operator=(const Router&) = delete;
    Router(Router&&) = delete;
    Router& operator=(Router&&) = delete;
    ~Router();

    void apply_congestion_update(std::size_t edge_start, std::size_t edge_end, float factor);
    [[nodiscard]] RouteResult compute_route(node_id source, node_id target) const;

    static Router from_json(const nlohmann::json& config);

private:
    Graph graph_;
    SegmentTree congestion_tree_;
    mutable std::shared_mutex mutex_;
};

}  // namespace georoute

