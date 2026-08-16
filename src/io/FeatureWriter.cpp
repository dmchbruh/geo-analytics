#include "io/FeatureWriter.h"
#include "io/FileUtils.h"
#include <fstream>
#include <h3/h3api.h>

void writeFeaturesCsv(
    const std::string& path,
    const std::vector<HexFeature>& features
)
{
    std::ofstream file = openOutputFile(path);

    if (!file.is_open())
    {
        return;
    }

    file << "rank,hex_id,points_count,neighbor_points_count,density,score,confidence,category\n";

    for (const auto& feature : features)
    {
        char h3String[17];

        h3ToString(feature.hexId, h3String, sizeof(h3String));

        

        file
            << feature.rank << ","
            << h3String << ","
            << feature.pointsCount << ","
            << feature.neighborPointsCount << ","
            << feature.density << ","
            << feature.score << ","
            << feature.confidence << ","
            << feature.category
            << "\n";
    }
}