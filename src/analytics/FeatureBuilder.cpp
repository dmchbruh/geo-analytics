#include "analytics/FeatureBuilder.h"
#include "h3/H3Neighbors.h"
#include "h3/HexMetrics.h"

#include <unordered_set>

std::vector<HexFeature> buildFeatures(
    const HexCountMap& hexes
)
{
    std::vector<HexFeature> features;

    features.reserve(hexes.size());

    for (const auto& [hexId, count] : hexes)
    {
        HexFeature feature;

        feature.hexId = hexId;
        feature.pointsCount = count;
        double area = getHexAreaKm2(hexId);

        if (area > 0.0)
        {
            feature.density =
                static_cast<double>(count) / area;
        }

        auto neighbors = getNeighbors(hexId);

        feature.neighborsCount = neighbors.size();

        for (auto neighbor : neighbors)
        {
            auto it = hexes.find(neighbor);

            if (it != hexes.end())
            {
                feature.neighborPointsCount += it->second;
            }
        }

        features.push_back(feature);
    }

    return features;
}

std::vector<HexFeature> buildOpportunityFeatures(
    const HexCountMap& competitorCounts,
    const HexCountMap& demandCounts,
    const HexCountMap& growthCounts
)
{
    std::vector<HexFeature> features;

    std::unordered_set<H3Index> allHexes;

    for (const auto& [hexId, count] : competitorCounts)
    {
        allHexes.insert(hexId);
    }

    for (const auto& [hexId, count] : demandCounts)
    {
        allHexes.insert(hexId);
    }

    for (const auto& [hexId, count] : growthCounts)
    {
        allHexes.insert(hexId);
    }

    for (H3Index hexId : allHexes)
    {
        HexFeature feature;
        feature.hexId = hexId;

        auto competitorIt = competitorCounts.find(hexId);
        feature.pointsCount = competitorIt != competitorCounts.end() ? competitorIt->second : 0;

        auto demandIt = demandCounts.find(hexId);
        feature.demandCount = demandIt != demandCounts.end() ? demandIt->second : 0;

        auto growthIt = growthCounts.find(hexId);
        feature.growthCount = growthIt != growthCounts.end() ? growthIt->second : 0;

        features.push_back(feature);
    }

    return features;
}