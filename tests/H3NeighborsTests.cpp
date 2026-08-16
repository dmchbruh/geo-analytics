#include <cstddef>
#include <cstdint>

#include <catch2/catch_test_macros.hpp>

#include <h3/h3api.h>

#include "h3/H3Neighbors.h"

namespace
{
    H3Index makeTestHex()
    {
        LatLng coord{};

        coord.lat = degsToRads(37.775938);
        coord.lng = degsToRads(-122.419281);

        H3Index cell{};

        latLngToCell(&coord, 9, &cell);

        return cell;
    }
}

TEST_CASE("getNeighbors returns six neighbors for a normal hex")
{
    H3Index hex = makeTestHex();

    auto neighbors = getNeighbors(hex);

    REQUIRE(neighbors.size() == 6);
}

TEST_CASE("getNeighbors does not include the origin hex")
{
    H3Index hex = makeTestHex();

    auto neighbors = getNeighbors(hex);

    for (auto neighbor : neighbors)
    {
        REQUIRE(neighbor != hex);
    }
}

TEST_CASE("getNeighbors returns cells at grid distance 1")
{
    H3Index hex = makeTestHex();

    auto neighbors = getNeighbors(hex);

    for (auto neighbor : neighbors)
    {
        int64_t distance = 0;

        gridDistance(hex, neighbor, &distance);

        REQUIRE(distance == 1);
    }
}

TEST_CASE("getNeighbors returns unique neighbors")
{
    H3Index hex = makeTestHex();

    auto neighbors = getNeighbors(hex);

    for (std::size_t i = 0; i < neighbors.size(); i++)
    {
        for (std::size_t j = i + 1; j < neighbors.size(); j++)
        {
            REQUIRE(neighbors[i] != neighbors[j]);
        }
    }
}