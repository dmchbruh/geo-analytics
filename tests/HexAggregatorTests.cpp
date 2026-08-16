#include <catch2/catch_test_macros.hpp>

#include "analytics/HexAggregator.h"

TEST_CASE("aggregateHexes returns empty map for empty input")
{
    std::vector<H3Point> points;

    auto result = aggregateHexes(points);

    REQUIRE(result.empty());
}

TEST_CASE("aggregateHexes counts points per hex")
{
    std::vector<H3Point> points =
    {
        {"p1", 0.0, 0.0, 100},
        {"p2", 0.0, 0.0, 100},
        {"p3", 0.0, 0.0, 200}
    };

    auto result = aggregateHexes(points);

    REQUIRE(result.size() == 2);
    REQUIRE(result.at(100) == 2);
    REQUIRE(result.at(200) == 1);
}

TEST_CASE("aggregateHexes handles single hex with many points")
{
    std::vector<H3Point> points;

    for (int i = 0; i < 10; i++)
    {
        points.push_back({ "p", 0.0, 0.0, 42 });
    }

    auto result = aggregateHexes(points);

    REQUIRE(result.size() == 1);
    REQUIRE(result.at(42) == 10);
}