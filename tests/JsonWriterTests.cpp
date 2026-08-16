#include <cstdio>
#include <fstream>

#include <catch2/catch_test_macros.hpp>

#include <nlohmann/json.hpp>

#include "io/JsonWriter.h"

TEST_CASE("writeFeaturesJson writes hex_id as string, not number")
{
    HexFeature feature;
    feature.hexId = 617700169958293503ULL;
    feature.pointsCount = 7;
    feature.score = 55.0;

    std::vector<HexFeature> features = { feature };

    std::string path = "test_json_temp.json";

    writeFeaturesJson(path, features);

    std::ifstream file(path);
    nlohmann::json output;
    file >> output;

    REQUIRE(output.size() == 1);
    REQUIRE(output[0]["hex_id"].is_string());
    REQUIRE(output[0]["points_count"] == 7);

    std::remove(path.c_str());
}

TEST_CASE("writeFeaturesJson handles empty features")
{
    std::vector<HexFeature> features;

    std::string path = "test_json_empty_temp.json";

    writeFeaturesJson(path, features);

    std::ifstream file(path);
    nlohmann::json output;
    file >> output;

    REQUIRE(output.empty());

    std::remove(path.c_str());
}