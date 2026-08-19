#pragma once

#include "h3/H3Processor.h"

#include <string>
#include <unordered_map>

using HexCountMap = std::unordered_map<H3Index, std::size_t>;

struct HexCompetitorData
{
    std::size_t chainCount = 0;
    std::size_t independentCount = 0;
    std::vector<std::string> chainNames;
};

using HexCompetitorMap = std::unordered_map<H3Index, HexCompetitorData>;

HexCompetitorMap aggregateCompetitors(const std::vector<H3Point>& points);

using HexLandmarkMap = std::unordered_map<H3Index, std::vector<std::string>>;

HexLandmarkMap aggregateLandmarks(const std::vector<H3Point>& points);

HexCountMap aggregateHexes(const std::vector<H3Point>& points);

using HexLandmarkMap = std::unordered_map<H3Index, std::vector<std::string>>;

HexLandmarkMap aggregateLandmarks(const std::vector<H3Point>& points);