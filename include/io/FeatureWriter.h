#pragma once

#include "analytics/HexFeature.h"

#include <string>
#include <vector>

void writeFeaturesCsv(
    const std::string& path,
    const std::vector<HexFeature>& features
);