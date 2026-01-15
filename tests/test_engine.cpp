#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <nlohmann/json.hpp>

#include "georoute/engine.hpp"
#include "georoute/graph.hpp"
#include "georoute/router.hpp"
#include "georoute/segment_tree.hpp"

TEST_CASE("GeoRouteEngine computes routes with stats", "[engine]") {
    georoute::Graph graph{4};
    graph.add_edge(0, 1, 1.0F);
    graph.add_edge(1, 3, 1.0F);
    graph.add_edge(0, 2, 2.0F);
    graph.add_edge(2, 3, 1.0F);

    georoute::SegmentTree tree{graph.edge_count()};
    georoute::Router router{std::move(graph), std::move(tree)};
    georoute::GeoRouteEngine engine{std::move(router)};

    const auto response = engine.route(0, 3);
    
    REQUIRE(response.result.reachable);
    REQUIRE(response.result.total_travel_time == Catch::Approx(2.0F));
    REQUIRE(response.result.nodes == std::vector<georoute::node_id>{0, 1, 3});
    REQUIRE(response.compute_time_us >= 0.0);
    REQUIRE(response.expanded_nodes > 0);
    
    const auto stats = engine.get_stats();
    REQUIRE(stats.total_queries == 1);
    REQUIRE(stats.total_updates == 0);
    REQUIRE(stats.total_compute_time_us >= 0.0);
}

TEST_CASE("GeoRouteEngine applies congestion updates", "[engine]") {
    georoute::Graph graph{4};
    graph.add_edge(0, 1, 1.0F);  // edge 0
    graph.add_edge(1, 3, 1.0F);  // edge 1
    graph.add_edge(0, 2, 2.0F);  // edge 2
    graph.add_edge(2, 3, 1.0F);  // edge 3

    georoute::SegmentTree tree{graph.edge_count()};
    georoute::Router router{std::move(graph), std::move(tree)};
    georoute::GeoRouteEngine engine{std::move(router)};

    const auto baseline = engine.route(0, 3);
    REQUIRE(baseline.result.total_travel_time == Catch::Approx(2.0F));
    REQUIRE(baseline.result.nodes == std::vector<georoute::node_id>{0, 1, 3});

    engine.apply_congestion_update(0, 1, 2.5F);

    const auto congested = engine.route(0, 3);
    REQUIRE(congested.result.total_travel_time == Catch::Approx(3.0F));
    REQUIRE(congested.result.nodes == std::vector<georoute::node_id>{0, 2, 3});

    const auto stats = engine.get_stats();
    REQUIRE(stats.total_queries == 2);
    REQUIRE(stats.total_updates == 1);
}

TEST_CASE("GeoRouteEngine loads from JSON", "[engine]") {
    const auto json = R"({
        "nodes": 4,
        "edges": [
            { "from": 0, "to": 1, "base_travel_time": 1.0 },
            { "from": 1, "to": 3, "base_travel_time": 1.0 },
            { "from": 0, "to": 2, "base_travel_time": 3.0 },
            { "from": 2, "to": 3, "base_travel_time": 1.0 }
        ]
    })"_json;

    auto engine = georoute::GeoRouteEngine::from_json(json);
    const auto response = engine.route(0, 3);

    REQUIRE(response.result.reachable);
    REQUIRE(response.result.total_travel_time == Catch::Approx(2.0F));
    REQUIRE(response.result.nodes == std::vector<georoute::node_id>{0, 1, 3});
    REQUIRE(response.expanded_nodes > 0);
}

TEST_CASE("GeoRouteEngine tracks stats across multiple queries", "[engine]") {
    georoute::Graph graph{3};
    graph.add_edge(0, 1, 1.0F);
    graph.add_edge(1, 2, 1.0F);

    georoute::SegmentTree tree{graph.edge_count()};
    georoute::Router router{std::move(graph), std::move(tree)};
    georoute::GeoRouteEngine engine{std::move(router)};

    engine.route(0, 1);
    engine.route(0, 2);
    engine.route(1, 2);

    const auto stats = engine.get_stats();
    REQUIRE(stats.total_queries == 3);
    REQUIRE(stats.total_compute_time_us > 0.0);
    
    engine.reset_stats();
    const auto reset_stats = engine.get_stats();
    REQUIRE(reset_stats.total_queries == 0);
    REQUIRE(reset_stats.total_compute_time_us == 0.0);
}

