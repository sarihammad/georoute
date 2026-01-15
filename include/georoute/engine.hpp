#pragma once

#include <cstdint>
#include <mutex>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "georoute/router.hpp"
#include "georoute/types.hpp"

namespace georoute {

struct EngineStats {
    std::uint64_t total_queries{0};
    std::uint64_t total_updates{0};
    double total_compute_time_us{0.0};
    double max_compute_time_us{0.0};
};

struct RouteQuery {
    node_id source;
    node_id target;
};

struct CongestionUpdate {
    std::size_t edge_start;
    std::size_t edge_end;
    float factor;
};

struct RouteResponse {
    RouteResult result;
    EngineStats stats;
    std::uint64_t expanded_nodes{0};
    double compute_time_us{0.0};
};

class GeoRouteEngine {
public:
    GeoRouteEngine() = default;
    explicit GeoRouteEngine(Router router);
    
    GeoRouteEngine(const GeoRouteEngine&) = delete;
    GeoRouteEngine& operator=(const GeoRouteEngine&) = delete;
    GeoRouteEngine(GeoRouteEngine&&) = default;
    GeoRouteEngine& operator=(GeoRouteEngine&&) = default;
    ~GeoRouteEngine() = default;

    [[nodiscard]] RouteResponse route(node_id source, node_id target);
    void apply_congestion_update(std::size_t edge_start, std::size_t edge_end, float factor);
    
    [[nodiscard]] EngineStats get_stats() const noexcept;
    void reset_stats() noexcept;
    
    static GeoRouteEngine from_json(const nlohmann::json& config);

private:
    Router router_;
    mutable EngineStats stats_;
    mutable std::mutex stats_mutex_;
};

}  // namespace georoute

