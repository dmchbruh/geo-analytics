#pragma once

#include "config/BusinessCategories.h"
#include "geo/OsmDataSource.h"
#include "io/CsvReader.h"

#include <optional>
#include <string>
#include <vector>

std::vector<Point> parseOsmiumGeoJson(const std::string& json);

std::optional<std::vector<Point>> fetchPbfPoints(
    const std::string& pbfPath,
    const BoundingBox& bbox,
    const std::vector<OsmTag>& tags
);