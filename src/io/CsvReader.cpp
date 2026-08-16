#include "io/CsvReader.h"
#include "io/FileUtils.h"
#include <fstream>
#include <sstream>

#include <spdlog/spdlog.h>

std::vector<Point> readCsv(const std::string& path)
{
    std::vector<Point> points;

    std::ifstream file(path);

    if (!file.is_open())
    {
        spdlog::error("Can't open file: {}", path);
        return points;
    }

    std::string line;

    std::getline(file, line);

    std::size_t lineNumber = 1;
    std::size_t skippedCount = 0;

    while (std::getline(file, line))
    {
        lineNumber++;

        std::stringstream ss(line);

        std::string idStr;
        std::string latStr;
        std::string lonStr;

        std::getline(ss, idStr, ',');
        std::getline(ss, latStr, ',');
        std::getline(ss, lonStr, ',');

        if (idStr.empty() || latStr.empty() || lonStr.empty())
        {
            spdlog::warn("Skipping line {}: missing fields", lineNumber);
            skippedCount++;
            continue;
        }

        double lat = 0.0;
        double lon = 0.0;

        try
        {
            lat = std::stod(latStr);
            lon = std::stod(lonStr);
        }
        catch (const std::exception&)
        {
            spdlog::warn("Skipping line {}: invalid coordinates '{}', '{}'", lineNumber, latStr, lonStr);
            skippedCount++;
            continue;
        }

        if (lat < -90.0 || lat > 90.0 || lon < -180.0 || lon > 180.0)
        {
            spdlog::warn("Skipping line {}: coordinates out of range ({}, {})", lineNumber, lat, lon);
            skippedCount++;
            continue;
        }

        points.push_back({
            idStr,
            lat,
            lon
            });
    }

    if (skippedCount > 0)
    {
        spdlog::warn("Total skipped lines: {}", skippedCount);
    }

    return points;
}