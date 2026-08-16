#include <cstdio>
#include <fstream>

#include <catch2/catch_test_macros.hpp>

#include <h3/h3api.h>
#include <nlohmann/json.hpp>

#include "analytics/AnalyticsReportWriter.h"

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

    std::vector<HexFeature> makeTestFeatures()
    {
        H3Index hex = makeTestHex();

        HexFeature a;
        a.hexId = hex;
        a.pointsCount = 10;
        a.neighborsCount = 6;
        a.neighborPointsCount = 4;
        a.density = 2.0;
        a.score = 90.0;
        a.confidence = 0.5;
        a.rank = 1;
        a.category = "VERY_HIGH";

        HexFeature b;
        b.hexId = hex + 1;
        b.pointsCount = 2;
        b.neighborsCount = 6;
        b.neighborPointsCount = 0;
        b.density = 0.5;
        b.score = 10.0;
        b.confidence = 0.1;
        b.rank = 2;
        b.category = "LOW";

        return { a, b };
    }
}

TEST_CASE("writeSummaryReport writes correct hex count and averages")
{
    auto features = makeTestFeatures();

    std::string path = "test_summary_temp.json";

    writeSummaryReport(path, features, 10);

    std::ifstream file(path);
    nlohmann::json report;
    file >> report;

    REQUIRE(report["hex_count"] == 2);

    double expectedAvgScore = (90.0 + 10.0) / 2.0;
    double actualAvgScore = report["averages"]["score"];

    REQUIRE(actualAvgScore == expectedAvgScore);

    std::remove(path.c_str());
}

TEST_CASE("writeSummaryReport builds correct category distribution")
{
    auto features = makeTestFeatures();

    std::string path = "test_summary_temp2.json";

    writeSummaryReport(path, features, 10);

    std::ifstream file(path);
    nlohmann::json report;
    file >> report;

    REQUIRE(report["category_distribution"]["VERY_HIGH"] == 1);
    REQUIRE(report["category_distribution"]["LOW"] == 1);

    std::remove(path.c_str());
}

TEST_CASE("writeSummaryReport limits top zones to requested count")
{
    auto features = makeTestFeatures();

    std::string path = "test_summary_temp3.json";

    writeSummaryReport(path, features, 1);

    std::ifstream file(path);
    nlohmann::json report;
    file >> report;

    REQUIRE(report["top_zones"].size() == 1);
    REQUIRE(report["top_zones"][0]["rank"] == 1);

    std::remove(path.c_str());
}

TEST_CASE("writeSummaryReport handles empty features")
{
    std::vector<HexFeature> features;

    std::string path = "test_summary_temp4.json";

    writeSummaryReport(path, features, 10);

    std::ifstream file(path);
    nlohmann::json report;
    file >> report;

    REQUIRE(report["hex_count"] == 0);
    REQUIRE(report["top_zones"].empty());

    std::remove(path.c_str());
}