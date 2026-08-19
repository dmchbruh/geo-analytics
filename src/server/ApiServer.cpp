#include "server/ApiServer.h"

#include "ProjectPaths.h"
#include "config/BusinessCategories.h"
#include "geo/OsmDataSource.h"
#include "geo/PbfDataSource.h"
#include "h3/H3Processor.h"
#include "analytics/HexAggregator.h"
#include "analytics/FeatureBuilder.h"
#include "analytics/ScoringEngine.h"
#include "io/GeoJsonWriter.h"
#include "analytics/AnalyticsReportWriter.h"

#include <fstream>
#include <map>
#include <sstream>

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace
{
    const std::vector<OsmTag> kGrowthTags = {
        {"landuse", "construction"},
        {"building", "construction"}
    };

    struct PbfSource
    {
        std::string path;
        std::string countryCode;
    };

    std::map<std::string, PbfSource> loadPbfSources()
    {
        std::map<std::string, PbfSource> sources;

        auto path = findProjectRoot() / "config" / "pbf_sources.json";

        std::ifstream file(path);

        if (!file.is_open())
        {
            return sources;
        }

        try
        {
            nlohmann::json json;
            file >> json;

            for (auto& [key, value] : json.items())
            {
                PbfSource source;
                source.path = (findProjectRoot() / value.value("path", "")).make_preferred().string();
                source.countryCode = value.value("country_code", "");
                sources[key] = source;
            }
        }
        catch (const std::exception& e)
        {
            spdlog::error("Failed to parse pbf_sources.json: {}", e.what());
        }

        return sources;
    }

    nlohmann::json analyzeCity(
        const std::string& cityName,
        const std::string& businessName,
        int resolution,
        const std::optional<PbfSource>& pbfSource,
        double competitorWeight,
        double demandWeight
    )
    {
        auto category = findBusinessCategory(businessName);

        if (!category)
        {
            return { {"error", "Unknown business category: " + businessName} };
        }

        auto geocodeResult = geocodeCityFull(cityName);

        if (!geocodeResult)
        {
            return { {"error", "Could not geocode city: " + cityName} };
        }

        if (pbfSource && !pbfSource->countryCode.empty() &&
            geocodeResult->countryCode != pbfSource->countryCode)
        {
            return {
                {"error", "City '" + cityName + "' is not in the selected country (detected country code: '" + geocodeResult->countryCode + "')"}
            };
        }

        auto bbox = geocodeResult->bbox;

        std::optional<std::vector<Point>> competitorResult;
        std::optional<std::vector<Point>> demandResult;
        std::optional<std::vector<Point>> growthResult;

        if (pbfSource)
        {
            competitorResult = fetchPbfPoints(pbfSource->path, bbox, category->competitorTags);
            demandResult = fetchPbfPoints(pbfSource->path, bbox, category->demandTags);
            growthResult = fetchPbfPoints(pbfSource->path, bbox, kGrowthTags);
        }
        else
        {
            competitorResult = fetchOsmPoints(bbox, category->competitorTags);
            demandResult = fetchOsmPoints(bbox, category->demandTags);
            growthResult = fetchOsmPoints(bbox, kGrowthTags);
        }

        std::vector<Point> competitorPoints = competitorResult.value_or(std::vector<Point>{});
        std::vector<Point> demandPoints = demandResult.value_or(std::vector<Point>{});
        std::vector<Point> growthPoints = growthResult.value_or(std::vector<Point>{});

        spdlog::info("Converting to H3: {} competitor, {} demand, {} growth points",
            competitorPoints.size(), demandPoints.size(), growthPoints.size());

        auto competitorH3Points = convertToH3(competitorPoints, resolution);
        auto competitorDetails = aggregateCompetitors(competitorH3Points);
        auto demandH3Points = convertToH3(demandPoints, resolution);
        auto growthH3Points = convertToH3(growthPoints, resolution);

        spdlog::info("Aggregating hexes");

        auto competitorHexes = aggregateHexes(competitorH3Points);
        auto demandHexes = aggregateHexes(demandH3Points);
        auto growthHexes = aggregateHexes(growthH3Points);

        spdlog::info("Hex counts: {} competitor, {} demand, {} growth",
            competitorHexes.size(), demandHexes.size(), growthHexes.size());

        auto features = buildOpportunityFeatures(
            competitorHexes,
            demandHexes,
            growthHexes,
            competitorDetails
        );

        spdlog::info("Built {} features, calculating scores", features.size());

        calculateOpportunityScores(
            features,
            competitorWeight,
            demandWeight
        );

        spdlog::info("Scores calculated, building GeoJSON");

        nlohmann::json result;
        result["geojson"] = buildGeoJson(features);

        spdlog::info("GeoJSON built, building summary");

        result["summary"] = buildSummaryReport(features, 10);

        spdlog::info("Summary built, assembling response");
        result["competitor_count"] = competitorPoints.size();
        result["demand_count"] = demandPoints.size();
        result["growth_count"] = growthPoints.size();
        result["competitor_fetch_failed"] = !competitorResult.has_value();
        result["demand_fetch_failed"] = !demandResult.has_value();
        result["growth_fetch_failed"] = !growthResult.has_value();

        return result;
    }
}

void runApiServer(int port, std::function<void()> onReady)
{
    httplib::Server server;

    server.set_default_headers({
        {"Access-Control-Allow-Origin", "*"}
        });

    server.Get("/", [](const httplib::Request&, httplib::Response& res)
        {
            auto path = findProjectRoot() / "web" / "index.html";
            std::ifstream file(path);

            if (!file.is_open())
            {
                res.status = 404;
                res.set_content("index.html not found in web/ folder", "text/plain");
                return;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            res.set_content(buffer.str(), "text/html");
        });

    server.Get("/health", [](const httplib::Request&, httplib::Response& res)
        {
            res.set_content("OK", "text/plain");
        });

    server.Get("/pbf-sources", [](const httplib::Request&, httplib::Response& res)
        {
            auto sources = loadPbfSources();

            nlohmann::json keys = nlohmann::json::array();

            for (const auto& [key, value] : sources)
            {
                keys.push_back(key);
            }

            res.set_content(keys.dump(), "application/json");
        });

    server.Get("/analyze", [](const httplib::Request& req, httplib::Response& res)
        {
            if (!req.has_param("city") || !req.has_param("business"))
            {
                res.status = 400;
                res.set_content(R"({"error":"missing 'city' or 'business' parameter"})", "application/json");
                return;
            }

            std::string city = req.get_param_value("city");
            std::string business = req.get_param_value("business");
            int resolution = 9;

            if (req.has_param("resolution"))
            {
                try
                {
                    resolution = std::stoi(req.get_param_value("resolution"));
                }
                catch (const std::exception&)
                {
                    resolution = 9;
                }
            }

            double demandWeight = 0.7;
            double competitorWeight = 0.3;

            if (req.has_param("demand_weight"))
            {
                try { demandWeight = std::stod(req.get_param_value("demand_weight")); }
                catch (const std::exception&) { demandWeight = 0.7; }
            }

            if (req.has_param("competitor_weight"))
            {
                try { competitorWeight = std::stod(req.get_param_value("competitor_weight")); }
                catch (const std::exception&) { competitorWeight = 0.3; }
            }

            if (demandWeight < 0.0 || competitorWeight < 0.0)
            {
                demandWeight = 0.7;
                competitorWeight = 0.3;
            }

            if (resolution < 0 || resolution > 15)
            {
                resolution = 9;
            }

            std::optional<PbfSource> pbfSource;

            if (req.has_param("source") && req.get_param_value("source") != "live")
            {
                auto sources = loadPbfSources();
                std::string sourceKey = req.get_param_value("source");

                auto it = sources.find(sourceKey);

                if (it == sources.end())
                {
                    res.status = 400;
                    res.set_content(R"({"error":"unknown data source"})", "application/json");
                    return;
                }

                pbfSource = it->second;
            }

            spdlog::info("Analyzing '{}' for business '{}' at resolution {}", city, business, resolution);

            try
            {
                auto result = analyzeCity(city, business, resolution, pbfSource, competitorWeight, demandWeight);
                const auto payload = result.dump();
                spdlog::info("/analyze response size: {} bytes", payload.size());
                res.set_content(payload, "application/json");
            }
            catch (const std::exception& e)
            {
                spdlog::error("Unhandled exception in /analyze: {}", e.what());
                res.status = 500;
                nlohmann::json error = { {"error", std::string("Internal error: ") + e.what()} };
                res.set_content(error.dump(), "application/json");
            }
            catch (...)
            {
                spdlog::error("Unknown unhandled exception in /analyze");
                res.status = 500;
                res.set_content(R"({"error":"Unknown internal error"})", "application/json");
            }
        });

    if (!server.bind_to_port("127.0.0.1", port))
    {
        spdlog::error("Failed to bind to port {}", port);
        return;
    }

    spdlog::info("GeoAnalytics API server listening on http://127.0.0.1:{}", port);

    if (onReady)
    {
        onReady();
    }

    server.listen_after_bind();
}