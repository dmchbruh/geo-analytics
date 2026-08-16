#include <cstdio>
#include <fstream>

#include <catch2/catch_test_macros.hpp>

#include <h3/h3api.h>
#include <nlohmann/json.hpp>

#include "io/GeoJsonWriter.h"

namespace
{
    H3Index makeTestHex()
    {
        LatLng coord{};

        coord.lat = degsToRads(37.775938);
        coord.lng = degsToRads(-122.419281);

        H3Index cell{};

        latLngToCell(&coord, 9, &cell);

        return cell;
    }
}

TEST_CASE("writeGeoJson writes valid FeatureCollection")
{
    HexFeature feature;
    feature.hexId = makeTestHex();
    feature.pointsCount = 5;
    feature.rank = 1;
    feature.category = "HIGH";

    std::vector<HexFeature> features = { feature };

    std::string path = "test_geojson_temp.geojson";

    writeGeoJson(path, features);

    std::ifstream file(path);
    nlohmann::json output;
    file >> output;

    REQUIRE(output["type"] == "FeatureCollection");
    REQUIRE(output["features"].size() == 1);
    REQUIRE(output["features"][0]["geometry"]["type"] == "Polygon");
    REQUIRE(output["features"][0]["properties"]["hex_id"].is_string());
    REQUIRE(output["features"][0]["properties"]["rank"] == 1);

    std::remove(path.c_str());
}