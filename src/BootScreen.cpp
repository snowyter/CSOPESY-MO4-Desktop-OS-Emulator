// BootScreen.cpp
#include "BootScreen.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <utility>

#include "Theme.h"

namespace {
    // Where each BIOS part starts, as a fraction of boot_bios_ms.
    const float RAM_CHECK_AT     = 0.15f;
    const float HARDWARE_INFO_AT = 0.55f;

    const long long TOTAL_RAM_KB = 8LL * 1024 * 1024;   // 8 GB, same as Task Manager

    const char* GROUP_LINE = "CSOPESY - Section S04 - Group 10";

    // Console colours (like the Windows terminal)
    const ImU32 GREY  = IM_COL32(204, 204, 204, 255);
    const ImU32 BLUE  = IM_COL32( 58, 150, 221, 255);
    const ImU32 GREEN = IM_COL32( 22, 198,  12, 255);

    // CSOPESY in big letters.
    const char* BANNER[] = {
        R"(  ____ ____   ___  ____  _____ ____  __   __)",
        R"( / ___/ ___| / _ \|  _ \| ____/ ___| \ \ / /)",
        R"(| |   \___ \| | | | |_) |  _| \___ \  \ V / )",
        R"(| |___ ___) | |_| |  __/| |___ ___) |  | |  )",
        R"( \____|____/ \___/|_|   |_____|____/   |_|  )",
    };

    float clamp01(float v) { return std::clamp(v, 0.0f, 1.0f); }

    // Any mouse click or key press this frame?
    bool anyInput() {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) return true;
        for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; ++k) {
            if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(k), false)) return true;
        }
        return false;
    }

    // Text centred horizontally on x.
    void centredText(ImDrawList* dl, float size, float x, float y, ImU32 color, const char* text) {
        const ImVec2 ts = Theme::RegularFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text);
        dl->AddText(Theme::RegularFont, size, ImVec2(x - ts.x * 0.5f, y), color, text);
    }
}

BootScreen::BootScreen(const AppConfig& config, SystemInfo info)
    : config_(config), info_(std::move(info)) {
    if (config_.bootBiosMs <= 0) phase_ = BootPhase::LOADING_OS;   // BIOS screen turned off
}

// ===========================================================================
// Phase changes
// ===========================================================================
void BootScreen::startLoading() {
    phase_ = BootPhase::LOADING_OS;
    loadingStart_ = ImGui::GetTime();
    if (config_.bootLoadingMs <= 0) phase_ = BootPhase::COMPLETE;  // loading screen turned off
}

void BootScreen::skip() {
    if (phase_ == BootPhase::LOADING_OS) phase_ = BootPhase::COMPLETE;
    else if (phase_ != BootPhase::COMPLETE) startLoading();
}

void BootScreen::update() {
    const double now = ImGui::GetTime();
    if (biosStart_ < 0.0) {                       // first frame of the boot
        biosStart_ = now;
        if (phase_ == BootPhase::LOADING_OS) startLoading();
    }

    if (anyInput()) {
        skip();
        return;
    }

    if (phase_ == BootPhase::LOADING_OS) {
        if ((now - loadingStart_) * 1000.0 >= config_.bootLoadingMs) phase_ = BootPhase::COMPLETE;
        return;
    }
    if (phase_ == BootPhase::COMPLETE) return;

    // Still on the BIOS screen: which part are we in?
    const float t = static_cast<float>((now - biosStart_) * 1000.0 / config_.bootBiosMs);
    if (t >= 1.0f)                   startLoading();
    else if (t >= HARDWARE_INFO_AT)  phase_ = BootPhase::HARDWARE_INFO;
    else if (t >= RAM_CHECK_AT)      phase_ = BootPhase::RAM_CHECK;
    else                             phase_ = BootPhase::BIOS_HEADER;
}

// ===========================================================================
// Drawing
// ===========================================================================
void BootScreen::draw() {
    update();
    if (finished()) return;

    if (phase_ == BootPhase::LOADING_OS) {
        drawSplash(config_, "Loading");
        return;
    }
    // The foreground draw list is drawn on top of everything, so the boot
    // screen covers the whole window.
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    dl->AddRectFilled(ImVec2(0, 0), ImGui::GetIO().DisplaySize, IM_COL32(0, 0, 0, 255));
    const float t = static_cast<float>((ImGui::GetTime() - biosStart_) * 1000.0 / config_.bootBiosMs);
    drawBios(dl, clamp01(t));
}

void BootScreen::drawBios(ImDrawList* dl, float t) {
    const float size = config_.fontSize;
    const float lineH = size * 1.3f;
    const float x = 16.0f;
    float y = 12.0f;

    auto line = [&](const std::string& text) {
        dl->AddText(Theme::RegularFont, size, ImVec2(x, y), GREY, text.c_str());
        y += lineH;
    };
    auto blank = [&](int n) { y += lineH * n; };

    // ---- BIOS_HEADER ----
    line("CSOPESY BIOS v1.0");
    line("Released: 10/11/26");
    line("Section S04 - Group 10 (C)2026 Justin David, Kryster Limpin, Keith Singson");
    blank(2);

    // ---- RAM_CHECK: count up to 8 GB ----
    if (t >= RAM_CHECK_AT) {
        line("Memory Test:");
        const float ram = clamp01((t - RAM_CHECK_AT) / (HARDWARE_INFO_AT - RAM_CHECK_AT - 0.05f));
        const long long kb = static_cast<long long>(TOTAL_RAM_KB * ram) / 1024 * 1024;   // whole MB steps
        char buf[64];
        std::snprintf(buf, sizeof(buf), "Checking RAM : %lldK%s", kb, ram >= 1.0f ? " OK" : "");
        line(buf);
    } else {
        blank(2);
    }
    blank(2);

    // ---- HARDWARE_INFO: one line at a time ----
    if (t >= HARDWARE_INFO_AT) {
        const std::string renderer = info_.glRenderer.size() > 60 ? info_.glRenderer.substr(0, 57) + "..." : info_.glRenderer;
        const std::string items[] = {
            "CPU Type: CSOPESY Virtual CPU",
            "BIOS Version: CSOPESY BIOS 1.0 (S04 Group 10)",
            "Graphics: " + renderer,
            "OpenGL Version: " + info_.glVersion,
            "Window System: GLFW " + info_.glfwVersion,
            "GUI Library: Dear ImGui " + std::string(IMGUI_VERSION),
            "",
            "Primary Master:   Local Disk (C:)",
            "Primary Slave:    None",
            "Display:          " + std::to_string(config_.windowWidth) + " x " + std::to_string(config_.windowHeight),
            "",
            "Booting from Local Disk (C:)...",
        };
        const int count = static_cast<int>(sizeof(items) / sizeof(items[0]));
        const float span = 0.95f - HARDWARE_INFO_AT;   // all lines shown by 95%
        const int shown = std::min(count, 1 + static_cast<int>((t - HARDWARE_INFO_AT) / span * count));
        for (int i = 0; i < shown; ++i) line(items[i]);
    }

    // Blinking underscore cursor, like a console
    if (std::fmod(ImGui::GetTime(), 1.0) < 0.5) line("_");

    // Footer
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    dl->AddText(Theme::RegularFont, size, ImVec2(x, display.y - lineH - 12.0f), GREY,
                "Press any key to continue...");
}

// Used by the loading screen AND the shutdown screen.
void BootScreen::drawSplash(const AppConfig& config, const char* status) {
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    dl->AddRectFilled(ImVec2(0, 0), display, IM_COL32(0, 0, 0, 255));

    const float size = config.fontSize;
    const float cx = display.x * 0.5f;
    const float lineH = size * 1.3f;

    // ---- Banner ----
    const float bannerSize = size * 1.4f;
    const int bannerLines = static_cast<int>(sizeof(BANNER) / sizeof(BANNER[0]));
    const float bannerW = Theme::RegularFont->CalcTextSizeA(bannerSize, FLT_MAX, 0.0f, BANNER[1]).x;
    float y = display.y * 0.24f;
    for (int i = 0; i < bannerLines; ++i) {
        dl->AddText(Theme::RegularFont, bannerSize, ImVec2(cx - bannerW * 0.5f, y), BLUE, BANNER[i]);
        y += bannerSize * 1.1f;
    }
    y += lineH;

    // ---- Title + group ----
    centredText(dl, size, cx, y, GREY, "CSOPESY Desktop OS Emulator v1.0");
    y += lineH;
    centredText(dl, size, cx, y, GREY, GROUP_LINE);
    y += lineH * 2.0f;

    // ---- "Loading." -> "Loading.." -> "Loading..." ----
    // Positioned as if the 3 dots were always there, so the word doesn't jump.
    const int dots = 1 + static_cast<int>(ImGui::GetTime() * 2.0) % 3;
    const std::string full = std::string(status) + "...";
    const std::string text = std::string(status) + std::string(dots, '.');
    const float w = Theme::RegularFont->CalcTextSizeA(size, FLT_MAX, 0.0f, full.c_str()).x;
    dl->AddText(Theme::RegularFont, size, ImVec2(cx - w * 0.5f, y), GREEN, text.c_str());
}
