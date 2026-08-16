#include <catch2/catch_test_macros.hpp>

#include "analytics/ScoringEngine.h"
#include "analytics/HexFeature.h"

TEST_CASE("calculateScores handles empty input")
{
    std::vector<HexFeature> features;

    calculateScores(features, 0.7, 0.3, 0.2, 20.0, 50.0, 80.0);

    REQUIRE(features.empty());
}

TEST_CASE("calculateScores does nothing when weights sum to zero")
{
    std::vector<HexFeature> features(2);

    features[0].pointsCount = 10;
    features[1].pointsCount = 20;

    calculateScores(features, 0.0, 0.0, 0.0, 20.0, 50.0, 80.0);

    REQUIRE(features[0].score == 0.0);
    REQUIRE(features[1].score == 0.0);
    REQUIRE(features[0].category.empty());
}

TEST_CASE("calculateScores assigns higher score to hex with more points")
{
    std::vector<HexFeature> features(2);

    features[0].hexId = 1;
    features[0].pointsCount = 100;
    features[0].density = 5.0;

    features[1].hexId = 2;
    features[1].pointsCount = 10;
    features[1].density = 1.0;

    calculateScores(features, 0.7, 0.3, 0.2, 20.0, 50.0, 80.0);

    REQUIRE(features[0].hexId == 1);
    REQUIRE(features[0].score > features[1].score);
    REQUIRE(features[0].rank == 1);
    REQUIRE(features[1].rank == 2);
}

TEST_CASE("calculateScores assigns correct category by threshold")
{
    std::vector<HexFeature> features(1);

    features[0].pointsCount = 100;
    features[0].density = 10.0;

    calculateScores(features, 1.0, 0.0, 0.0, 20.0, 50.0, 80.0);

    REQUIRE(features[0].score == 100.0);
    REQUIRE(features[0].category == "VERY_HIGH");
}

TEST_CASE("calculateScores clamps score to 100")
{
    std::vector<HexFeature> features(1);

    features[0].pointsCount = 50;
    features[0].neighborPointsCount = 50;
    features[0].density = 100.0;

    calculateScores(features, 1.0, 1.0, 1.0, 20.0, 50.0, 80.0);

    REQUIRE(features[0].score <= 100.0);
}