#include "ProjectPaths.h"
#include "util/Logging.h"
#include "analytics/HexAggregator.h"
#include "config/AppConfig.h"
#include "h3/H3Processor.h"
#include "io/CsvReader.h"
#include "io/CsvWriter.h"
#include "analytics/FeatureBuilder.h"
#include <spdlog/spdlog.h>
#include "analytics/ScoringEngine.h"
#include "io/FeatureWriter.h"
#include "io/JsonWriter.h"
#include "io/GeoJsonWriter.h"
#include "analytics/AnalyticsReportWriter.h"
#include "db/Database.h"
#include "geo/OsmDataSource.h"
#include "config/BusinessCategories.h"
#include <iomanip>
#include <optional>

int runCli(int argc, char* argv[])
{
    std::optional<std::string> inputOverride;
    std::optional<int> resolutionOverride;
    std::optional<std::string> cityArg;
    std::optional<std::string> businessArg;

    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "--input" && i + 1 < argc)
        {
            inputOverride = argv[++i];
        }
        else if (arg == "--resolution" && i + 1 < argc)
        {
            std::string value = argv[++i];

            try
            {
                resolutionOverride = std::stoi(value);
            }
            catch (const std::exception&)
            {
                spdlog::error("Invalid --resolution value: '{}', ignoring", value);
            }
        }
        else if (arg == "--city" && i + 1 < argc)
        {
            cityArg = argv[++i];
        }
        else if (arg == "--business" && i + 1 < argc)
        {
            businessArg = argv[++i];
        }
    }

    auto config = loadConfig();

    Database database(config.databaseFile);

    spdlog::info(
        "Scoring weights | points: {} | neighbors: {} | density: {}",
        config.pointWeight,
        config.neighborWeight,
        config.densityWeight
    );

    if (inputOverride)
    {
        config.inputFile = *inputOverride;
    }

    if (resolutionOverride)
    {
        if (*resolutionOverride < 0 || *resolutionOverride > 15)
        {
            spdlog::error("Invalid --resolution {}, must be 0-15, keeping config value {}", *resolutionOverride, config.resolution);
        }
        else
        {
            config.resolution = *resolutionOverride;
        }
    }

    spdlog::info(
        "Category thresholds | LOW: {} | MEDIUM: {} | HIGH: {}",
        config.lowThreshold,
        config.mediumThreshold,
        config.highThreshold
    );

    std::vector<Point> points;
    std::string inputDescription = config.inputFile.string();
    std::vector<HexFeature> features;

    if (cityArg && businessArg)
    {
        auto category = findBusinessCategory(*businessArg);

        if (!category)
        {
            spdlog::error("Unknown business category: '{}'", *businessArg);
            return 1;
        }

        auto bbox = geocodeCity(*cityArg);

        if (!bbox)
        {
            spdlog::error("Could not geocode city: '{}'", *cityArg);
            return 1;
        }

        spdlog::info("Fetching '{}' locations in '{}' from OpenStreetMap", category->name, *cityArg);

        auto competitorResult = fetchOsmPoints(*bbox, category->competitorTags);
        auto demandResult = fetchOsmPoints(*bbox, category->demandTags);

        if (!competitorResult || !demandResult)
        {
            spdlog::error("Failed to fetch data from OpenStreetMap, aborting");
            return 1;
        }

        points = *competitorResult;
        inputDescription = *cityArg + " / " + *businessArg;

        auto demandPoints = *demandResult;

        spdlog::info("Demand points fetched: {}", demandPoints.size());

        auto competitorH3Points = convertToH3(points, config.resolution);
        auto demandH3Points = convertToH3(demandPoints, config.resolution);

        auto competitorHexes = aggregateHexes(competitorH3Points);
        auto demandHexes = aggregateHexes(demandH3Points);

        features = buildOpportunityFeatures(competitorHexes, demandHexes);

        calculateOpportunityScores(
            features,
            0.3,
            0.7
        );
    }
    else
    {
        points = readCsv(config.inputFile.string());

        auto h3Points = convertToH3(points, config.resolution);
        auto hexes = aggregateHexes(h3Points);

        features = buildFeatures(hexes);

        calculateScores(
            features,
            config.pointWeight,
            config.neighborWeight,
            config.densityWeight,
            config.lowThreshold,
            config.mediumThreshold,
            config.highThreshold
        );
    }

    std::int64_t runId = database.insertRun(
        inputDescription,
        config.resolution
    );

    database.insertPoints(runId, points);

    spdlog::info("Points read: {}", points.size());

    database.insertFeatures(runId, features);

    spdlog::info("Features created: {}", features.size());

    spdlog::info("Top 10 hexes:");

    std::size_t topCount =
        std::min<std::size_t>(
            10,
            features.size()
        );

    for (std::size_t i = 0; i < topCount; i++)
    {
        const auto& feature = features[i];

        spdlog::info(
            "Rank: {} | Hex: {} | points: {} | score: {:.2f} | confidence: {:.2f} | category: '{}'",
            feature.rank,
            feature.hexId,
            feature.pointsCount,
            feature.score,
            feature.confidence,
            feature.category
        );
    }

    writeFeaturesCsv(
        config.featuresOutputFile.string(),
        features
    );

    spdlog::info(
        "Features saved in {}",
        config.featuresOutputFile.string()
    );

    writeCsv(config.outputFile.string(), convertToH3(points, config.resolution));

    writeFeaturesJson(
        config.featuresJsonOutputFile.string(),
        features
    );

    writeGeoJson(
        config.geoJsonOutputFile.string(),
        features
    );

    writeSummaryReport(
        config.summaryOutputFile.string(),
        features
    );

    spdlog::info(
        "Summary report saved in {}",
        config.summaryOutputFile.string()
    );

    spdlog::info("GeoJSON saved");

    spdlog::info(
        "JSON saved in {}",
        config.featuresJsonOutputFile.string()
    );

    spdlog::info(
        "Current path: {}",
        std::filesystem::current_path().string()
    );

    return 0;
}

int main(int argc, char* argv[])
{
    auto logPath = (findProjectRoot() / "logs" / "cli.log").string();
    initLogging(logPath);
    installCrashHandler();

    int exitCode = 0;

    try
    {
        exitCode = runCli(argc, argv);
    }
    catch (const std::exception& e)
    {
        exitCode = 1;
        spdlog::critical("CLI failed: {}", e.what());
    }
    catch (...)
    {
        exitCode = 1;
        spdlog::critical("CLI failed with unknown error");
    }

    waitForExitPrompt(exitCode);
    return exitCode;
}