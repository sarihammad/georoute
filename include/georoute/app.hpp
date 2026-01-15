#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace georoute {

class GeoRouteEngine;

struct AppConfig {
    std::string graph_path{};
    std::string host{"0.0.0.0"};
    std::uint16_t port{8080};
};

class GeoRouteApp {
public:
    explicit GeoRouteApp(AppConfig config);
    ~GeoRouteApp();
    
    GeoRouteApp(const GeoRouteApp&) = delete;
    GeoRouteApp& operator=(const GeoRouteApp&) = delete;
    GeoRouteApp(GeoRouteApp&&) = default;
    GeoRouteApp& operator=(GeoRouteApp&&) = default;
    
    bool initialize();
    int run();
    void shutdown();

private:
    AppConfig config_;
    std::unique_ptr<GeoRouteEngine> engine_;
    bool initialized_{false};
};

}  // namespace georoute

