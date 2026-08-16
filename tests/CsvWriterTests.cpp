#include <cstdio>
#include <fstream>

#include <catch2/catch_test_macros.hpp>

#include "io/CsvWriter.h"

TEST_CASE("writeCsv writes header and h3_index as hex string")
{
    H3Point point;
    point.id = "p1";
    point.lat = 37.775938;
    point.lon = -122.419281;
    point.h3Index = 0;

    std::vector<H3Point> points = { point };

    std::string path = "test_csv_writer_temp.csv";

    writeCsv(path, points);

    std::ifstream file(path);

    std::string headerLine;
    std::getline(file, headerLine);

    std::string dataLine;
    std::getline(file, dataLine);

    REQUIRE(headerLine.find("h3_index") != std::string::npos);
    REQUIRE(dataLine.find("p1") != std::string::npos);

    std::remove(path.c_str());
}