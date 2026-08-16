#pragma once

#include <functional>

void runApiServer(int port, std::function<void()> onReady = nullptr);