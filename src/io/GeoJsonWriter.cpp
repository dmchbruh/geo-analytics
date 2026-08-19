#include "io/GeoJsonWriter.h"

#include "h3/HexGeometry.h"
#include "io/FileUtils.h"

#include <h3/h3api.h>

nlohmann::json buildGeoJson(const std::vector<HexFeature>& features)
{
    nlohmann::json geoJson;

    geoJson["type"] = "FeatureCollection";
    geoJson["features"] = nlohmann::json::array();

    for (const auto& feature : features)
    {
        auto boundary = getHexBoundary(feature.hexId);

        nlohmann::json polygon = nlohmann::json::array();

        for (const auto& point : boundary)
        {
            polygon.push_back({
                point.lon,
                point.lat
                });
        }

        char h3String[17];

        h3ToString(feature.hexId, h3String, sizeof(h3String));

        nlohmann::json properties =
        {
            {"hex_id", h3String},
            {"rank", feature.rank},
            {"points_count", feature.pointsCount},
            {"neighbors_count", feature.neighborsCount},
            {"neighbor_points_count", feature.neighborPointsCount},
            {"demand_count", feature.demandCount},
            {"score", feature.score},
            {"confidence", feature.confidence},
            {"category", feature.category},
            {"growth_count", feature.growthCount},
            {"investment_score", feature.investmentScore},
            {"chain_count", feature.chainCount},
            {"independent_count", feature.independentCount},
            {"chain_names", feature.chainNames},
            {
    "style",
    {
        {
            "fill",
            feature.category == "LOW" ? "#ff0000" :
            feature.category == "MEDIUM" ? "#ffff00" :
            feature.category == "HIGH" ? "#90ee90" :
            "#00aa00"
        },
        {
            "fill-opacity",
            0.6
        },
        {
            "stroke",
            "#333333"
        },
        {
            "stroke-width",
            1
        }
    }
}
        };

        geoJson["features"].push_back(
            {
                {"type", "Feature"},
                {
                    "geometry",
                    {
                        {"type", "Polygon"},
                        {
                            "coordinates",
                            {
                                polygon
                            }
                        }
                    }
                },
                {
                    "properties",
                    properties
                }
            }
        );
    }

    return geoJson;
}

void writeGeoJson(
    const std::string& path,
    const std::vector<HexFeature>& features
)
{
    auto geoJson = buildGeoJson(features);

    std::ofstream file = openOutputFile(path);

    if (!file.is_open())
    {
        return;
    }

    file << geoJson.dump(4);
}