#include "config/AppConfig.h"
#include "ProjectPaths.h"

#include <fstream>

#include <spdlog/spdlog.h>

AppConfig parseConfig(
    const nlohmann::json& json,
    const std::filesystem::path& root
)
{
    AppConfig config;

    config.inputFile =
        root / json.value("input", "data/raw/points.csv");

    config.outputFile =
        root / json.value("output", "data/processed/points_h3.csv");

    config.featuresOutputFile =
        root / json.value("features_output", "data/processed/hex_features.csv");

    config.featuresJsonOutputFile =
        root / json.value("features_json_output", "data/processed/hex_features.json");

    config.geoJsonOutputFile =
        root / json.value("geojson_output", "data/processed/hex_features.geojson");

    config.summaryOutputFile =
        root / json.value("summary_output", "data/processed/summary.json");

    config.databaseFile =
        root / json.value("database", "data/geoanalytics.db");

    config.resolution = json.value("resolution", 9);

    if (config.resolution < 0 || config.resolution > 15)
    {
        spdlog::error("Invalid resolution {} in config, falling back to 9", config.resolution);
        config.resolution = 9;
    }

    config.pointWeight = json.value("point_weight", 0.7);
    config.neighborWeight = json.value("neighbor_weight", 0.2);
    config.densityWeight = json.value("density_weight", 0.1);

    config.lowThreshold = json.value("low_threshold", 20.0);
    config.mediumThreshold = json.value("medium_threshold", 50.0);
    config.highThreshold = json.value("high_threshold", 80.0);

    if (config.pointWeight < 0.0 || config.neighborWeight < 0.0 || config.densityWeight < 0.0)
    {
        spdlog::error("Negative scoring weights in config, falling back to defaults");
        config.pointWeight = 0.7;
        config.neighborWeight = 0.2;
        config.densityWeight = 0.1;
    }

    if (!(config.lowThreshold < config.mediumThreshold && config.mediumThreshold < config.highThreshold))
    {
        spdlog::error(
            "Category thresholds not in ascending order (low={}, medium={}, high={}), falling back to defaults",
            config.lowThreshold,
            config.mediumThreshold,
            config.highThreshold
        );
        config.lowThreshold = 20.0;
        config.mediumThreshold = 50.0;
        config.highThreshold = 80.0;
    }

    return config;
}

AppConfig loadConfig()
{
    auto root = findProjectRoot();

    auto configPath = root / "config/app.json";

    nlohmann::json json;

    std::ifstream file(configPath);

    if (!file.is_open())
    {
        spdlog::warn("Config file not found: {}, using defaults", configPath.string());
    }
    else
    {
        try
        {
            file >> json;
        }
        catch (const nlohmann::json::parse_error& e)
        {
            spdlog::error("Failed to parse config file {}: {}", configPath.string(), e.what());
            json = nlohmann::json::object();
        }
    }

    return parseConfig(json, root);
}