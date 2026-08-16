#pragma once

#include <string>
#include <vector>

struct Point
{
    std::string id;
    double lat;
    double lon;
};

std::vector<Point> readCsv(const std::string& path);