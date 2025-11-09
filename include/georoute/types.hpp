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

}  // namespace georoute

