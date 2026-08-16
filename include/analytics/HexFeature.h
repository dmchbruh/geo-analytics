#pragma once

#include <h3/h3api.h>

#include <cstddef>
#include <string>

struct HexFeature
{
    H3Index hexId;

    std::size_t pointsCount = 0;

    std::size_t neighborsCount = 0;

    std::size_t neighborPointsCount = 0;

    std::size_t demandCount = 0;

    std::size_t growthCount = 0;

    double investmentScore = 0.0;

    double density = 0.0;

    double score = 0.0;

    double confidence = 0.0;

    int rank = 0;

    std::string category;
};