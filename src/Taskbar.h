#pragma once
// Taskbar.h
// ---------------------------------------------------------------------------
// The fixed bar along the bottom of the screen:
//   left  : CSOPESY logo + one icon button per app (File Explorer, Notepad,
//           Task Manager). A bar under an icon means its window is open.
//   right : system tray -- frame rate and the PWR (shutdown) button.
//           PWR first asks "Are you sure?" in a pop-up.
//
// It is an ImGui window with no title bar that can't be moved, and it is
// pushed to the front every frame so app windows never cover it.
// ---------------------------------------------------------------------------
#include "Config.h"
#include "imgui.h"

class Taskbar {
public:
    explicit Taskbar(const AppConfig& config);

    // Height of the bar in pixels (the desktop uses it to place the status line).
    static float height() { return 64.0f; }

    // Draws the taskbar. Called every frame, after the app windows.
    void draw();

private:
    void drawLogo();
    void drawAppButtons();
    void drawTray();
    void drawShutdownConfirm();   // the "Are you sure?" pop-up after PWR
    bool roundButton(const char* id, const char* label, unsigned int labelColor, float diameter);

    // One taskbar entry: which window it opens and how its icon is drawn.
    struct AppButton {
        const char* windowId;     // UIManager id, e.g. "taskmgr"
        const char* tooltip;      // shown on hover
        void (*drawIcon)(ImDrawList*, ImVec2 center, float size);
    };
    bool appButton(const AppButton& app, float size);

    const AppConfig& config_;
    bool askShutdown_ = false;    // PWR was clicked: open the pop-up
};
