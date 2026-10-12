// Desktop.cpp
#include "Desktop.h"

#include <algorithm>
#include <cfloat>   // FLT_MAX
#include <ctime>

#include "Log.h"
#include "Taskbar.h"   // Taskbar::height()
#include "Theme.h"

namespace {
    const float NOTICE_SECONDS = 12.0f;   // how long config warnings stay visible

    ImU32 toU32(const Color& c, int alpha = 255) {
        return IM_COL32(c.r, c.g, c.b, alpha);
    }

    // Current local date/time as text, e.g. "Sunday, Oct 11, 2026 | 02:38:15 PM"
    std::string clockText(bool use24h) {
        std::time_t now = std::time(nullptr);
        std::tm local{};
#ifdef _WIN32
        localtime_s(&local, &now);
#else
        localtime_r(&now, &local);
#endif
        char buf[64];
        std::strftime(buf, sizeof(buf),
                      use24h ? "%A, %b %d, %Y | %H:%M:%S" : "%A, %b %d, %Y | %I:%M:%S %p",
                      &local);
        return buf;
    }
}

Desktop::Desktop(const AppConfig& config) : config_(config) {}

void Desktop::loadWallpaper(std::vector<std::string>& warnings) {
    if (config_.wallpaperMode != "image") {
        Log::line("Wallpaper: gradient mode");
        return;
    }
    std::string error;
    if (loadTexture(config_.wallpaperPath, wallpaper_, error)) {
        Log::line("Wallpaper loaded: " + config_.wallpaperPath + " (" +
                  std::to_string(wallpaper_.width) + "x" + std::to_string(wallpaper_.height) + ")");
    } else {
        warnings.push_back("Wallpaper '" + config_.wallpaperPath + "' could not be loaded (" +
                           error + "); showing the gradient instead.");
    }
}

void Desktop::release() {
    freeTexture(wallpaper_);
}

void Desktop::setNotices(const std::vector<std::string>& notices) {
    notices_ = notices;
    noticesStart_ = ImGui::GetTime();
}

void Desktop::draw() {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    const ImVec2 display = ImGui::GetIO().DisplaySize;

    drawWallpaper(dl, display);   // 1. base layer: fills the whole window
    drawStatusLine(dl, display);  // 2. text on top of the wallpaper
    drawClock(dl, display);
    drawNotices(dl);
}

// ---------------------------------------------------------------------------
// Wallpaper
// ---------------------------------------------------------------------------
void Desktop::drawWallpaper(ImDrawList* dl, ImVec2 display) {
    if (!wallpaper_.valid()) {
        drawGradient(dl, display);
        return;
    }

    const float imgW = static_cast<float>(wallpaper_.width);
    const float imgH = static_cast<float>(wallpaper_.height);
    const float scaleX = display.x / imgW;
    const float scaleY = display.y / imgH;

    if (config_.wallpaperFit == "stretch") {
        // Squash/stretch the image to exactly the window size.
        dl->AddImage(wallpaper_.imguiId(), ImVec2(0, 0), display);
    } else if (config_.wallpaperFit == "fit") {
        // Whole image visible, black bars where the shapes differ.
        const float s = std::min(scaleX, scaleY);
        const ImVec2 size(imgW * s, imgH * s);
        const ImVec2 pos((display.x - size.x) * 0.5f, (display.y - size.y) * 0.5f);
        dl->AddRectFilled(ImVec2(0, 0), display, IM_COL32(0, 0, 0, 255));
        dl->AddImage(wallpaper_.imguiId(), pos, ImVec2(pos.x + size.x, pos.y + size.y));
    } else {
        // "cover": fill the whole window, cropping the image's edges evenly.
        // We crop by choosing which part of the image (UV = 0..1) to show.
        const float s = std::max(scaleX, scaleY);
        const float visibleW = display.x / s / imgW;   // fraction of image width shown
        const float visibleH = display.y / s / imgH;
        const ImVec2 uv0((1.0f - visibleW) * 0.5f, (1.0f - visibleH) * 0.5f);
        const ImVec2 uv1(uv0.x + visibleW, uv0.y + visibleH);
        dl->AddImage(wallpaper_.imguiId(), ImVec2(0, 0), display, uv0, uv1);
    }
}

void Desktop::drawGradient(ImDrawList* dl, ImVec2 display) {
    // A vertical gradient: top colour -> bottom colour (both from config.txt).
    const ImU32 top = toU32(config_.gradientTop);
    const ImU32 bottom = toU32(config_.gradientBottom);
    dl->AddRectFilledMultiColor(ImVec2(0, 0), display, top, top, bottom, bottom);

    // A subtle dot grid on top, so it reads as a "pattern" wallpaper.
    const float step = 32.0f;
    for (float y = step; y < display.y; y += step) {
        for (float x = step; x < display.x; x += step) {
            dl->AddCircleFilled(ImVec2(x, y), 1.2f, IM_COL32(255, 255, 255, 22));
        }
    }
}

// ---------------------------------------------------------------------------
// Clock: rebuilt from the system time EVERY frame, so it is always current.
// ---------------------------------------------------------------------------
void Desktop::drawClock(ImDrawList* dl, ImVec2 display) {
    const std::string text = clockText(config_.clock24h);
    ImFont* font = Theme::BoldFont;
    const float size = config_.fontSize;

    const ImVec2 textSize = font->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());
    const ImVec2 pad(12, 7);
    const ImVec2 boxMax(display.x - 12, 12 + textSize.y + pad.y * 2);
    const ImVec2 boxMin(boxMax.x - textSize.x - pad.x * 2, 12);

    dl->AddRectFilled(boxMin, boxMax, Theme::PanelMid, 6.0f);
    dl->AddRect(boxMin, boxMax, Theme::Border, 6.0f);
    dl->AddText(font, size, ImVec2(boxMin.x + pad.x, boxMin.y + pad.y), Theme::Text, text.c_str());
}

void Desktop::drawStatusLine(ImDrawList* dl, ImVec2 display) {
    const float size = config_.fontSize;
    const ImVec2 pos(16, display.y - Taskbar::height() - size - 12);
    // A dark shadow behind the text keeps it readable on bright wallpapers.
    dl->AddText(Theme::RegularFont, size, ImVec2(pos.x + 1, pos.y + 1), IM_COL32(0, 0, 0, 160),
                "CSOPESY OS v1.0 | System Online");
    dl->AddText(Theme::RegularFont, size, pos, Theme::Green, "CSOPESY OS v1.0 | System Online");
}

void Desktop::drawNotices(ImDrawList* dl) {
    if (notices_.empty()) return;
    const float t = static_cast<float>(ImGui::GetTime() - noticesStart_);
    if (t > NOTICE_SECONDS) return;

    // Fade out over the last second.
    const float alpha = std::min(1.0f, NOTICE_SECONDS - t);
    const int a = static_cast<int>(alpha * 255);
    const float size = config_.fontSize * 0.85f;
    const float wrap = 560.0f;

    ImVec2 pos(16, 16);
    float height = 0;
    for (const auto& n : notices_) {
        height += Theme::RegularFont->CalcTextSizeA(size, FLT_MAX, wrap, n.c_str()).y + 4;
    }
    dl->AddRectFilled(ImVec2(8, 8), ImVec2(16 + wrap + 8, 16 + height + 4),
                      IM_COL32(13, 17, 23, static_cast<int>(alpha * 220)), 6.0f);
    for (const auto& n : notices_) {
        dl->AddText(Theme::RegularFont, size, pos, IM_COL32(227, 179, 65, a), n.c_str(), nullptr, wrap);
        pos.y += Theme::RegularFont->CalcTextSizeA(size, FLT_MAX, wrap, n.c_str()).y + 4;
    }
}
