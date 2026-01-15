#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "georoute/router.hpp"
#include "georoute/graph.hpp"
#include "georoute/segment_tree.hpp"

TEST_CASE("Congestion update changes route cost deterministically", "[congestion]") {
    // Create a simple graph: 0 -> 1 -> 2
    // Two paths: direct (0->2) and via (0->1->2)
    georoute::Graph graph{3};
    graph.add_edge(0, 1, 1.0F);  // edge 0: cost 1.0
    graph.add_edge(1, 2, 1.0F);  // edge 1: cost 1.0
    graph.add_edge(0, 2, 3.0F);  // edge 2: cost 3.0 (longer direct path)

    georoute::SegmentTree tree{graph.edge_count()};
    georoute::Router router{std::move(graph), std::move(tree)};

    // Baseline: should take path 0->1->2 (cost 2.0) not 0->2 (cost 3.0)
    const auto baseline = router.compute_route(0, 2);
    REQUIRE(baseline.result.reachable);
    REQUIRE(baseline.result.total_travel_time == Catch::Approx(2.0F));
    REQUIRE(baseline.result.nodes == std::vector<georoute::node_id>{0, 1, 2});

    // Apply congestion to edge 0 (0->1), doubling its cost
    router.apply_congestion_update(0, 0, 2.0F);

    // Now path 0->1->2 costs 2.0 + 1.0 = 3.0, same as direct path
    // Should still prefer 0->1->2 (tie-breaker: first found)
    const auto after_congestion = router.compute_route(0, 2);
    REQUIRE(after_congestion.result.reachable);
    REQUIRE(after_congestion.result.total_travel_time == Catch::Approx(3.0F));

    // Apply more congestion to make direct path better
    router.apply_congestion_update(0, 1, 2.0F);  // Both edges 0 and 1 get 2x more

    // Now 0->1->2 costs (1.0*2*2) + (1.0*2) = 4.0 + 2.0 = 6.0
    // Direct path 0->2 still costs 3.0, so should switch
    const auto after_more_congestion = router.compute_route(0, 2);
    REQUIRE(after_more_congestion.result.reachable);
    REQUIRE(after_more_congestion.result.total_travel_time == Catch::Approx(3.0F));
    REQUIRE(after_more_congestion.result.nodes == std::vector<georoute::node_id>{0, 2});
}

TEST_CASE("Congestion update affects multiple edges in range", "[congestion]") {
    georoute::Graph graph{4};
    graph.add_edge(0, 1, 1.0F);  // edge 0
    graph.add_edge(1, 2, 1.0F);  // edge 1
    graph.add_edge(2, 3, 1.0F);  // edge 2
    graph.add_edge(0, 3, 5.0F);  // edge 3: longer direct path

    georoute::SegmentTree tree{graph.edge_count()};
    georoute::Router router{std::move(graph), std::move(tree)};

    // Baseline: path 0->1->2->3 (cost 3.0)
    const auto baseline = router.compute_route(0, 3);
    REQUIRE(baseline.result.total_travel_time == Catch::Approx(3.0F));

    // Apply congestion to edges 0-2 (all edges in the path)
    router.apply_congestion_update(0, 2, 2.0F);

    // Now path 0->1->2->3 costs 3.0 * 2.0 = 6.0
    // Direct path 0->3 still costs 5.0, so should switch
    const auto after_congestion = router.compute_route(0, 3);
    REQUIRE(after_congestion.result.total_travel_time == Catch::Approx(5.0F));
    REQUIRE(after_congestion.result.nodes == std::vector<georoute::node_id>{0, 3});
}

