#include <cstdint>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include <nlohmann/json.hpp>

#include "georoute/router.hpp"

namespace {

struct CongestionUpdate {
    std::size_t edge_start;
    std::size_t edge_end;
    float factor;
};

struct RouteQuery {
    georoute::node_id source;
    georoute::node_id target;
};

using Operation = std::variant<CongestionUpdate, RouteQuery>;

struct CliArguments {
    std::string graph_path;
    std::vector<Operation> operations;
};

void print_usage(const char* binary) {
    std::cout << "GeoRoute CLI\n"
              << "Usage: " << binary
              << " --graph <path> [--congestion <edge_start> <edge_end> <factor>]... [--route <source> <target>]...\n";
}

bool parse_arguments(int argc, char** argv, CliArguments& out_args) {
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg{argv[i]};
        if (arg == "--graph") {
            if (i + 1 >= argc) {
                std::cerr << "--graph requires a path argument\n";
                return false;
            }
            out_args.graph_path = argv[++i];
        } else if (arg == "--congestion") {
            if (i + 3 >= argc) {
                std::cerr << "--congestion requires start end factor\n";
                return false;
            }
            try {
                const auto start = static_cast<std::size_t>(std::stoul(argv[++i]));
                const auto end = static_cast<std::size_t>(std::stoul(argv[++i]));
                const auto factor = std::stof(argv[++i]);
                out_args.operations.emplace_back(CongestionUpdate{start, end, factor});
            } catch (const std::exception& ex) {
                std::cerr << "Invalid --congestion parameters: " << ex.what() << '\n';
                return false;
            }
        } else if (arg == "--route") {
            if (i + 2 >= argc) {
                std::cerr << "--route requires source target\n";
                return false;
            }
            try {
                const auto source = static_cast<georoute::node_id>(std::stoul(argv[++i]));
                const auto target = static_cast<georoute::node_id>(std::stoul(argv[++i]));
                out_args.operations.emplace_back(RouteQuery{source, target});
            } catch (const std::exception& ex) {
                std::cerr << "Invalid --route parameters: " << ex.what() << '\n';
                return false;
            }
        } else if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return false;
        } else {
            std::cerr << "Unknown argument: " << arg << '\n';
            return false;
        }
    }

    if (out_args.graph_path.empty()) {
        std::cerr << "--graph argument is required\n";
        return false;
    }

    return true;
}

std::optional<nlohmann::json> load_graph_json(const std::string& path) {
    std::ifstream input{path};
    if (!input) {
        std::cerr << "Failed to open graph file: " << path << '\n';
        return std::nullopt;
    }
    try {
        nlohmann::json data;
        input >> data;
        return data;
    } catch (const std::exception& ex) {
        std::cerr << "Failed to parse graph JSON: " << ex.what() << '\n';
        return std::nullopt;
    }
}

void print_route_result(const georoute::RouteResult& result) {
    if (!result.reachable) {
        std::cout << "Route unreachable\n";
        return;
    }

    std::cout << "Total travel time: " << result.total_travel_time << " seconds\n";
    std::cout << "Path nodes: ";
    for (std::size_t i = 0; i < result.nodes.size(); ++i) {
        std::cout << result.nodes[i];
        if (i + 1 < result.nodes.size()) {
            std::cout << " -> ";
        }
    }
    std::cout << '\n';
}

}  // namespace

int main(int argc, char** argv) {
    CliArguments args;
    if (!parse_arguments(argc, argv, args)) {
        print_usage(argv[0]);
        return 1;
    }

    const auto graph_json = load_graph_json(args.graph_path);
    if (!graph_json) {
        return 1;
    }

    georoute::Router router = georoute::Router::from_json(*graph_json);

    if (args.operations.empty()) {
        std::cout << "No operations supplied. Use --route and/or --congestion.\n";
        return 0;
    }

    try {
        for (const auto& op : args.operations) {
            std::visit(
                [&](const auto& operation) {
                    using T = std::decay_t<decltype(operation)>;
                    if constexpr (std::is_same_v<T, CongestionUpdate>) {
                        router.apply_congestion_update(operation.edge_start, operation.edge_end, operation.factor);
                        std::cout << "Applied congestion factor " << operation.factor << " to edges ["
                                  << operation.edge_start << ", " << operation.edge_end << "]\n";
                    } else if constexpr (std::is_same_v<T, RouteQuery>) {
                        const auto computation = router.compute_route(operation.source, operation.target);
                        std::cout << "Route from " << operation.source << " to " << operation.target << ":\n";
                        print_route_result(computation.result);
                    }
                },
                op);
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error during CLI execution: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}


