#include "db/Database.h"

#include <sqlite3.h>

#include <h3/h3api.h>
#include <spdlog/spdlog.h>

Database::Database(const std::filesystem::path& path)
{
    if (path.has_parent_path())
    {
        std::filesystem::create_directories(path.parent_path());
    }

    int result = sqlite3_open(path.string().c_str(), &db_);

    if (result != SQLITE_OK)
    {
        spdlog::error("Can't open database: {}", db_ ? sqlite3_errmsg(db_) : "unknown error");
        sqlite3_close(db_);
        db_ = nullptr;
        return;
    }

    const char* schema =
        "CREATE TABLE IF NOT EXISTS runs ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "created_at TEXT NOT NULL,"
        "input_file TEXT NOT NULL,"
        "resolution INTEGER NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS points ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "run_id INTEGER NOT NULL,"
        "external_id TEXT NOT NULL,"
        "lat REAL NOT NULL,"
        "lon REAL NOT NULL,"
        "FOREIGN KEY(run_id) REFERENCES runs(id)"
        ");"
        "CREATE TABLE IF NOT EXISTS hex_features ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "run_id INTEGER NOT NULL,"
        "hex_id TEXT NOT NULL,"
        "rank INTEGER NOT NULL,"
        "points_count INTEGER NOT NULL,"
        "neighbors_count INTEGER NOT NULL,"
        "neighbor_points_count INTEGER NOT NULL,"
        "density REAL NOT NULL,"
        "score REAL NOT NULL,"
        "confidence REAL NOT NULL,"
        "category TEXT NOT NULL,"
        "FOREIGN KEY(run_id) REFERENCES runs(id)"
        ");";

    char* errorMessage = nullptr;

    result = sqlite3_exec(db_, schema, nullptr, nullptr, &errorMessage);

    if (result != SQLITE_OK)
    {
        spdlog::error("Can't create schema: {}", errorMessage);
        sqlite3_free(errorMessage);
    }
}

Database::~Database()
{
    if (db_ != nullptr)
    {
        sqlite3_close(db_);
    }
}

bool Database::isOpen() const
{
    return db_ != nullptr;
}

std::int64_t Database::insertRun(
    const std::string& inputFile,
    int resolution
)
{
    if (db_ == nullptr)
    {
        return -1;
    }

    const char* sql =
        "INSERT INTO runs (created_at, input_file, resolution) VALUES (datetime('now'), ?, ?);";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        spdlog::error("Failed to prepare insertRun statement: {}", sqlite3_errmsg(db_));
        return -1;
    }

    sqlite3_bind_text(statement, 1, inputFile.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(statement, 2, resolution);

    if (sqlite3_step(statement) != SQLITE_DONE)
    {
        spdlog::error("Failed to insert run: {}", sqlite3_errmsg(db_));
        sqlite3_finalize(statement);
        return -1;
    }

    sqlite3_finalize(statement);

    return sqlite3_last_insert_rowid(db_);
}

void Database::insertPoints(
    std::int64_t runId,
    const std::vector<Point>& points
)
{
    if (db_ == nullptr || runId < 0)
    {
        return;
    }

    sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    const char* sql =
        "INSERT INTO points (run_id, external_id, lat, lon) VALUES (?, ?, ?, ?);";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        spdlog::error("Failed to prepare insertPoints statement: {}", sqlite3_errmsg(db_));
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    for (const auto& point : points)
    {
        sqlite3_bind_int64(statement, 1, runId);
        sqlite3_bind_text(statement, 2, point.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(statement, 3, point.lat);
        sqlite3_bind_double(statement, 4, point.lon);

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            spdlog::error("Failed to insert point '{}': {}", point.id, sqlite3_errmsg(db_));
        }

        sqlite3_reset(statement);
    }

    sqlite3_finalize(statement);

    sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, nullptr);
}

void Database::insertFeatures(
    std::int64_t runId,
    const std::vector<HexFeature>& features
)
{
    if (db_ == nullptr || runId < 0)
    {
        return;
    }

    sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    const char* sql =
        "INSERT INTO hex_features "
        "(run_id, hex_id, rank, points_count, neighbors_count, neighbor_points_count, density, score, confidence, category) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(db_, sql, -1, &statement, nullptr) != SQLITE_OK)
    {
        spdlog::error("Failed to prepare insertFeatures statement: {}", sqlite3_errmsg(db_));
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    for (const auto& feature : features)
    {
        char h3String[17];
        h3ToString(feature.hexId, h3String, sizeof(h3String));

        sqlite3_bind_int64(statement, 1, runId);
        sqlite3_bind_text(statement, 2, h3String, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(statement, 3, feature.rank);
        sqlite3_bind_int64(statement, 4, static_cast<sqlite3_int64>(feature.pointsCount));
        sqlite3_bind_int64(statement, 5, static_cast<sqlite3_int64>(feature.neighborsCount));
        sqlite3_bind_int64(statement, 6, static_cast<sqlite3_int64>(feature.neighborPointsCount));
        sqlite3_bind_double(statement, 7, feature.density);
        sqlite3_bind_double(statement, 8, feature.score);
        sqlite3_bind_double(statement, 9, feature.confidence);
        sqlite3_bind_text(statement, 10, feature.category.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(statement) != SQLITE_DONE)
        {
            spdlog::error("Failed to insert feature: {}", sqlite3_errmsg(db_));
        }

        sqlite3_reset(statement);
    }

    sqlite3_finalize(statement);

    sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, nullptr);

}

std::int64_t Database::countRows(const std::string& table) const
{
    if (db_ == nullptr)
    {
        return -1;
    }

    std::string sql = "SELECT COUNT(*) FROM " + table + ";";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &statement, nullptr) != SQLITE_OK)
    {
        spdlog::error("Failed to prepare countRows statement: {}", sqlite3_errmsg(db_));
        return -1;
    }

    std::int64_t count = -1;

    if (sqlite3_step(statement) == SQLITE_ROW)
    {
        count = sqlite3_column_int64(statement, 0);
    }

    sqlite3_finalize(statement);

    return count;
}