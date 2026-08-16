#include <catch2/catch_test_macros.hpp>

#include <h3/h3api.h>

#include "h3/HexGeometry.h"

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

TEST_CASE("getHexBoundary returns closed polygon")
{
    H3Index hex = makeTestHex();

    auto boundary = getHexBoundary(hex);

    REQUIRE(boundary.size() >= 6);

    bool isClosed =
        boundary.front().lat == boundary.back().lat &&
        boundary.front().lon == boundary.back().lon;

    REQUIRE(isClosed);
}

TEST_CASE("getHexBoundary returns coordinates in valid range")
{
    H3Index hex = makeTestHex();

    auto boundary = getHexBoundary(hex);

    for (const auto& point : boundary)
    {
        REQUIRE(point.lat >= -90.0);
        REQUIRE(point.lat <= 90.0);
        REQUIRE(point.lon >= -180.0);
        REQUIRE(point.lon <= 180.0);
    }
}

TEST_CASE("getHexBoundary returns empty vector for invalid hex")
{
    H3Index invalidHex = 0;

    auto boundary = getHexBoundary(invalidHex);

    REQUIRE(boundary.empty());
}