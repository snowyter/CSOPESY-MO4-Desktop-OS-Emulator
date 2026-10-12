#pragma once
// UIManager.h
// ---------------------------------------------------------------------------
// The "window registry" from the lecture: one shared object that keeps a list
// of every app window, like a kernel's process table. The taskbar asks it to
// open/close windows by name, and the main loop asks it to draw them all.
//
// It's a SINGLETON: UIManager::instance() always returns the same object, so
// any part of the program can reach it without passing it around.
//
// It also holds the "power off requested" flag set by the PWR button.
// ---------------------------------------------------------------------------
#include <memory>
#include <string>
#include <vector>

#include "AWindow.h"

class UIManager {
public:
    static UIManager& instance();

    // Registry
    void registerWindow(std::shared_ptr<AWindow> window);
    AWindow* find(const std::string& id) const;
    const std::vector<std::shared_ptr<AWindow>>& windows() const { return windows_; }

    void show(const std::string& id);
    void hide(const std::string& id);
    bool isShown(const std::string& id) const;

    bool isMinimized(const std::string& id) const;

    // Taskbar click: closed or minimised -> open it; open but behind -> bring
    // it to the front; open and in front -> minimise it (like Windows).
    void activate(const std::string& id);

    // The app window the user last worked in. Clicking the taskbar moves
    // ImGui's focus to the taskbar, so we remember this separately.
    void setActive(const std::string& id) { active_ = id; }
    bool isActive(const std::string& id) const { return active_ == id && isShown(id) && !isMinimized(id); }

    // Draws every visible window. Called once per frame by the main loop.
    void renderAllWindows();

    // Removes all windows (used at shutdown).
    void clear();

    // Power
    void requestPowerOff()            { powerOff_ = true; }
    bool powerOffRequested() const    { return powerOff_; }

private:
    UIManager() = default;                       // only instance() can create it
    UIManager(const UIManager&) = delete;
    UIManager& operator=(const UIManager&) = delete;

    std::vector<std::shared_ptr<AWindow>> windows_;
    std::string active_;
    bool powerOff_ = false;
};
