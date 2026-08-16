#include "server/ApiServer.h"
#include "ProjectPaths.h"
#include "util/Logging.h"

#include <exception>
#include <filesystem>
#include <iostream>

#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

int runServer()
{
    auto logPath = (findProjectRoot() / "logs" / "server.log").string();
    initLogging(logPath);
    installCrashHandler();

    spdlog::info("GeoAnalytics server starting (cwd: {})", fs::current_path().string());

    runApiServer(8080, []()
        {
            std::system("start http://127.0.0.1:8080/");
        });

    spdlog::warn("Server stopped listening");
    return 0;
}

int main()
{
    int exitCode = 0;

    try
    {
        exitCode = runServer();
    }
    catch (const std::exception& e)
    {
        exitCode = 1;

        try
        {
            spdlog::critical("Server failed: {}", e.what());
        }
        catch (...)
        {
            std::cerr << "Server failed: " << e.what() << std::endl;
        }
    }
    catch (...)
    {
        exitCode = 1;

        try
        {
            spdlog::critical("Server failed with unknown error");
        }
        catch (...)
        {
            std::cerr << "Server failed with unknown error" << std::endl;
        }
    }

    waitForExitPrompt(exitCode);
    return exitCode;
}
