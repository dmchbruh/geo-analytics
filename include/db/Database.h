#pragma once

#include "analytics/HexFeature.h"
#include "io/CsvReader.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

struct sqlite3;

class Database
{
public:
    
    std::int64_t countRows(const std::string& table) const;
    
    explicit Database(const std::filesystem::path& path);
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    bool isOpen() const;

    std::int64_t insertRun(
        const std::string& inputFile,
        int resolution
    );

    void insertPoints(
        std::int64_t runId,
        const std::vector<Point>& points
    );

    void insertFeatures(
        std::int64_t runId,
        const std::vector<HexFeature>& features
    );

private:
    sqlite3* db_ = nullptr;
};