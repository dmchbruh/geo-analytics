#pragma once

#include "io/CsvReader.h"
#include <h3/h3api.h>
#include <string>
#include <vector>

struct H3Point
{
    std::string id;
    double lat;
    double lon;
    H3Index h3Index;
    bool isChain = false;
    std::string brandName;
};

std::vector<H3Point> convertToH3(
    const std::vector<Point>& points,
    int resolution
);