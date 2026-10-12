#pragma once
// Desktop.h
// ---------------------------------------------------------------------------
// The bottom layer of the OS, drawn FIRST every frame:
//   - the wallpaper (an image file, or a colour gradient drawn in code)
//   - a real-time clock in the top-right corner
//   - a status line ("CSOPESY OS v1.0 | System Online")
//   - warnings about config.txt, shown for a few seconds after startup
//
// Everything here is painted on ImGui's BACKGROUND draw list, which is always
// drawn behind every window -- exactly what a desktop should be.
// ---------------------------------------------------------------------------
#include <string>
#include <vector>

#include "Config.h"
#include "Texture.h"
#include "imgui.h"

class Desktop {
public:
    explicit Desktop(const AppConfig& config);

    // Loads the wallpaper image (if wallpaper_mode=image). Must be called
    // after OpenGL is ready. Problems are added to `warnings`.
    void loadWallpaper(std::vector<std::string>& warnings);

    // Frees the wallpaper texture (call before OpenGL shuts down).
    void release();

    // Lines to show in the top-left notice box for a few seconds.
    void setNotices(const std::vector<std::string>& notices);

    // Draws the desktop. Called every frame.
    void draw();

private:
    void drawWallpaper(ImDrawList* dl, ImVec2 display);
    void drawGradient(ImDrawList* dl, ImVec2 display);
    void drawClock(ImDrawList* dl, ImVec2 display);
    void drawStatusLine(ImDrawList* dl, ImVec2 display);
    void drawNotices(ImDrawList* dl);

    const AppConfig& config_;
    Texture wallpaper_;
    std::vector<std::string> notices_;
    double noticesStart_ = 0.0;   // when they were set (they show for NOTICE_SECONDS)
};
