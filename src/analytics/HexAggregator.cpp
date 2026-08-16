#include "analytics/HexAggregator.h"

HexCountMap aggregateHexes(const std::vector<H3Point>& points)
{
    HexCountMap result;

    for (const auto& point : points)
    {
        ++result[point.h3Index];
    }

    return result;
}