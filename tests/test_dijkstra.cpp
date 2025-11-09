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
    const auto result = router.shortest_path(0, 3);

    REQUIRE(result.reachable);
    REQUIRE(result.total_travel_time == Catch::Approx(4.0F));
    REQUIRE(result.nodes == std::vector<georoute::node_id>{0, 1, 2, 3});
}

TEST_CASE("Dijkstra handles unreachable target", "[dijkstra]") {
    georoute::Graph graph{3};
    graph.add_edge(0, 1, 2.0F);

    georoute::SegmentTree congestion{graph.edge_count()};
    georoute::DijkstraRouter router{graph, congestion};

    const auto result = router.shortest_path(0, 2);
    REQUIRE_FALSE(result.reachable);
    REQUIRE(result.nodes.empty());
    REQUIRE(result.total_travel_time == Catch::Approx(0.0F));
}

TEST_CASE("Dijkstra zero-cost when source equals target", "[dijkstra]") {
    georoute::Graph graph{2};
    graph.add_edge(0, 1, 3.0F);

    georoute::SegmentTree congestion{graph.edge_count()};
    georoute::DijkstraRouter router{graph, congestion};

    const auto result = router.shortest_path(1, 1);
    REQUIRE(result.reachable);
    REQUIRE(result.total_travel_time == Catch::Approx(0.0F));
    REQUIRE(result.nodes == std::vector<georoute::node_id>{1});
}


