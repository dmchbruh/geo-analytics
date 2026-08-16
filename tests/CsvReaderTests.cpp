#include <cstdio>
#include <fstream>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include "io/CsvReader.h"

namespace
{
    std::string writeTempCsv(const std::string& content)
    {
        std::string path = "test_points_temp.csv";

        std::ofstream file(path);

        file << content;

        return path;
    }
}

TEST_CASE("readCsv parses valid rows")
{
    auto path = writeTempCsv(
        "id,lat,lon\n"
        "p1,37.775938,-122.419281\n"
        "p2,40.689167,-74.044444\n"
    );

    auto points = readCsv(path);

    REQUIRE(points.size() == 2);
    REQUIRE(points[0].id == "p1");
    REQUIRE(points[1].id == "p2");

    std::remove(path.c_str());
}

TEST_CASE("readCsv skips rows with missing fields")
{
    auto path = writeTempCsv(
        "id,lat,lon\n"
        "p1,37.775938,-122.419281\n"
        "p2,,-74.044444\n"
        "p3,40.689167,\n"
    );

    auto points = readCsv(path);

    REQUIRE(points.size() == 1);
    REQUIRE(points[0].id == "p1");

    std::remove(path.c_str());
}

TEST_CASE("readCsv skips rows with non-numeric coordinates")
{
    auto path = writeTempCsv(
        "id,lat,lon\n"
        "p1,37.775938,-122.419281\n"
        "p2,abc,-74.044444\n"
    );

    auto points = readCsv(path);

    REQUIRE(points.size() == 1);
    REQUIRE(points[0].id == "p1");

    std::remove(path.c_str());
}

TEST_CASE("readCsv skips rows with out-of-range coordinates")
{
    auto path = writeTempCsv(
        "id,lat,lon\n"
        "p1,37.775938,-122.419281\n"
        "p2,120.0,-74.044444\n"
        "p3,40.689167,-200.0\n"
    );

    auto points = readCsv(path);

    REQUIRE(points.size() == 1);
    REQUIRE(points[0].id == "p1");

    std::remove(path.c_str());
}

TEST_CASE("readCsv returns empty vector for missing file")
{
    auto points = readCsv("this_file_does_not_exist.csv");

    REQUIRE(points.empty());
}