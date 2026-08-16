#include "h3/HexGeometry.h"

#include <h3/h3api.h>

std::vector<GeoPoint> getHexBoundary(H3Index hexId)
{
    std::vector<GeoPoint> boundary;

    if (!isValidCell(hexId))
    {
        return boundary;
    }

    CellBoundary cellBoundary{};

    H3Error error = cellToBoundary(hexId, &cellBoundary);

    if (error != E_SUCCESS)
    {
        return boundary;
    }

    for (int i = 0; i < cellBoundary.numVerts; ++i)
    {
        boundary.push_back({
            radsToDegs(cellBoundary.verts[i].lat),
            radsToDegs(cellBoundary.verts[i].lng)
            });
    }

    if (!boundary.empty())
    {
        boundary.push_back(boundary.front());
    }

    return boundary;
}