#include <catch2/catch_test_macros.hpp>

#include <h3/h3api.h>

#include "h3/HexMetrics.h"

namespace
{
    H3Index makeTestHex(int resolution)
    {
        LatLng coord{};

        coord.lat = degsToRads(37.775938);
        coord.lng = degsToRads(-122.419281);

        H3Index cell{};

        latLngToCell(&coord, resolution, &cell);

        return cell;
    }
}

TEST_CASE("getHexAreaKm2 returns positive area for valid hex")
{
    H3Index hex = makeTestHex(9);

    double area = getHexAreaKm2(hex);

    REQUIRE(area > 0.0);
}

TEST_CASE("getHexAreaKm2 decreases as resolution increases")
{
    double areaLowRes = getHexAreaKm2(makeTestHex(7));
    double areaHighRes = getHexAreaKm2(makeTestHex(9));

    REQUIRE(areaLowRes > areaHighRes);
}

TEST_CASE("getHexAreaKm2 returns zero for invalid hex")
{
    H3Index invalidHex = 0;

    double area = getHexAreaKm2(invalidHex);

    REQUIRE(area == 0.0);
}