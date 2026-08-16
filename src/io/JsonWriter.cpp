#include "io/JsonWriter.h"
#include "io/FileUtils.h"

#include <fstream>

#include <h3/h3api.h>
#include <nlohmann/json.hpp>

void writeFeaturesJson(
    const std::string& path,
    const std::vector<HexFeature>& features
)
{
    nlohmann::json output = nlohmann::json::array();

    for (const auto& feature : features)
    {
        char h3String[17];

        h3ToString(feature.hexId, h3String, sizeof(h3String));

        output.push_back({
            {"hex_id", h3String},
            {"points_count", feature.pointsCount},
            {"score", feature.score}
            });
    }

    std::ofstream file = openOutputFile(path);

    if (!file.is_open())
    {
        return;
    }

    file << output.dump(4);
}