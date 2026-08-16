cmake_minimum_required(VERSION 3.21)

project(GeoAnalytics LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(h3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED)

add_library(GeoAnalyticsCore STATIC
    src/ProjectPaths.cpp
    src/io/CsvReader.cpp
    src/io/CsvWriter.cpp
    src/h3/H3Processor.cpp
    src/config/AppConfig.cpp
    src/analytics/HexAggregator.cpp
    src/analytics/HexFeature.cpp
    src/analytics/FeatureBuilder.cpp
    src/analytics/ScoringEngine.cpp
    src/analytics/AnalyticsReportWriter.cpp
    src/io/FeatureWriter.cpp
    src/io/GeoJsonWriter.cpp
    src/h3/HexGeometry.cpp
    src/io/JsonWriter.cpp
    src/h3/H3Neighbors.cpp
    src/h3/HexMetrics.cpp
    src/io/FileUtils.cpp
    src/db/Database.cpp
    src/geo/OsmDataSource.cpp
    src/config/BusinessCategories.cpp
    src/server/ApiServer.cpp 
    src/geo/PbfDataSource.cpp
    src/util/Logging.cpp)

target_include_directories(GeoAnalyticsCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(GeoAnalyticsCore
    PUBLIC
        h3::h3
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        unofficial::sqlite3::sqlite3
        cpr::cpr
        httplib::httplib
)

add_executable(${PROJECT_NAME}
    src/main.cpp
     )

    add_executable(GeoAnalyticsServer
    src/server_main.cpp
 )

target_link_libraries(GeoAnalyticsServer
    PRIVATE
        GeoAnalyticsCore
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        GeoAnalyticsCore
)

option(GEOANALYTICS_BUILD_TESTS "Build unit tests" ON)

if(GEOANALYTICS_BUILD_TESTS)
    find_package(Catch2 3 CONFIG REQUIRED)

    add_executable(GeoAnalyticsTests
        tests/ScoringEngineTests.cpp
        tests/H3NeighborsTests.cpp
        tests/CsvReaderTests.cpp
        tests/AppConfigTests.cpp
        tests/HexAggregatorTests.cpp
        tests/FeatureBuilderTests.cpp
        tests/AnalyticsReportWriterTests.cpp
        tests/HexGeometryTests.cpp
        tests/HexMetricsTests.cpp
        tests/FeatureWriterTests.cpp
        tests/JsonWriterTests.cpp
        tests/GeoJsonWriterTests.cpp
        tests/CsvWriterTests.cpp
        tests/DatabaseTests.cpp
        tests/OsmDataSourceTests.cpp
        tests/BusinessCategoriesTests.cpp )

    target_link_libraries(GeoAnalyticsTests
        PRIVATE
            GeoAnalyticsCore
            Catch2::Catch2WithMain
    )

    include(CTest)
    include(Catch)
    catch_discover_tests(GeoAnalyticsTests)
endif()