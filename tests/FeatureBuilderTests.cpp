#include <catch2/catch_test_macros.hpp>

#include <h3/h3api.h>

#include "analytics/FeatureBuilder.h"
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

TEST_CASE("buildFeatures creates one feature per hex")
{
    H3Index hex = makeTestHex();

    HexCountMap hexes;
    hexes[hex] = 5;

    auto features = buildFeatures(hexes);

    REQUIRE(features.size() == 1);
    REQUIRE(features[0].hexId == hex);
    REQUIRE(features[0].pointsCount == 5);
}

TEST_CASE("buildFeatures computes positive density")
{
    H3Index hex = makeTestHex();

    HexCountMap hexes;
    hexes[hex] = 10;

    auto features = buildFeatures(hexes);

    REQUIRE(features[0].density > 0.0);
}

TEST_CASE("buildFeatures counts neighbors correctly")
{
    H3Index hex = makeTestHex();

    HexCountMap hexes;
    hexes[hex] = 5;

    auto features = buildFeatures(hexes);

    REQUIRE(features[0].neighborsCount == 6);
}

TEST_CASE("buildFeatures sums points from neighboring hexes")
{
    H3Index hex = makeTestHex();
    auto neighbors = getNeighbors(hex);

    HexCountMap hexes;
    hexes[hex] = 5;
    hexes[neighbors[0]] = 3;
    hexes[neighbors[1]] = 2;

    auto features = buildFeatures(hexes);

    HexFeature* target = nullptr;

    for (auto& feature : features)
    {
        if (feature.hexId == hex)
        {
            target = &feature;
        }
    }

    REQUIRE(target != nullptr);
    REQUIRE(target->neighborPointsCount == 5);
}

TEST_CASE("buildFeatures ignores non-adjacent hexes for neighbor count")
{
    H3Index hex = makeTestHex();

    HexCountMap hexes;
    hexes[hex] = 5;

    auto features = buildFeatures(hexes);

    REQUIRE(features[0].neighborPointsCount == 0);
}