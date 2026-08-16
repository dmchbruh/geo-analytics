#pragma once

#include "h3/H3Processor.h"

#include <string>
#include <vector>

void writeCsv(
    const std::string& path,
    const std::vector<H3Point>& points
);