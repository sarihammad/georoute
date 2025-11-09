#include <cstdint>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

#include <nlohmann/json.hpp>

#include "georoute/http_server.hpp"
#include "georoute/router.hpp"

namespace {

struct ServerConfig {
    std::string graph_path{};
    std::string host{"0.0.0.0"};
    std::uint16_t port{8080};
};

void print_usage(const char* binary) {
    std::cout << "Usage: " << binary << " --graph <path> [--host <host>] [--port <port>]" << '\n';
}

std::optional<ServerConfig> parse_arguments(int argc, char** argv) {
    ServerConfig config;

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg{argv[i]};
        if (arg == "--graph" && i + 1 < argc) {
            config.graph_path = argv[++i];
        } else if (arg == "--host" && i + 1 < argc) {
            config.host = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            config.port = static_cast<std::uint16_t>(std::stoi(argv[++i]));
        } else {
            return std::nullopt;
        }
    }

    if (config.graph_path.empty()) {
        return std::nullopt;
    }

    return config;
}

}  // namespace

int main(int argc, char** argv) {
    const auto config = parse_arguments(argc, argv);
    if (!config) {
        print_usage(argv[0]);
        return 1;
    }

    std::ifstream input{config->graph_path};
    if (!input) {
        std::cerr << "Failed to open graph file: " << config->graph_path << '\n';
        return 1;
    }

    try {
        nlohmann::json data;
        input >> data;

        auto router = georoute::Router::from_json(data);

        std::cout << "Starting GeoRoute server on " << config->host << ':' << config->port << '\n';
        georoute::HttpServerOptions options{config->host, config->port};
        return georoute::run_http_server(router, options);
    } catch (const std::exception& ex) {
        std::cerr << "Failed to initialize router: " << ex.what() << '\n';
        return 1;
    }
}

