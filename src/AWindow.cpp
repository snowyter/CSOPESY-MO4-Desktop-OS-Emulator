// AWindow.cpp
#include "AWindow.h"

#include <utility>

#include "UIManager.h"

AWindow::AWindow(std::string id, std::string title, ImVec2 defaultSize, ImVec2 defaultPos)
    : id_(std::move(id)), title_(std::move(title)), appName_(title_),
      defaultSize_(defaultSize), defaultPos_(defaultPos) {}

void AWindow::show() {
    visible_ = true;
    minimized_ = false;
    focusNextFrame_ = true;
}

void AWindow::hide() {
    visible_ = false;
    minimized_ = false;
    focused_ = false;
}

void AWindow::minimize() {
    minimized_ = true;
    focused_ = false;
}

bool AWindow::beginWindow(ImGuiWindowFlags extraFlags) {
    if (!visible_ || minimized_) return false;

    // First time only: give it a size and a starting position.
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowSize(defaultSize_, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(display.x * defaultPos_.x, display.y * defaultPos_.y),
                            ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    if (focusNextFrame_) {
        ImGui::SetNextWindowFocus();
        focusNextFrame_ = false;
    }

    // "Title###id": ImGui shows the part before ### and uses the part after
    // it as the window's identity, so a title can change without ImGui
    // treating it as a new window.
    const std::string label = title_ + "###" + id_;

    // Passing &visible_ gives the window an X button that sets visible_ = false.
    const bool expanded = ImGui::Begin(label.c_str(), &visible_,
                                      ImGuiWindowFlags_NoSavedSettings | extraFlags);
    focused_ = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    if (focused_) UIManager::instance().setActive(id_);

    if (!expanded) {          // collapsed (double-clicked title bar)
        ImGui::End();
        return false;
    }
    return true;
}

void AWindow::endWindow() {
    ImGui::End();
}
