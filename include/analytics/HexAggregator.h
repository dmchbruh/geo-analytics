#pragma once

#include "h3/H3Processor.h"

#include <string>
#include <unordered_map>

using HexCountMap = std::unordered_map<H3Index, std::size_t>;

HexCountMap aggregateHexes(const std::vector<H3Point>& points);