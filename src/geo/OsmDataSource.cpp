#include "geo/OsmDataSource.h"

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <chrono>
#include <thread>

namespace
{
    const char* kUserAgent = "GeoAnalytics/1.0 (student diploma project)";
}

std::optional<BoundingBox> parseGeocodeResponse(const std::string& json)
{
    nlohmann::json parsed;

    try
    {
        parsed = nlohmann::json::parse(json);
    }
    catch (const nlohmann::json::parse_error& e)
    {
        spdlog::error("Failed to parse geocoding response: {}", e.what());
        return std::nullopt;
    }

    if (!parsed.is_array() || parsed.empty())
    {
        return std::nullopt;
    }

    const auto& place = parsed[0];

    if (!place.contains("boundingbox") || !place["boundingbox"].is_array() || place["boundingbox"].size() != 4)
    {
        spdlog::error("Unexpected geocoding response format");
        return std::nullopt;
    }

    try
    {
        BoundingBox bbox;
        bbox.south = std::stod(place["boundingbox"][0].get<std::string>());
        bbox.north = std::stod(place["boundingbox"][1].get<std::string>());
        bbox.west = std::stod(place["boundingbox"][2].get<std::string>());
        bbox.east = std::stod(place["boundingbox"][3].get<std::string>());

        return bbox;
    }
    catch (const std::exception& e)
    {
        spdlog::error("Failed to parse bounding box: {}", e.what());
        return std::nullopt;
    }
}

std::optional<BoundingBox> geocodeCity(const std::string& cityName)
{
    auto response = cpr::Get(
        cpr::Url{ "https://nominatim.openstreetmap.org/search" },
        cpr::Parameters{
            {"q", cityName},
            {"format", "json"},
            {"limit", "1"},
            {"accept-language", "en"}
        },
        cpr::Header{ {"User-Agent", kUserAgent} }
    );

    if (response.status_code != 200)
    {
        spdlog::error("Geocoding request failed for '{}': HTTP {}", cityName, response.status_code);
        return std::nullopt;
    }

    auto bbox = parseGeocodeResponse(response.text);

    if (!bbox)
    {
        spdlog::error("City not found or unparsable: '{}'", cityName);
    }

    return bbox;
}

std::vector<Point> parseOverpassResponse(const std::string& json)
{
    std::vector<Point> points;

    nlohmann::json parsed;

    try
    {
        parsed = nlohmann::json::parse(json);
    }
    catch (const nlohmann::json::parse_error& e)
    {
        spdlog::error("Failed to parse Overpass response: {}", e.what());
        return points;
    }

    if (!parsed.contains("elements") || !parsed["elements"].is_array())
    {
        return points;
    }

    for (const auto& element : parsed["elements"])
    {
        if (!element.contains("id"))
        {
            continue;
        }

        double lat = 0.0;
        double lon = 0.0;
        bool hasCoords = false;

        if (element.contains("lat") && element.contains("lon"))
        {
            lat = element["lat"].get<double>();
            lon = element["lon"].get<double>();
            hasCoords = true;
        }
        else if (element.contains("center") && element["center"].contains("lat") && element["center"].contains("lon"))
        {
            lat = element["center"]["lat"].get<double>();
            lon = element["center"]["lon"].get<double>();
            hasCoords = true;
        }

        if (!hasCoords)
        {
            continue;
        }

        bool isChain = false;
        std::string brandName;

        if (element.contains("tags"))
        {
            const auto& tags = element["tags"];

            if (tags.contains("brand"))
            {
                isChain = true;

                if (tags["brand"].is_string())
                {
                    brandName = tags["brand"].get<std::string>();
                }
            }
        }

        points.push_back({
            std::to_string(element["id"].get<std::int64_t>()),
            lat,
            lon,
            isChain,
            brandName
            });
    }

    return points;
}

std::optional<std::vector<Point>> fetchOsmPoints(
    const BoundingBox& bbox,
    const std::vector<OsmTag>& tags
)
{
    if (tags.empty())
    {
        return std::vector<Point>{};
    }

    std::this_thread::sleep_for(std::chrono::seconds(4));

    std::string bboxStr =
        std::to_string(bbox.south) + "," +
        std::to_string(bbox.west) + "," +
        std::to_string(bbox.north) + "," +
        std::to_string(bbox.east);

    std::string query = "[out:json][timeout:25];\n(\n";

    for (const auto& tag : tags)
    {
        query += "  nwr[" + toOverpassFilter(tag) + "](" + bboxStr + ");\n";
    }

    query += ");\nout center tags;";

    cpr::Response response;

    for (int attempt = 0; attempt < 2; attempt++)
    {
        response = cpr::Post(
            cpr::Url{ "https://overpass-api.de/api/interpreter" },
            cpr::Payload{ {"data", query} },
            cpr::Header{ {"User-Agent", kUserAgent} },
            cpr::Timeout{ 30000 }
        );

        if (response.status_code == 200)
        {
            break;
        }

        spdlog::warn("Overpass attempt {} failed: HTTP {}", attempt + 1, response.status_code);

        if (attempt == 0)
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }

    if (response.status_code != 200)
    {
        spdlog::error("Overpass request failed after retries: HTTP {}", response.status_code);
        return std::nullopt;
    }

    auto points = parseOverpassResponse(response.text);

    spdlog::info("Fetched {} points from OSM", points.size());

    return points;
}

std::optional<GeocodeResult> parseGeocodeResponseFull(const std::string& json)
{
    nlohmann::json parsed;

    try
    {
        parsed = nlohmann::json::parse(json);
    }
    catch (const nlohmann::json::parse_error& e)
    {
        spdlog::error("Failed to parse geocoding response: {}", e.what());
        return std::nullopt;
    }

    if (!parsed.is_array() || parsed.empty())
    {
        return std::nullopt;
    }

    const auto& place = parsed[0];

    if (!place.contains("boundingbox") || !place["boundingbox"].is_array() || place["boundingbox"].size() != 4)
    {
        spdlog::error("Unexpected geocoding response format");
        return std::nullopt;
    }

    try
    {
        GeocodeResult result;
        result.bbox.south = std::stod(place["boundingbox"][0].get<std::string>());
        result.bbox.north = std::stod(place["boundingbox"][1].get<std::string>());
        result.bbox.west = std::stod(place["boundingbox"][2].get<std::string>());
        result.bbox.east = std::stod(place["boundingbox"][3].get<std::string>());

        if (place.contains("address") && place["address"].contains("country_code"))
        {
            result.countryCode = place["address"]["country_code"].get<std::string>();
        }

        return result;
    }
    catch (const std::exception& e)
    {
        spdlog::error("Failed to parse bounding box: {}", e.what());
        return std::nullopt;
    }
}

std::optional<GeocodeResult> geocodeCityFull(const std::string& cityName)
{
    auto response = cpr::Get(
        cpr::Url{ "https://nominatim.openstreetmap.org/search" },
        cpr::Parameters{
            {"q", cityName},
            {"format", "json"},
            {"limit", "1"},
            {"addressdetails", "1"},
            {"accept-language", "en"}
        },
        cpr::Header{ {"User-Agent", kUserAgent} }
    );

    if (response.status_code != 200)
    {
        spdlog::error("Geocoding request failed for '{}': HTTP {}", cityName, response.status_code);
        return std::nullopt;
    }

    auto result = parseGeocodeResponseFull(response.text);

    if (!result)
    {
        spdlog::error("City not found or unparsable: '{}'", cityName);
    }

    return result;
}