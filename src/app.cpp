#include "georoute/app.hpp"

#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include "georoute/engine.hpp"
#include "georoute/http_server.hpp"

namespace georoute {

GeoRouteApp::GeoRouteApp(AppConfig config)
    : config_(std::move(config)) {}

GeoRouteApp::~GeoRouteApp() {
    shutdown();
}

bool GeoRouteApp::initialize() {
    if (initialized_) {
        return true;
    }
    
    std::ifstream input{config_.graph_path};
    if (!input) {
        std::cerr << "Failed to open graph file: " << config_.graph_path << '\n';
        return false;
    }
    
    try {
        nlohmann::json data;
        input >> data;
        engine_ = std::make_unique<GeoRouteEngine>(GeoRouteEngine::from_json(data));
        initialized_ = true;
        std::cout << "GeoRoute engine initialized with graph from: " << config_.graph_path << '\n';
        return true;
    } catch (const std::exception& ex) {
        std::cerr << "Failed to initialize engine: " << ex.what() << '\n';
        return false;
    }
}

int GeoRouteApp::run() {
    if (!initialized_) {
        if (!initialize()) {
            return 1;
        }
    }
    
    std::cout << "Starting GeoRoute server on " << config_.host << ':' << config_.port << '\n';
    
    HttpServerOptions options{config_.host, config_.port};
    return run_http_server(*engine_, options);
}

void GeoRouteApp::shutdown() {
    if (initialized_) {
        std::cout << "Shutting down GeoRoute server...\n";
        initialized_ = false;
    }
}

}  // namespace georoute

