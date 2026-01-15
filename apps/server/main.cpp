#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

#include "georoute/app.hpp"

namespace {

void print_usage(const char* binary) {
    std::cout << "Usage: " << binary << " --graph <path> [--host <host>] [--port <port>]" << '\n';
}

std::optional<georoute::AppConfig> parse_arguments(int argc, char** argv) {
    georoute::AppConfig config;

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

    georoute::GeoRouteApp app{*config};
    if (!app.initialize()) {
        return 1;
    }

    return app.run();
}

