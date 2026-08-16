#include "h3/H3Neighbors.h"

#include <h3/h3api.h>

std::vector<H3Index> getNeighbors(H3Index hexId)
{
    H3Index neighbors[7]{};

    gridDisk(hexId, 1, neighbors);

    std::vector<H3Index> result;

    result.reserve(6);

    for (H3Index neighbor : neighbors)
    {
        if (neighbor != 0 && neighbor != hexId)
        {
            result.push_back(neighbor);
        }
    }

    return result;
}