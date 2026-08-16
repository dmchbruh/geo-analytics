#include "io/CsvWriter.h"
#include "io/FileUtils.h"

#include <h3/h3api.h>
#include <spdlog/spdlog.h>

void writeCsv(
    const std::string& path,
    const std::vector<H3Point>& points
)
{
    std::ofstream file = openOutputFile(path);

    if (!file.is_open())
    {
        return;
    }

    file << "id,lat,lon,h3_index\n";

    for (const auto& point : points)
    {
        char h3String[17];
        h3ToString(point.h3Index, h3String, sizeof(h3String));

        file
            << point.id << ","
            << point.lat << ","
            << point.lon << ","
            << h3String
            << "\n";
    }
}