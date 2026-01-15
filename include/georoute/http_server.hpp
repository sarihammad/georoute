#pragma once

#include <cstdint>
#include <string>

namespace georoute {

class GeoRouteEngine;

struct HttpServerOptions {
    std::string host{"0.0.0.0"};
    std::uint16_t port{8080};
};

int run_http_server(GeoRouteEngine& engine, const HttpServerOptions& options);

}  // namespace georoute

