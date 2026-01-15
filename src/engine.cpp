#include "georoute/engine.hpp"

#include <chrono>
#include <nlohmann/json.hpp>

namespace georoute {

GeoRouteEngine::GeoRouteEngine(Router router)
    : router_(std::move(router)), stats_{} {}

RouteResponse GeoRouteEngine::route(node_id source, node_id target) {
    const auto start = std::chrono::high_resolution_clock::now();
    
    const auto computation = router_.compute_route(source, target);
    
    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    const double compute_time_us = static_cast<double>(duration.count());
    
    {
        std::lock_guard<std::mutex> lock{stats_mutex_};
        stats_.total_queries++;
        stats_.total_compute_time_us += compute_time_us;
        stats_.max_compute_time_us = std::max(stats_.max_compute_time_us, compute_time_us);
    }
    
    RouteResponse response;
    response.result = computation.result;
    response.compute_time_us = compute_time_us;
    response.expanded_nodes = computation.stats.expanded_nodes;
    {
        std::lock_guard<std::mutex> lock{stats_mutex_};
        response.stats = stats_;
    }
    
    return response;
}

void GeoRouteEngine::apply_congestion_update(std::size_t edge_start, std::size_t edge_end, float factor) {
    router_.apply_congestion_update(edge_start, edge_end, factor);
    std::lock_guard<std::mutex> lock{stats_mutex_};
    stats_.total_updates++;
}

EngineStats GeoRouteEngine::get_stats() const noexcept {
    std::lock_guard<std::mutex> lock{stats_mutex_};
    return stats_;
}

void GeoRouteEngine::reset_stats() noexcept {
    std::lock_guard<std::mutex> lock{stats_mutex_};
    stats_ = EngineStats{};
}

GeoRouteEngine GeoRouteEngine::from_json(const nlohmann::json& config) {
    return GeoRouteEngine{Router::from_json(config)};
}

}  // namespace georoute

