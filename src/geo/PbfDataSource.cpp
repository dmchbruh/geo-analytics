#include "geo/PbfDataSource.h"

#include <atomic>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

namespace
{
    std::mutex gPbfPipelineMutex;
    std::atomic<std::uint64_t> gPbfJobCounter{ 0 };

    struct TempPaths
    {
        fs::path extracted;
        fs::path filtered;
        fs::path exported;
    };

    TempPaths makeTempPaths()
    {
        const auto jobId = gPbfJobCounter.fetch_add(1);
        const auto base = fs::temp_directory_path() / ("geoanalytics_pbf_" + std::to_string(jobId));

        return {
            base.string() + "_extract.osm.pbf",
            base.string() + "_filtered.osm.pbf",
            base.string() + "_export.geojson"
        };
    }

    void cleanupTempFiles(const TempPaths& paths)
    {
        std::error_code ec;
        fs::remove(paths.extracted, ec);
        fs::remove(paths.filtered, ec);
        fs::remove(paths.exported, ec);
    }
}

std::vector<Point> parseOsmiumGeoJson(const std::string& json)
{
    std::vector<Point> points;

    nlohmann::json parsed;

    try
    {
        parsed = nlohmann::json::parse(json);
            parsed.contains("features") ? parsed["features"].size() : 0;
    }

    catch (const nlohmann::json::parse_error& e)
    {
        spdlog::error("Failed to parse osmium export output: {}", e.what());
        return points;
    }

    if (!parsed.contains("features") || !parsed["features"].is_array())
    {
        return points;
    }

    std::size_t skipped = 0;
    std::int64_t fallbackId = 0;

    for (const auto& feature : parsed["features"])
    {
        if (!feature.contains("geometry") || feature["geometry"].is_null())
        {
            skipped++;
            continue;
        }

        const auto& geometry = feature["geometry"];

        if (!geometry.contains("coordinates"))
        {
            skipped++;
            continue;
        }

        if (geometry.value("type", "") != "Point")
        {
            skipped++;
            continue;
        }

        const auto& coords = geometry["coordinates"];

        if (!coords.is_array() || coords.size() < 2)
        {
            skipped++;
            continue;
        }

        if (!coords[0].is_number() || !coords[1].is_number())
        {
            skipped++;
            continue;
        }

        double lon = coords[0].get<double>();
        double lat = coords[1].get<double>();

        if (lat < -90.0 || lat > 90.0 || lon < -180.0 || lon > 180.0)
        {
            skipped++;
            continue;
        }

        std::string id = std::to_string(fallbackId++);

        if (feature.contains("properties") && !feature["properties"].is_null() && feature["properties"].contains("@id"))
        {
            id = feature["properties"]["@id"].dump();
        }

        bool isChain = false;
        std::string brandName;

        if (feature.contains("properties") && !feature["properties"].is_null())
        {
            const auto& props = feature["properties"];

            if (props.contains("brand") && props["brand"].is_string())
            {
                isChain = true;
                brandName = props["brand"].get<std::string>();
            }
            else if (props.contains("name") && props["name"].is_string())
            {
                brandName = props["name"].get<std::string>();
            }
        }

        points.push_back({
            id,
            lat,
            lon,
            isChain,
            brandName
            });
    }

    if (skipped > 0)
    {
        spdlog::warn("parseOsmiumGeoJson: skipped {} invalid features", skipped);
    }

    return points;
}

std::optional<std::vector<Point>> fetchPbfPoints(
    const std::string& pbfPath,
    const BoundingBox& bbox,
    const std::vector<OsmTag>& tags
)
{
    if (tags.empty())
    {
        return std::vector<Point>{};
    }

    std::lock_guard lock(gPbfPipelineMutex);

    const TempPaths tempPaths = makeTempPaths();

    std::string bboxStr =
        std::to_string(bbox.west) + "," +
        std::to_string(bbox.south) + "," +
        std::to_string(bbox.east) + "," +
        std::to_string(bbox.north);

    std::ostringstream extractCmd;
    extractCmd << "osmium extract -b \"" << bboxStr << "\" -s smart --overwrite -o \""
        << tempPaths.extracted.string() << "\" \"" << pbfPath << "\"";

    spdlog::info("Running: {}", extractCmd.str());

    int extractResult = std::system(extractCmd.str().c_str());

    spdlog::info("osmium extract finished with code {}", extractResult);

    if (extractResult != 0)
    {
        spdlog::error("osmium extract failed for '{}'", pbfPath);
        cleanupTempFiles(tempPaths);
        return std::nullopt;
    }

    std::ostringstream filterCmd;
    filterCmd << "osmium tags-filter --overwrite -o \"" << tempPaths.filtered.string()
        << "\" \"" << tempPaths.extracted.string() << "\"";

    for (const auto& tag : tags)
    {
        filterCmd << " " << toOsmiumFilter(tag);
    }

    spdlog::info("Running: {}", filterCmd.str());

    int filterResult = std::system(filterCmd.str().c_str());

    spdlog::info("osmium tags-filter finished with code {}", filterResult);

    if (filterResult != 0)
    {
        spdlog::error("osmium tags-filter failed");
        cleanupTempFiles(tempPaths);
        return std::nullopt;
    }

    std::ostringstream exportCmd;
    exportCmd << "osmium export --geometry-type=point -f geojson --overwrite -o \""
        << tempPaths.exported.string() << "\" \"" << tempPaths.filtered.string() << "\"";

    spdlog::info("Running: {}", exportCmd.str());

    int exportResult = std::system(exportCmd.str().c_str());

    spdlog::info("osmium export finished with code {}", exportResult);

    if (exportResult != 0)
    {
        spdlog::error("osmium export failed");
        cleanupTempFiles(tempPaths);
        return std::nullopt;
    }

    std::ifstream file(tempPaths.exported);

    if (!file.is_open())
    {
        spdlog::error("Could not open osmium export output: {}", tempPaths.exported.string());
        cleanupTempFiles(tempPaths);
        return std::nullopt;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    spdlog::info("Parsing osmium export output ({} bytes)", buffer.str().size());;

    auto points = parseOsmiumGeoJson(buffer.str());

    spdlog::info("Fetched {} points from local PBF ({})", points.size(), pbfPath);

    cleanupTempFiles(tempPaths);

    return points;
}