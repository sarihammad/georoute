#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "georoute/dijkstra.hpp"
#include "georoute/graph.hpp"
#include "georoute/segment_tree.hpp"

TEST_CASE("Dijkstra finds shortest path in simple graph", "[dijkstra]") {
    georoute::Graph graph{4};
    graph.add_edge(0, 1, 1.0F);
    graph.add_edge(1, 2, 1.0F);
    graph.add_edge(0, 2, 5.0F);
    graph.add_edge(2, 3, 2.0F);

    georoute::SegmentTree congestion{graph.edge_count()};

    georoute::DijkstraRouter router{graph, congestion};
    const auto computation = router.shortest_path(0, 3);

    REQUIRE(computation.result.reachable);
    REQUIRE(computation.result.total_travel_time == Catch::Approx(4.0F));
    REQUIRE(computation.result.nodes == std::vector<georoute::node_id>{0, 1, 2, 3});
    REQUIRE(computation.stats.expanded_nodes > 0);
}

TEST_CASE("Dijkstra handles unreachable target", "[dijkstra]") {
    georoute::Graph graph{3};
    graph.add_edge(0, 1, 2.0F);

    georoute::SegmentTree congestion{graph.edge_count()};
    georoute::DijkstraRouter router{graph, congestion};

    const auto computation = router.shortest_path(0, 2);
    REQUIRE_FALSE(computation.result.reachable);
    REQUIRE(computation.result.nodes.empty());
    REQUIRE(computation.result.total_travel_time == Catch::Approx(0.0F));
    REQUIRE(computation.stats.expanded_nodes > 0);
}

TEST_CASE("Dijkstra zero-cost when source equals target", "[dijkstra]") {
    georoute::Graph graph{2};
    graph.add_edge(0, 1, 3.0F);

    georoute::SegmentTree congestion{graph.edge_count()};
    georoute::DijkstraRouter router{graph, congestion};

    const auto computation = router.shortest_path(1, 1);
    REQUIRE(computation.result.reachable);
    REQUIRE(computation.result.total_travel_time == Catch::Approx(0.0F));
    REQUIRE(computation.result.nodes == std::vector<georoute::node_id>{1});
    REQUIRE(computation.stats.expanded_nodes == 1);
}

TEST_CASE("Dijkstra stats are non-zero on non-trivial graphs", "[dijkstra]") {
    // Create a larger graph to ensure meaningful stats
    georoute::Graph graph{10};
    // Create a grid-like structure
    for (georoute::node_id i = 0; i < 9; ++i) {
        graph.add_edge(i, i + 1, 1.0F);
        if (i % 3 != 2) {
            graph.add_edge(i, i + 3, 1.0F);
        }
    }

    georoute::SegmentTree congestion{graph.edge_count()};
    georoute::DijkstraRouter router{graph, congestion};

    const auto computation = router.shortest_path(0, 9);
    REQUIRE(computation.result.reachable);
    
    // On a non-trivial graph, we should expand multiple nodes
    REQUIRE(computation.stats.expanded_nodes > 1);
    REQUIRE(computation.stats.relaxed_edges > 0);
    REQUIRE(computation.stats.visited_nodes > 1);
    
    // Stats should be consistent
    REQUIRE(computation.stats.visited_nodes <= computation.stats.expanded_nodes);
}


