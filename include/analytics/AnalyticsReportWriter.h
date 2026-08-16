#pragma once

#include "analytics/HexFeature.h"

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

nlohmann::json buildSummaryReport(
    const std::vector<HexFeature>& features,
    std::size_t topZonesCount = 10
);

void writeSummaryReport(
    const std::string& path,
    const std::vector<HexFeature>& features,
    std::size_t topZonesCount = 10
);