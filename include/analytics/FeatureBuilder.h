#pragma once

#include "analytics/HexAggregator.h"
#include "analytics/HexFeature.h"

#include <vector>

std::vector<HexFeature> buildFeatures(
    const HexCountMap& hexes
);

std::vector<HexFeature> buildOpportunityFeatures(
    const HexCountMap& competitorCounts,
    const HexCountMap& demandCounts,
    const HexCountMap& growthCounts,
    const HexCompetitorMap& competitorDetails
);