#pragma once
// Config.h
// ---------------------------------------------------------------------------
// Every setting you might need to change during the quiz lives in config.txt
// (no recompiling allowed). AppConfig holds the DEFAULT values; loadConfig()
// overwrites whatever config.txt sets. A missing or bad line never stops the
// program: it keeps the default and adds a warning that is shown on screen.
// ---------------------------------------------------------------------------
#include <string>
#include <vector>

// An RGB colour, 0-255 per channel. Written in config.txt as #RRGGBB.
struct Color {
    int r = 0, g = 0, b = 0;
};

struct AppConfig {
    // ---- Window ----
    int windowWidth  = 1280;
    int windowHeight = 720;
    std::string windowTitle = "CSOPESY Desktop OS Emulator";
    bool vsync = true;                     // true = lock to the monitor's refresh rate

    // ---- Look ----
    std::string fontPath = "assets/fonts/JetBrainsMono-Regular.ttf";
    float fontSize = 16.0f;

    // ---- Wallpaper ----
    std::string wallpaperMode = "image";   // "image" or "gradient"
    std::string wallpaperPath = "assets/wallpaper.jpg";
    std::string wallpaperFit  = "cover";   // "cover", "fit" or "stretch"
    Color gradientTop    {16, 42, 84};     // #102A54
    Color gradientBottom {58, 120, 168};   // #3A78A8

    // ---- Clock ----
    bool clock24h = false;                 // false = 02:38:15 PM, true = 14:38:15

    // ---- Boot / shutdown ----
    bool bootEnabled = true;               // show the BIOS + loading screens
    int bootBiosMs = 4000;                 // how long the BIOS screen lasts
    int bootLoadingMs = 3000;              // how long the "Loading" screen lasts
    int shutdownMs = 3000;                 // how long "Shutting down..." lasts

    // ---- Task Manager ----
    int taskmgrRefreshMs = 1000;           // how often the dummy CPU/memory values change
};

// Reads `path` and overwrites the matching fields of `config`.
// Returns false if the file could not be opened (defaults are kept).
// Every problem is added to `warnings` as a readable sentence.
bool loadConfig(const std::string& path, AppConfig& config, std::vector<std::string>& warnings);
