#include "util/Logging.h"

#include "ProjectPaths.h"

#include <filesystem>
#include <iostream>
#include <limits>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace fs = std::filesystem;

namespace
{
#ifdef _WIN32
    LONG WINAPI unhandledExceptionHandler(EXCEPTION_POINTERS* info)
    {
        const DWORD code = info && info->ExceptionRecord
            ? info->ExceptionRecord->ExceptionCode
            : 0;

        spdlog::critical("Unhandled Windows exception (code 0x{:08X}). See log file for details.", code);
        flushLogging();

        std::cerr << "\nFatal error (code 0x" << std::hex << code << std::dec
            << "). Details were written to the log file.\n"
            << "Press Enter to close..." << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();

        return EXCEPTION_EXECUTE_HANDLER;
    }
#endif
}

void initLogging(const std::string& logFilePath)
{
    std::error_code ec;
    fs::create_directories(fs::path(logFilePath).parent_path(), ec);

    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath, true);

    consoleSink->set_level(spdlog::level::info);
    fileSink->set_level(spdlog::level::debug);

    auto logger = std::make_shared<spdlog::logger>(
        "geoanalytics",
        spdlog::sinks_init_list{ consoleSink, fileSink }
    );

    logger->set_level(spdlog::level::debug);
    logger->flush_on(spdlog::level::debug);

    spdlog::set_default_logger(logger);
    spdlog::info("Logging to {}", logFilePath);
}

void flushLogging()
{
    if (auto logger = spdlog::default_logger())
    {
        logger->flush();
    }
}

void installCrashHandler()
{
#ifdef _WIN32
    SetUnhandledExceptionFilter(unhandledExceptionHandler);
#endif
}

void waitForExitPrompt(int exitCode)
{
    flushLogging();

    std::cerr << "\nProcess finished with exit code " << exitCode
        << ". Press Enter to close..." << std::endl;

    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}