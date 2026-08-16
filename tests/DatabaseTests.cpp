#include <catch2/catch_test_macros.hpp>

#include <h3/h3api.h>

#include "db/Database.h"

TEST_CASE("Database opens in-memory database successfully")
{
    Database db(":memory:");

    REQUIRE(db.isOpen());
}

TEST_CASE("insertRun returns a valid positive id")
{
    Database db(":memory:");

    auto runId = db.insertRun("data/raw/points.csv", 9);

    REQUIRE(runId > 0);
}

TEST_CASE("insertRun creates a row in runs table")
{
    Database db(":memory:");

    db.insertRun("data/raw/points.csv", 9);

    REQUIRE(db.countRows("runs") == 1);
}

TEST_CASE("insertPoints stores all points")
{
    Database db(":memory:");

    auto runId = db.insertRun("data/raw/points.csv", 9);

    std::vector<Point> points =
    {
        {"p1", 37.775938, -122.419281},
        {"p2", 40.689167, -74.044444}
    };

    db.insertPoints(runId, points);

    REQUIRE(db.countRows("points") == 2);
}

TEST_CASE("insertFeatures stores all features")
{
    Database db(":memory:");

    auto runId = db.insertRun("data/raw/points.csv", 9);

    LatLng coord{};
    coord.lat = degsToRads(37.775938);
    coord.lng = degsToRads(-122.419281);

    H3Index hex{};
    latLngToCell(&coord, 9, &hex);

    HexFeature feature;
    feature.hexId = hex;
    feature.pointsCount = 5;
    feature.rank = 1;
    feature.category = "HIGH";

    std::vector<HexFeature> features = { feature };

    db.insertFeatures(runId, features);

    REQUIRE(db.countRows("hex_features") == 1);
}

TEST_CASE("insertPoints does nothing for invalid run id")
{
    Database db(":memory:");

    std::vector<Point> points = { {"p1", 0.0, 0.0} };

    db.insertPoints(-1, points);

    REQUIRE(db.countRows("points") == 0);
}