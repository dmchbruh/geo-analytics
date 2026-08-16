#include "io/FileUtils.h"

#include <filesystem>

#include <spdlog/spdlog.h>

std::ofstream openOutputFile(const std::string& path)
{
    std::filesystem::path filePath(path);

    if (filePath.has_parent_path())
    {
        std::filesystem::create_directories(filePath.parent_path());
    }

    std::ofstream file(path);

    if (!file.is_open())
    {
        spdlog::error("Can't open output file: {}", path);
    }

    return file;
}