#pragma once

#include <string>

void initLogging(const std::string& logFilePath);

void flushLogging();

void installCrashHandler();

void waitForExitPrompt(int exitCode);
