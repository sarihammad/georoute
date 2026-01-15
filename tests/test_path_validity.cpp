#include <catch2/catch_test_macros.hpp>

#include <set>
#include <vector>

#include "georoute/graph.hpp"
#include "georoute/router.hpp"
#include "georoute/segment_tree.hpp"

namespace {

// Helper to verify path validity using public Graph API
bool verify_path_validity(const georoute::Graph& graph, const std::vector<georoute::node_id>& path,
                          georoute::node_id source, georoute::node_id target) {
    if (path.empty()) {
        return false;
    }

    // First node must be source
    if (path.front() != source) {
        return false;
    }

    // Last node must be target
    if (path.back() != target) {
        return false;
    }

    // Verify each consecutive pair in path is a valid edge
    for (std::size_t i = 0; i + 1 < path.size(); ++i) {
        const auto from = path[i];
        const auto to = path[i + 1];
        const auto& neighbors = graph.neighbors(from);
        bool edge_exists = false;
        for (const auto& edge : neighbors) {
            if (edge.to == to) {
                edge_exists = true;
                break;
            }
        }
        if (!edge_exists) {
            return false;
        }
    }

    return true;
}

}  // namespace

TEST_CASE("Router returns valid paths", "[path_validity]") {
    georoute::Graph graph{5};
    graph.add_edge(0, 1, 1.0F);
    graph.add_edge(1, 2, 1.0F);
    graph.add_edge(2, 3, 1.0F);
    graph.add_edge(0, 4, 2.0F);
    graph.add_edge(4, 3, 1.0F);

    georoute::SegmentTree tree{graph.edge_count()};
    georoute::Router router{std::move(graph), std::move(tree)};

    const auto computation = router.compute_route(0, 3);
    REQUIRE(computation.result.reachable);
    
    // Rebuild graph for verification (since router moved it)
    georoute::Graph verify_graph{5};
    verify_graph.add_edge(0, 1, 1.0F);
    verify_graph.add_edge(1, 2, 1.0F);
    verify_graph.add_edge(2, 3, 1.0F);
    verify_graph.add_edge(0, 4, 2.0F);
    verify_graph.add_edge(4, 3, 1.0F);
    
    REQUIRE(verify_path_validity(verify_graph, computation.result.nodes, 0, 3));
}

TEST_CASE("Router path starts at source and ends at target", "[path_validity]") {
    georoute::Graph graph{4};
    graph.add_edge(0, 1, 1.0F);
    graph.add_edge(1, 2, 1.0F);
    graph.add_edge(2, 3, 1.0F);

    georoute::SegmentTree tree{graph.edge_count()};
    georoute::Router router{std::move(graph), std::move(tree)};

    const auto computation = router.compute_route(0, 3);
    REQUIRE(computation.result.reachable);
    REQUIRE(computation.result.nodes.front() == 0);
    REQUIRE(computation.result.nodes.back() == 3);
}

TEST_CASE("Router path has consecutive valid edges", "[path_validity]") {
    georoute::Graph graph{6};
    // Create a more complex graph
    graph.add_edge(0, 1, 1.0F);
    graph.add_edge(1, 2, 2.0F);
    graph.add_edge(2, 5, 1.0F);
    graph.add_edge(0, 3, 1.5F);
    graph.add_edge(3, 4, 1.0F);
    graph.add_edge(4, 5, 1.0F);

    georoute::SegmentTree tree{graph.edge_count()};
    georoute::Router router{std::move(graph), std::move(tree)};

    const auto computation = router.compute_route(0, 5);
    REQUIRE(computation.result.reachable);
    
    // Rebuild graph for verification
    georoute::Graph verify_graph{6};
    verify_graph.add_edge(0, 1, 1.0F);
    verify_graph.add_edge(1, 2, 2.0F);
    verify_graph.add_edge(2, 5, 1.0F);
    verify_graph.add_edge(0, 3, 1.5F);
    verify_graph.add_edge(3, 4, 1.0F);
    verify_graph.add_edge(4, 5, 1.0F);
    
    REQUIRE(verify_path_validity(verify_graph, computation.result.nodes, 0, 5));
}

TEST_CASE("Self-loop path is valid", "[path_validity]") {
    georoute::Graph graph{3};
    graph.add_edge(0, 1, 1.0F);
    graph.add_edge(1, 2, 1.0F);

    georoute::SegmentTree tree{graph.edge_count()};
    georoute::Router router{std::move(graph), std::move(tree)};

    const auto computation = router.compute_route(0, 0);
    REQUIRE(computation.result.reachable);
    REQUIRE(computation.result.nodes.size() == 1);
    REQUIRE(computation.result.nodes[0] == 0);
}

