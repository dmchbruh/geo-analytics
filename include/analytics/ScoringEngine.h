#pragma once

#include "analytics/HexFeature.h"

#include <vector>

void calculateScores(
    std::vector<HexFeature>& features,
    double pointWeight,
    double neighborWeight,
    double densityWeight,
    double lowThreshold,
    double mediumThreshold,
    double highThreshold
);
void calculateOpportunityScores(
    std::vector<HexFeature>& features,
    double competitorWeight,
    double demandWeight
);