#include "h3/H3Processor.h"

#include <h3/h3api.h>

#include <spdlog/spdlog.h>

std::vector<H3Point> convertToH3(
    const std::vector<Point>& points,
    int resolution
)
{
    std::vector<H3Point> result;

    result.reserve(points.size());

    std::size_t skippedCount = 0;

    for (const auto& point : points)
    {
        LatLng coord{};

        coord.lat = degsToRads(point.lat);
        coord.lng = degsToRads(point.lon);

        H3Index cell{};

        H3Error error = latLngToCell(&coord, resolution, &cell);

        if (error != E_SUCCESS)
        {
            spdlog::warn("Failed to convert point '{}' to H3 cell: error code {}", point.id, static_cast<int>(error));
            skippedCount++;
            continue;
        }

        result.push_back({
             point.id,
             point.lat,
             point.lon,
             cell,
             point.isChain,
             point.brandName
            });
    }

    if (skippedCount > 0)
    {
        spdlog::warn("Total points skipped due to H3 conversion errors: {}", skippedCount);
    }

    return result;
}