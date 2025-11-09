#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <stdexcept>

#include "georoute/segment_tree.hpp"

TEST_CASE("SegmentTree single element update", "[segment_tree]") {
    georoute::SegmentTree tree{5};
    tree.range_multiply(2, 2, 1.5F);

    REQUIRE(tree.point_query(0) == Catch::Approx(1.0F));
    REQUIRE(tree.point_query(2) == Catch::Approx(1.5F));
    REQUIRE(tree.point_query(4) == Catch::Approx(1.0F));
}

TEST_CASE("SegmentTree overlapping updates", "[segment_tree]") {
    georoute::SegmentTree tree{6};
    tree.range_multiply(0, 3, 2.0F);
    tree.range_multiply(2, 5, 0.5F);

    REQUIRE(tree.point_query(1) == Catch::Approx(2.0F));
    REQUIRE(tree.point_query(2) == Catch::Approx(1.0F));
    REQUIRE(tree.point_query(4) == Catch::Approx(0.5F));
}

TEST_CASE("SegmentTree entire range update", "[segment_tree]") {
    georoute::SegmentTree tree{4};
    tree.range_multiply(0, 3, 1.2F);
    tree.range_multiply(1, 2, 0.8F);

    REQUIRE(tree.point_query(0) == Catch::Approx(1.2F));
    REQUIRE(tree.point_query(1) == Catch::Approx(0.96F));
    REQUIRE(tree.point_query(2) == Catch::Approx(0.96F));
    REQUIRE(tree.point_query(3) == Catch::Approx(1.2F));
}

TEST_CASE("SegmentTree invalid operations throw", "[segment_tree]") {
    georoute::SegmentTree tree{3};

    REQUIRE_THROWS_AS(tree.range_multiply(2, 1, 1.0F), std::invalid_argument);
    REQUIRE_THROWS_AS(tree.range_multiply(0, 3, 1.0F), std::out_of_range);
    REQUIRE_THROWS_AS(tree.point_query(3), std::out_of_range);
}


