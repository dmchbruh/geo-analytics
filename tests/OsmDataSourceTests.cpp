#include <catch2/catch_test_macros.hpp>

#include "geo/OsmDataSource.h"

TEST_CASE("parseGeocodeResponse extracts bounding box")
{
    std::string json = R"([{"boundingbox":["41.6002","41.8802","44.6772","44.9532"],"lat":"41.7151377","lon":"44.827096"}])";

    auto bbox = parseGeocodeResponse(json);

    REQUIRE(bbox.has_value());
    REQUIRE(bbox->south == 41.6002);
    REQUIRE(bbox->north == 41.8802);
    REQUIRE(bbox->west == 44.6772);
    REQUIRE(bbox->east == 44.9532);
}

TEST_CASE("parseGeocodeResponse returns nullopt for empty results")
{
    auto bbox = parseGeocodeResponse("[]");

    REQUIRE_FALSE(bbox.has_value());
}

TEST_CASE("parseGeocodeResponse returns nullopt for invalid json")
{
    auto bbox = parseGeocodeResponse("not json");

    REQUIRE_FALSE(bbox.has_value());
}

TEST_CASE("parseOverpassResponse extracts node points")
{
    std::string json = R"({"elements":[{"type":"node","id":123456,"lat":41.71,"lon":44.82},{"type":"node","id":654321,"lat":41.72,"lon":44.83}]})";

    auto points = parseOverpassResponse(json);

    REQUIRE(points.size() == 2);
    REQUIRE(points[0].id == "123456");
    REQUIRE(points[0].lat == 41.71);
}

TEST_CASE("parseOverpassResponse extracts way centroids")
{
    std::string json = R"({"elements":[{"type":"way","id":999,"center":{"lat":41.70,"lon":44.80}}]})";

    auto points = parseOverpassResponse(json);

    REQUIRE(points.size() == 1);
    REQUIRE(points[0].id == "999");
    REQUIRE(points[0].lat == 41.70);
}

TEST_CASE("parseOverpassResponse handles empty elements")
{
    auto points = parseOverpassResponse(R"({"elements":[]})");

    REQUIRE(points.empty());
}

TEST_CASE("parseOverpassResponse handles missing elements key")
{
    auto points = parseOverpassResponse(R"({"foo":"bar"})");

    REQUIRE(points.empty());
}