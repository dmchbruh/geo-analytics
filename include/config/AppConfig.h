#pragma once

#include <filesystem>

#include <nlohmann/json.hpp>

struct AppConfig
{
    std::filesystem::path inputFile;
    std::filesystem::path outputFile;
    std::filesystem::path featuresOutputFile;
    std::filesystem::path featuresJsonOutputFile;
    std::filesystem::path geoJsonOutputFile;
    std::filesystem::path summaryOutputFile;
    std::filesystem::path databaseFile;
    int resolution;
    double pointWeight;
    double neighborWeight;
    double densityWeight;

    double lowThreshold;
    double mediumThreshold;
    double highThreshold;
};

AppConfig parseConfig(
    const nlohmann::json& json,
    const std::filesystem::path& root
);

AppConfig loadConfig();