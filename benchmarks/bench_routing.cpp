#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "georoute/graph.hpp"
#include "georoute/router.hpp"
#include "georoute/segment_tree.hpp"

namespace {

struct BenchmarkContext {
    georoute::Router router;
    std::size_t node_count;
    std::size_t edge_count;
};

BenchmarkContext build_grid_router(std::size_t rows, std::size_t cols) {
    georoute::Graph graph{rows * cols};

    const auto index = [cols](std::size_t r, std::size_t c) {
        return static_cast<georoute::node_id>(r * cols + c);
    };

    for (std::size_t r = 0; r < rows; ++r) {
        for (std::size_t c = 0; c < cols; ++c) {
            const auto current = index(r, c);
            if (c + 1 < cols) {
                const auto right = index(r, c + 1);
                const float base = 1.0F + static_cast<float>((r + c) % 7) * 0.1F;
                graph.add_edge(current, right, base);
                graph.add_edge(right, current, base);
            }
            if (r + 1 < rows) {
                const auto down = index(r + 1, c);
                const float base = 1.0F + static_cast<float>((r + c) % 5) * 0.15F;
                graph.add_edge(current, down, base);
                graph.add_edge(down, current, base);
            }
        }
    }

    const auto edge_count = graph.edge_count();
    georoute::SegmentTree tree{edge_count};

    return BenchmarkContext{georoute::Router{std::move(graph), std::move(tree)}, rows * cols, edge_count};
}

struct Statistics {
    double total_microseconds{0.0};
    double max_microseconds{0.0};
    std::size_t count{0};

    void add(double microseconds) {
        total_microseconds += microseconds;
        max_microseconds = std::max(max_microseconds, microseconds);
        ++count;
    }

    [[nodiscard]] double average() const {
        if (count == 0) {
            return 0.0;
        }
        return total_microseconds / static_cast<double>(count);
    }
};

}  // namespace

int main() {
    constexpr std::size_t rows = 160;
    constexpr std::size_t cols = 160;
    constexpr std::size_t total_queries = 200;
    constexpr std::size_t update_interval = 10;

    auto context = build_grid_router(rows, cols);

    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<std::size_t> node_dist(0, context.node_count - 1);
    std::uniform_int_distribution<std::size_t> edge_dist(0, context.edge_count > 0 ? context.edge_count - 1 : 0);
    const std::size_t max_span = context.edge_count > 0
                                     ? std::min<std::size_t>(750, context.edge_count - 1)
                                     : 0;
    std::uniform_real_distribution<float> factor_dist(0.8F, 1.3F);

    Statistics route_stats;
    Statistics update_stats;
    std::size_t unreachable_count = 0;

    for (std::size_t i = 0; i < total_queries; ++i) {
        if (i % update_interval == 0 && context.edge_count > 0) {
            const auto start_idx = edge_dist(rng);
            std::uniform_int_distribution<std::size_t> span_dist(0, max_span);
            const auto span = std::min(span_dist(rng), context.edge_count - start_idx - 1);
            const auto end_idx = start_idx + span;
            const auto factor = factor_dist(rng);

            const auto update_begin = std::chrono::high_resolution_clock::now();
            context.router.apply_congestion_update(start_idx, end_idx, factor);
            const auto update_end = std::chrono::high_resolution_clock::now();

            const std::chrono::duration<double, std::micro> update_duration = update_end - update_begin;
            update_stats.add(update_duration.count());
        }

        georoute::node_id source = static_cast<georoute::node_id>(node_dist(rng));
        georoute::node_id target = static_cast<georoute::node_id>(node_dist(rng));
        if (source == target) {
            target = static_cast<georoute::node_id>((target + 1) % context.node_count);
        }

        const auto route_begin = std::chrono::high_resolution_clock::now();
        const auto result = context.router.compute_route(source, target);
        const auto route_end = std::chrono::high_resolution_clock::now();

        const std::chrono::duration<double, std::micro> route_duration = route_end - route_begin;
        route_stats.add(route_duration.count());

        if (!result.reachable) {
            ++unreachable_count;
        }
    }

    std::cout << "GeoRoute Routing Benchmark\n";
    std::cout << "Grid size: " << rows << " x " << cols << " (" << context.node_count << " nodes, "
              << context.edge_count << " directed edges)\n";
    std::cout << "Total queries: " << route_stats.count
              << ", average route time: " << route_stats.average() << " us, "
              << "max route time: " << route_stats.max_microseconds << " us\n";
    if (update_stats.count > 0) {
        std::cout << "Congestion updates: " << update_stats.count
                  << ", average update time: " << update_stats.average() << " us\n";
    }
    std::cout << "Unreachable routes: " << unreachable_count << '\n';

    return 0;
}


