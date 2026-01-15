#pragma once

#include <optional>
#include <vector>

#include "georoute/graph.hpp"
#include "georoute/segment_tree.hpp"
#include "georoute/types.hpp"

namespace georoute {

class DijkstraRouter {
public:
    DijkstraRouter(const Graph& graph, const SegmentTree& congestion_tree);

    [[nodiscard]] RouteComputation shortest_path(node_id source, node_id target) const;

private:
    const Graph& graph_;
    const SegmentTree& congestion_tree_;
};

} 

