#pragma once

#include <string>
#include <vector>

struct Point
{
    std::string id;
    double lat;
    double lon;
    bool isChain = false;
    std::string brandName;
};

std::vector<Point> readCsv(const std::string& path);