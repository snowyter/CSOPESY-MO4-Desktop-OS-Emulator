#pragma once
// AWindow.h
// ---------------------------------------------------------------------------
// Base class for every app window (Task Manager, File Explorer, Notepad).
// Each subclass only writes draw(); this class handles showing, hiding,
// focusing and the ImGui::Begin()/End() pair.
//
// Immediate mode reminder: the window does not "exist" between frames.
// While visible, draw() is called EVERY frame and re-declares the whole
// window from scratch.
// ---------------------------------------------------------------------------
#include <string>

#include "imgui.h"

class AWindow {
public:
    // defaultPos is where the window's centre goes the first time it opens,
    // as a fraction of the screen (0.5, 0.45 = middle, a bit above centre).
    AWindow(std::string id, std::string title, ImVec2 defaultSize,
            ImVec2 defaultPos = ImVec2(0.5f, 0.45f));
    virtual ~AWindow() = default;

    // Called every frame while the window is visible.
    virtual void draw() = 0;

    void show();                  // open / restore it and bring it to the front
    void hide();                  // close it (the X button does the same)
    void minimize();              // keep it running but stop drawing it
    bool isShown() const { return visible_; }        // open (even if minimised)
    bool isMinimized() const { return visible_ && minimized_; }
    bool isFocused() const { return focused_; }   // was it the active window last frame?

    const std::string& id() const { return id_; }
    const std::string& title() const { return title_; }
    // The app's plain name ("Notepad"). Stays the same even if the title
    // bar changes (e.g. "notes.txt - Notepad"). Task Manager lists this.
    const std::string& appName() const { return appName_; }

protected:
    // Wraps ImGui::Begin(). Returns false if the window is hidden or
    // collapsed -- then draw() must return without drawing anything.
    bool beginWindow(ImGuiWindowFlags extraFlags = 0);
    // Wraps ImGui::End(). Only call it if beginWindow() returned true.
    void endWindow();

    std::string id_;              // unique name used by UIManager ("taskmgr")
    std::string title_;           // text in the title bar ("Task Manager"); may change
    std::string appName_;         // fixed app name (= the first title)
    ImVec2 defaultSize_;          // size the first time it opens
    ImVec2 defaultPos_;           // centre position the first time it opens (fraction of screen)

private:
    bool visible_ = false;
    bool minimized_ = false;
    bool focused_ = false;
    bool focusNextFrame_ = false; // show() asks ImGui to focus it once
};
