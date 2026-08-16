#include "analytics/ScoringEngine.h"
#include <spdlog/spdlog.h>

#include <algorithm>
#include <string>

namespace
{
    std::string getCategory(
        double score,
        double lowThreshold,
        double mediumThreshold,
        double highThreshold
    )
    {
        if (score < lowThreshold)
            return "LOW";

        if (score < mediumThreshold)
            return "MEDIUM";

        if (score < highThreshold)
            return "HIGH";

        return "VERY_HIGH";
    }
}

void calculateScores(
    std::vector<HexFeature>& features,
    double pointWeight,
    double neighborWeight,
    double densityWeight,
    double lowThreshold,
    double mediumThreshold,
    double highThreshold
)
{
    double weightSum =
        pointWeight +
        neighborWeight +
        densityWeight;

    if (weightSum <= 0.0)
    {
        spdlog::warn("All scoring weights are zero or negative, skipping score calculation");
        return;
    }

    pointWeight /= weightSum;
    neighborWeight /= weightSum;
    densityWeight /= weightSum;

    std::size_t maxPoints = 0;
    double maxDensity = 0.0;

    for (const auto& feature : features)
    {
        maxPoints = std::max(
            maxPoints,
            feature.pointsCount
        );

        maxDensity = std::max(
            maxDensity,
            feature.density
        );
    }

    for (auto& feature : features)
    {
        double pointScore = 0.0;

        if (maxPoints > 0)
        {
            pointScore =
                static_cast<double>(feature.pointsCount) /
                static_cast<double>(maxPoints) *
                pointWeight *
                100.0;
        }

        double neighborPointScore = 0.0;

        if (maxPoints > 0)
        {
            neighborPointScore =
                static_cast<double>(feature.neighborPointsCount) /
                static_cast<double>(maxPoints) *
                neighborWeight *
                100.0;
        }

        double densityScore = 0.0;

        if (maxDensity > 0.0)
        {
            densityScore =
                feature.density /
                maxDensity *
                100.0;
        }

        feature.score = std::min(
            pointScore +
            neighborPointScore +
            densityScore * densityWeight,
            100.0
        );

        feature.category = getCategory(
            feature.score,
            lowThreshold,
            mediumThreshold,
            highThreshold
        );
    }

    for (auto& feature : features)
    {
        double confidencePoints =
            static_cast<double>(feature.pointsCount);

        feature.confidence =
            std::min(
                confidencePoints / 100.0,
                1.0
            );
    }

    std::sort(
        features.begin(),
        features.end(),
        [](const HexFeature& a, const HexFeature& b)
        {
            return a.score > b.score;
        }
    );

    int rank = 1;

    for (auto& feature : features)
    {
        feature.rank = rank++;
    }



}

void calculateOpportunityScores(
    std::vector<HexFeature>& features,
    double competitorWeight,
    double demandWeight
)
{
    double weightSum = competitorWeight + demandWeight;

    if (weightSum <= 0.0)
    {
        spdlog::warn("All scoring weights are zero or negative, skipping score calculation");
        return;
    }

    competitorWeight /= weightSum;
    demandWeight /= weightSum;

    std::vector<std::size_t> competitorCounts;
    std::vector<std::size_t> demandCounts;

    competitorCounts.reserve(features.size());
    demandCounts.reserve(features.size());

    for (const auto& feature : features)
    {
        competitorCounts.push_back(feature.pointsCount);
        demandCounts.push_back(feature.demandCount);
    }

    std::sort(competitorCounts.begin(), competitorCounts.end());
    std::sort(demandCounts.begin(), demandCounts.end());

    auto percentileValue = [](const std::vector<std::size_t>& sorted, double percentile) -> std::size_t
        {
            if (sorted.empty())
            {
                return 0;
            }

            std::size_t index = static_cast<std::size_t>(percentile * static_cast<double>(sorted.size() - 1));

            return sorted[index];
        };

    std::size_t maxCompetitors = percentileValue(competitorCounts, 0.9);
    std::size_t maxDemand = percentileValue(demandCounts, 0.9);

    for (auto& feature : features)
    {
        double demandRatio = 0.0;

        if (maxDemand > 0)
        {
            demandRatio = std::min(
                1.0,
                static_cast<double>(feature.demandCount) / static_cast<double>(maxDemand)
            );
        }

        double competitorRatio = 0.0;

        if (maxCompetitors > 0)
        {
            competitorRatio = std::min(
                1.0,
                static_cast<double>(feature.pointsCount) / static_cast<double>(maxCompetitors)
            );
        }

        double demandScore = demandRatio * demandWeight * 100.0;
        double competitionPenalty = competitorRatio * competitorWeight * 100.0;

        feature.score = std::clamp(
            demandScore - competitionPenalty,
            0.0,
            100.0
        );
    }

    std::vector<std::size_t> growthCounts;
    growthCounts.reserve(features.size());

    for (const auto& feature : features)
    {
        growthCounts.push_back(feature.growthCount);
    }

    std::sort(growthCounts.begin(), growthCounts.end());

    std::size_t maxGrowth = percentileValue(growthCounts, 0.9);

    for (auto& feature : features)
    {
        feature.confidence = std::min(
            static_cast<double>(feature.demandCount) / 100.0,
            1.0
        );

        double growthRatio = 0.0;

        if (maxGrowth > 0)
        {
            growthRatio = std::min(
                1.0,
                static_cast<double>(feature.growthCount) / static_cast<double>(maxGrowth)
            );
        }

        feature.investmentScore = growthRatio * 100.0;
    }

    std::sort(
        features.begin(),
        features.end(),
        [](const HexFeature& a, const HexFeature& b)
        {
            return a.score > b.score;
        }
    );

    std::size_t total = features.size();

    for (std::size_t i = 0; i < total; i++)
    {
        features[i].rank = static_cast<int>(i) + 1;

        double percentileRank = total > 1
            ? static_cast<double>(i) / static_cast<double>(total - 1)
            : 0.0;

        if (percentileRank <= 0.10)
        {
            features[i].category = "VERY_HIGH";
        }
        else if (percentileRank <= 0.30)
        {
            features[i].category = "HIGH";
        }
        else if (percentileRank <= 0.70)
        {
            features[i].category = "MEDIUM";
        }
        else
        {
            features[i].category = "LOW";
        }
    }
}