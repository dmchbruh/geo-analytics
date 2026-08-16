#pragma once

#include <h3/h3api.h>

#include <vector>

struct GeoPoint
{
    double lat;
    double lon;
};

std::vector<GeoPoint> getHexBoundary(H3Index hexId);