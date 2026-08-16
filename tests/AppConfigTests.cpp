#include <filesystem>

#include <catch2/catch_test_macros.hpp>

#include <nlohmann/json.hpp>

#include "config/AppConfig.h"

TEST_CASE("parseConfig uses defaults for empty json")
{
    nlohmann::json json = nlohmann::json::object();

    AppConfig config = parseConfig(json, "/project/root");

    REQUIRE(config.resolution == 9);
    REQUIRE(config.pointWeight == 0.7);
    REQUIRE(config.neighborWeight == 0.2);
    REQUIRE(config.densityWeight == 0.1);
    REQUIRE(config.lowThreshold == 20.0);
    REQUIRE(config.mediumThreshold == 50.0);
    REQUIRE(config.highThreshold == 80.0);
}

TEST_CASE("parseConfig reads provided values")
{
    nlohmann::json json =
    {
        {"resolution", 7},
        {"point_weight", 0.5},
        {"neighbor_weight", 0.3},
        {"density_weight", 0.2}
    };

    AppConfig config = parseConfig(json, "/project/root");

    REQUIRE(config.resolution == 7);
    REQUIRE(config.pointWeight == 0.5);
    REQUIRE(config.neighborWeight == 0.3);
    REQUIRE(config.densityWeight == 0.2);
}

TEST_CASE("parseConfig falls back to default resolution when out of range")
{
    nlohmann::json json = { {"resolution", 42} };

    AppConfig config = parseConfig(json, "/project/root");

    REQUIRE(config.resolution == 9);
}

TEST_CASE("parseConfig falls back to default resolution when negative")
{
    nlohmann::json json = { {"resolution", -3} };

    AppConfig config = parseConfig(json, "/project/root");

    REQUIRE(config.resolution == 9);
}

TEST_CASE("parseConfig builds paths relative to root")
{
    nlohmann::json json = nlohmann::json::object();

    AppConfig config = parseConfig(json, "/project/root");

    bool pathMatches =
        config.inputFile.generic_string() == "/project/root/data/raw/points.csv";

    REQUIRE(pathMatches);
}

TEST_CASE("parseConfig rejects negative weights")
{
    nlohmann::json json = { {"point_weight", -1.0} };

    AppConfig config = parseConfig(json, "/project/root");

    REQUIRE(config.pointWeight == 0.7);
    REQUIRE(config.neighborWeight == 0.2);
    REQUIRE(config.densityWeight == 0.1);
}

TEST_CASE("parseConfig rejects thresholds out of order")
{
    nlohmann::json json =
    {
        {"low_threshold", 80.0},
        {"medium_threshold", 50.0},
        {"high_threshold", 20.0}
    };

    AppConfig config = parseConfig(json, "/project/root");

    REQUIRE(config.lowThreshold == 20.0);
    REQUIRE(config.mediumThreshold == 50.0);
    REQUIRE(config.highThreshold == 80.0);
}