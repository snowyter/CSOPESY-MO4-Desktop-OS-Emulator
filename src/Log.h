#pragma once
// Log.h
// ---------------------------------------------------------------------------
// A tiny logger. The program is a GUI app (no console window on Windows), so
// messages go to csopesy.log next to the .exe. Open that file when something
// doesn't work, e.g. the wallpaper image didn't load.
// ---------------------------------------------------------------------------
#include <string>

namespace Log {
    void open(const std::string& path);   // call once at startup
    void line(const std::string& message);
    void close();
}
