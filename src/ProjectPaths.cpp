#include "ProjectPaths.h"

#include <filesystem>

#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

fs::path findProjectRoot()
{
    fs::path current = fs::current_path();

    while (current.has_parent_path())
    {
        if (fs::exists(current / "CMakeLists.txt") &&
            fs::exists(current / "data"))
        {
            return current;
        }

        current = current.parent_path();
    }

    spdlog::warn("Could not locate project root (CMakeLists.txt + data/), falling back to current path: {}", fs::current_path().string());

    return fs::current_path();
}