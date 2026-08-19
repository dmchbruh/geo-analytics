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

HexCompetitorMap aggregateCompetitors(const std::vector<H3Point>& points)
{
    HexCompetitorMap result;

    for (const auto& point : points)
    {
        auto& data = result[point.h3Index];

        if (point.isChain)
        {
            data.chainCount++;

            if (!point.brandName.empty())
            {
                bool alreadyAdded = false;

                for (const auto& name : data.chainNames)
                {
                    if (name == point.brandName)
                    {
                        alreadyAdded = true;
                        break;
                    }
                }

                if (!alreadyAdded)
                {
                    data.chainNames.push_back(point.brandName);
                }
            }
        }
        else
        {
            data.independentCount++;
        }
    }

    return result;
}