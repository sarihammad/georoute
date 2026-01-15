#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <random>
#include <string>
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

double percentile(const std::vector<double>& sorted, double p) {
    if (sorted.empty()) {
        return 0.0;
    }
    const auto idx = static_cast<std::size_t>(std::ceil(p * sorted.size())) - 1;
    return sorted[std::min(idx, sorted.size() - 1)];
}

struct PercentileStats {
    double p50{0.0};
    double p95{0.0};
    double p99{0.0};
    double max{0.0};
    double min{0.0};
    double mean{0.0};
    std::size_t count{0};

    static PercentileStats compute(std::vector<double> values) {
        PercentileStats stats;
        if (values.empty()) {
            return stats;
        }
        
        std::sort(values.begin(), values.end());
        stats.count = values.size();
        stats.min = values.front();
        stats.max = values.back();
        stats.p50 = percentile(values, 0.50);
        stats.p95 = percentile(values, 0.95);
        stats.p99 = percentile(values, 0.99);
        
        double sum = 0.0;
        for (const auto v : values) {
            sum += v;
        }
        stats.mean = sum / static_cast<double>(stats.count);
        
        return stats;
    }
};

void print_percentile_stats(const std::string& label, const PercentileStats& stats) {
    std::cout << label << "\n";
    std::cout << "  queries=" << stats.count << "\n";
    std::cout << "  p50_us=" << stats.p50 << "\n";
    std::cout << "  p95_us=" << stats.p95 << "\n";
    std::cout << "  p99_us=" << stats.p99 << "\n";
    std::cout << "  max_us=" << stats.max << "\n";
    std::cout << "  min_us=" << stats.min << "\n";
    std::cout << "  mean_us=" << stats.mean << "\n";
}

}  // namespace

int main(int argc, char** argv) {
    std::string mode = "mixed";
    std::size_t queries = 10000;
    std::size_t updates = 1000;
    std::size_t seed = 0;
    std::size_t grid_size = 160;

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        const std::string arg{argv[i]};
        if (arg == "--mode" && i + 1 < argc) {
            mode = argv[++i];
        } else if (arg == "--queries" && i + 1 < argc) {
            queries = static_cast<std::size_t>(std::stoul(argv[++i]));
        } else if (arg == "--updates" && i + 1 < argc) {
            updates = static_cast<std::size_t>(std::stoul(argv[++i]));
        } else if (arg == "--seed" && i + 1 < argc) {
            seed = static_cast<std::size_t>(std::stoul(argv[++i]));
        } else if (arg == "--grid-size" && i + 1 < argc) {
            grid_size = static_cast<std::size_t>(std::stoul(argv[++i]));
        }
    }

    std::mt19937 rng{seed == 0 ? std::random_device{}() : static_cast<std::mt19937::result_type>(seed)};

    std::cout << "GeoRoute Benchmark\n";
    std::cout << "==================\n";
    std::cout << "Mode: " << mode << "\n";
    std::cout << "Grid size: " << grid_size << "x" << grid_size << "\n";
    std::cout << "Queries: " << queries << "\n";
    std::cout << "Updates: " << updates << "\n";
    std::cout << "Seed: " << (seed == 0 ? "random" : std::to_string(seed)) << "\n";
    std::cout << "\n";

    auto context = build_grid_router(grid_size, grid_size);
    std::cout << "Graph: " << context.node_count << " nodes, " << context.edge_count << " edges\n\n";

    std::uniform_int_distribution<std::size_t> node_dist(0, context.node_count - 1);
    std::uniform_int_distribution<std::size_t> edge_dist(0, context.edge_count > 0 ? context.edge_count - 1 : 0);
    const std::size_t max_span = context.edge_count > 0 ? std::min<std::size_t>(750, context.edge_count - 1) : 0;
    std::uniform_real_distribution<float> factor_dist(0.8F, 1.3F);

    std::vector<double> route_times;
    std::vector<double> update_times;
    std::size_t unreachable_count = 0;

    const auto update_interval = (mode == "mixed" && queries > 0) ? queries / updates : 0;

    for (std::size_t i = 0; i < queries; ++i) {
        if (mode == "mixed" && update_interval > 0 && i % update_interval == 0 && context.edge_count > 0) {
            const auto start_idx = edge_dist(rng);
            std::uniform_int_distribution<std::size_t> span_dist(0, max_span);
            const auto span = std::min(span_dist(rng), context.edge_count - start_idx - 1);
            const auto end_idx = start_idx + span;
            const auto factor = factor_dist(rng);

            const auto update_begin = std::chrono::high_resolution_clock::now();
            context.router.apply_congestion_update(start_idx, end_idx, factor);
            const auto update_end = std::chrono::high_resolution_clock::now();

            const std::chrono::duration<double, std::micro> update_duration = update_end - update_begin;
            update_times.push_back(update_duration.count());
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
        route_times.push_back(route_duration.count());

        if (!result.reachable) {
            ++unreachable_count;
        }
    }

    // Run update-only benchmark if requested
    if (mode == "update" && updates > 0) {
        for (std::size_t i = 0; i < updates; ++i) {
            const auto start_idx = edge_dist(rng);
            std::uniform_int_distribution<std::size_t> span_dist(0, max_span);
            const auto span = std::min(span_dist(rng), context.edge_count - start_idx - 1);
            const auto end_idx = start_idx + span;
            const auto factor = factor_dist(rng);

            const auto update_begin = std::chrono::high_resolution_clock::now();
            context.router.apply_congestion_update(start_idx, end_idx, factor);
            const auto update_end = std::chrono::high_resolution_clock::now();

            const std::chrono::duration<double, std::micro> update_duration = update_end - update_begin;
            update_times.push_back(update_duration.count());
        }
    }

    std::cout << "ROUTE_BENCH\n";
    if (!route_times.empty()) {
        const auto route_stats = PercentileStats::compute(route_times);
        print_percentile_stats("route", route_stats);
    }
    std::cout << "\n";

    if (!update_times.empty()) {
        std::cout << "UPDATE_BENCH\n";
        const auto update_stats = PercentileStats::compute(update_times);
        print_percentile_stats("update", update_stats);
        std::cout << "  throughput_updates_per_sec=" << (update_stats.mean > 0 ? 1000000.0 / update_stats.mean : 0.0) << "\n";
        std::cout << "\n";
    }

    if (unreachable_count > 0) {
        std::cout << "Unreachable routes: " << unreachable_count << "\n";
    }

    return 0;
}

