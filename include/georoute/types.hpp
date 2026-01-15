#pragma once

#include <cstdint>
#include <vector>

namespace georoute {

using node_id = std::uint32_t;
using edge_id = std::uint32_t;

struct RouteResult {
    std::vector<node_id> nodes{};
    float total_travel_time{0.0F};
    bool reachable{false};
};

struct RouteStats {
    std::uint32_t expanded_nodes{0};
    std::uint32_t relaxed_edges{0};
    std::uint32_t visited_nodes{0};
};

struct RouteComputation {
    RouteResult result;
    RouteStats stats;
};

}  // namespace georoute

