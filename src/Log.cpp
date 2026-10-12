// Log.cpp
#include "Log.h"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>

namespace {
    std::ofstream logFile;

    std::string timestamp() {
        std::time_t now = std::time(nullptr);
        std::tm local{};
#ifdef _WIN32
        localtime_s(&local, &now);
#else
        localtime_r(&now, &local);
#endif
        char buf[16];
        std::strftime(buf, sizeof(buf), "%H:%M:%S", &local);
        return buf;
    }
}

namespace Log {

void open(const std::string& path) {
    logFile.open(path, std::ios::out | std::ios::trunc);
}

void line(const std::string& message) {
    std::string text = "[" + timestamp() + "] " + message;
    if (logFile.is_open()) {
        logFile << text << '\n';
        logFile.flush();             // so the file is complete even after a crash
    }
    std::cerr << text << '\n';       // also visible when run from a terminal
}

void close() {
    if (logFile.is_open()) logFile.close();
}

} // namespace Log
