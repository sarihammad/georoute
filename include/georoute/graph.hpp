#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "georoute/types.hpp"

namespace georoute {

struct Edge {
    node_id to{0};
    float base_travel_time{0.0F};
    edge_id id{0};
};

class Graph {
public:
    explicit Graph(std::size_t node_count = 0);

    void add_edge(node_id from, node_id to, float base_travel_time);

    [[nodiscard]] const std::vector<Edge>& neighbors(node_id u) const noexcept;

    [[nodiscard]] std::size_t node_count() const noexcept;
    [[nodiscard]] std::size_t edge_count() const noexcept;

private:
    std::vector<std::vector<Edge>> adjacency_{};
    edge_id next_edge_id_{0};
};

}  // namespace georoute

