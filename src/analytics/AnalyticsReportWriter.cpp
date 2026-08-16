#include "analytics/AnalyticsReportWriter.h"
#include "io/FileUtils.h"

#include <algorithm>
#include <map>

#include <h3/h3api.h>

namespace
{
    std::string hexIdToString(H3Index hexId)
    {
        char buffer[17];

        h3ToString(hexId, buffer, sizeof(buffer));

        return std::string(buffer);
    }
}

nlohmann::json buildSummaryReport(
    const std::vector<HexFeature>& features,
    std::size_t topZonesCount
)
{
    nlohmann::json report;

    report["hex_count"] = features.size();

    std::map<std::string, std::size_t> categoryDistribution;

    double densitySum = 0.0;
    double scoreSum = 0.0;
    double confidenceSum = 0.0;
    std::size_t pointsSum = 0;

    for (const auto& feature : features)
    {
        categoryDistribution[feature.category]++;

        densitySum += feature.density;
        scoreSum += feature.score;
        confidenceSum += feature.confidence;
        pointsSum += feature.pointsCount;
    }

    report["category_distribution"] = categoryDistribution;

    std::size_t count = features.size();

    nlohmann::json averages =
    {
        {"density", count > 0 ? densitySum / static_cast<double>(count) : 0.0},
        {"score", count > 0 ? scoreSum / static_cast<double>(count) : 0.0},
        {"confidence", count > 0 ? confidenceSum / static_cast<double>(count) : 0.0},
        {"points_count", count > 0 ? static_cast<double>(pointsSum) / static_cast<double>(count) : 0.0}
    };

    report["averages"] = averages;

    nlohmann::json topZones = nlohmann::json::array();

    std::size_t limit = std::min(topZonesCount, features.size());

    for (std::size_t i = 0; i < limit; i++)
    {
        const auto& feature = features[i];

        topZones.push_back({
            {"rank", feature.rank},
            {"hex_id", hexIdToString(feature.hexId)},
            {"score", feature.score},
            {"category", feature.category},
            {"points_count", feature.pointsCount}
            });
    }

    report["top_zones"] = topZones;

    return report;
}

void writeSummaryReport(
    const std::string& path,
    const std::vector<HexFeature>& features,
    std::size_t topZonesCount
)
{
    auto report = buildSummaryReport(features, topZonesCount);

    std::ofstream file = openOutputFile(path);

    if (!file.is_open())
    {
        return;
    }

    file << report.dump(4);
}