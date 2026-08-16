#pragma once

#include "config/BusinessCategories.h"
#include "io/CsvReader.h"

#include <optional>
#include <string>
#include <vector>

struct BoundingBox
{
    double south;
    double west;
    double north;
    double east;
};

struct GeocodeResult
{
    BoundingBox bbox;
    std::string countryCode;
};

std::optional<BoundingBox> parseGeocodeResponse(const std::string& json);

std::optional<GeocodeResult> parseGeocodeResponseFull(const std::string& json);

std::optional<BoundingBox> geocodeCity(const std::string& cityName);

std::optional<GeocodeResult> geocodeCityFull(const std::string& cityName);

std::vector<Point> parseOverpassResponse(const std::string& json);

std::optional<std::vector<Point>> fetchOsmPoints(
    const BoundingBox& bbox,
    const std::vector<OsmTag>& tags
);