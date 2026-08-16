#include <cstdio>
#include <fstream>
#include <sstream>

#include <catch2/catch_test_macros.hpp>

#include <h3/h3api.h>

#include "io/FeatureWriter.h"

TEST_CASE("writeFeaturesCsv writes header and rows")
{
    HexFeature feature;
    feature.hexId = 0;
    feature.pointsCount = 5;
    feature.density = 1.5;
    feature.score = 42.0;
    feature.confidence = 0.3;
    feature.rank = 1;
    feature.category = "MEDIUM";

    std::vector<HexFeature> features = { feature };

    std::string path = "test_features_temp.csv";

    writeFeaturesCsv(path, features);

    std::ifstream file(path);

    std::string headerLine;
    std::getline(file, headerLine);

    std::string dataLine;
    std::getline(file, dataLine);

    REQUIRE_FALSE(headerLine.empty());
    REQUIRE_FALSE(dataLine.empty());
    REQUIRE(dataLine.find("MEDIUM") != std::string::npos);

    std::remove(path.c_str());
}

TEST_CASE("writeFeaturesCsv handles empty features")
{
    std::vector<HexFeature> features;

    std::string path = "test_features_empty_temp.csv";

    writeFeaturesCsv(path, features);

    std::ifstream file(path);

    std::string headerLine;
    std::getline(file, headerLine);

    std::string dataLine;
    bool hasDataLine = static_cast<bool>(std::getline(file, dataLine));

    REQUIRE_FALSE(headerLine.empty());
    REQUIRE_FALSE(hasDataLine);

    std::remove(path.c_str());
}