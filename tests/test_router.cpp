#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <nlohmann/json.hpp>

#include <utility>
#include <vector>

#include "georoute/router.hpp"

namespace {

georoute::Router build_sample_router() {
    constexpr georoute::node_id node_count = 4;
    georoute::Graph graph{node_count};
    graph.add_edge(0, 1, 1.0F);  // edge 0
    graph.add_edge(1, 3, 1.0F);  // edge 1
    graph.add_edge(0, 2, 2.0F);  // edge 2
    graph.add_edge(2, 3, 1.0F);  // edge 3

    georoute::SegmentTree tree{graph.edge_count()};
    return georoute::Router{std::move(graph), std::move(tree)};
}

}  // namespace

TEST_CASE("Router computes routes with congestion applied", "[router]") {
    auto router = build_sample_router();

    const auto baseline = router.compute_route(0, 3);
    REQUIRE(baseline.reachable);
    REQUIRE(baseline.total_travel_time == Catch::Approx(2.0F));
    REQUIRE(baseline.nodes == std::vector<georoute::node_id>{0, 1, 3});

    router.apply_congestion_update(0, 1, 2.5F);

    const auto congested = router.compute_route(0, 3);
    REQUIRE(congested.reachable);
    REQUIRE(congested.total_travel_time == Catch::Approx(3.0F));
    REQUIRE(congested.nodes == std::vector<georoute::node_id>{0, 2, 3});
}

TEST_CASE("Router loads from JSON configuration", "[router]") {
    const auto json = R"({
        "nodes": 4,
        "edges": [
            { "from": 0, "to": 1, "base_travel_time": 1.0 },
            { "from": 1, "to": 3, "base_travel_time": 1.0 },
            { "from": 0, "to": 2, "base_travel_time": 3.0 },
            { "from": 2, "to": 3, "base_travel_time": 1.0 }
        ]
    })"_json;

    auto router = georoute::Router::from_json(json);
    const auto route = router.compute_route(0, 3);

    REQUIRE(route.reachable);
    REQUIRE(route.total_travel_time == Catch::Approx(2.0F));
    REQUIRE(route.nodes == std::vector<georoute::node_id>{0, 1, 3});
}


