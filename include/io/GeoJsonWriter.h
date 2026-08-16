#pragma once

#include "analytics/HexFeature.h"

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

nlohmann::json buildGeoJson(const std::vector<HexFeature>& features);

void writeGeoJson(
    const std::string& path,
    const std::vector<HexFeature>& features
);