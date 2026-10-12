#pragma once
// BootScreen.h
// ---------------------------------------------------------------------------
// The startup sequence shown before the desktop, using the boot phases from
// the lecture:
//
//   BIOS_HEADER    BIOS name, release date and copyright
//   RAM_CHECK      "Checking RAM" counts up to 8 GB
//   HARDWARE_INFO  detected "hardware" is listed (real OpenGL/GLFW/ImGui info)
//   LOADING_OS     CSOPESY banner + "Loading..."
//   COMPLETE       done -> the desktop takes over
//
// Plain console look (grey text on black), like the examples in the specs.
// The first three make up the BIOS screen (boot_bios_ms in config.txt), the
// loading screen lasts boot_loading_ms. Click or press any key to skip ahead.
// The shutdown screen reuses the same banner (drawSplash).
// ---------------------------------------------------------------------------
#include <string>

#include "Config.h"
#include "imgui.h"

// Real details about the machine, filled in by GUIApplication after OpenGL starts.
struct SystemInfo {
    std::string glVersion;    // "3.3.0 NVIDIA 560.94"
    std::string glRenderer;   // "NVIDIA GeForce RTX 3060/PCIe/SSE2"
    std::string glfwVersion;  // "3.4.0"
};

class BootScreen {
public:
    enum class BootPhase { BIOS_HEADER, RAM_CHECK, HARDWARE_INFO, LOADING_OS, COMPLETE };

    BootScreen(const AppConfig& config, SystemInfo info);

    void draw();                        // call once per frame while booting
    bool finished() const { return phase_ == BootPhase::COMPLETE; }
    BootPhase phase() const { return phase_; }

    // Black screen with the CSOPESY banner, title, section/group and a green
    // status word with moving dots ("Loading", "Shutting down").
    static void drawSplash(const AppConfig& config, const char* status);

private:
    void update();                      // moves to the next phase when its time is up
    void skip();                        // click / key: jump to the next screen
    void startLoading();
    void drawBios(ImDrawList* dl, float t);

    const AppConfig& config_;
    SystemInfo info_;

    BootPhase phase_ = BootPhase::BIOS_HEADER;
    double biosStart_ = -1.0;           // ImGui time when the BIOS screen started
    double loadingStart_ = -1.0;        // ... and when the loading screen started
};
