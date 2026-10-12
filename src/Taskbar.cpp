// Taskbar.cpp
#include "Taskbar.h"

#include <cfloat>
#include <cstdio>
#include <string>

#include "imgui.h"
#include "imgui_internal.h"   // BringWindowToDisplayFront
#include "Icons.h"
#include "Theme.h"
#include "UIManager.h"

namespace {
    // Tooltip for the last item, shown ABOVE it (the default spot, under the
    // mouse, would hang off the bottom of the screen). pivotX: 0 = tooltip
    // starts at the item's left edge, 0.5 = centred, 1 = ends at its right edge.
    void tooltipAbove(const char* text, float pivotX = 0.5f) {
        if (!ImGui::IsItemHovered()) return;
        const ImVec2 min = ImGui::GetItemRectMin();
        const ImVec2 max = ImGui::GetItemRectMax();
        const float x = min.x + (max.x - min.x) * pivotX;
        ImGui::SetNextWindowPos(ImVec2(x, min.y - 14.0f), ImGuiCond_Always, ImVec2(pivotX, 1.0f));
        ImGui::SetTooltip("%s", text);
    }
}

Taskbar::Taskbar(const AppConfig& config) : config_(config) {}

void Taskbar::draw() {
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    const float h = height();

    // Pin it to the bottom edge, full width.
    ImGui::SetNextWindowPos(ImVec2(0, display.y - h));
    ImGui::SetNextWindowSize(ImVec2(display.x, h));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ColorConvertU32ToFloat4(Theme::PanelDark));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 0));

    ImGui::Begin("##Taskbar", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse |
                 ImGuiWindowFlags_NoFocusOnAppearing);

    // Keep the taskbar above every app window, every frame
    // (but not above an open pop-up, or it would cover it).
    if (!ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel)) {
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
    }

    drawLogo();
    drawAppButtons();
    drawTray();
    drawShutdownConfirm();

    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

// A circular button drawn by hand: dark circle, lighter on hover, label inside.
// Returns true on the frame it is clicked (immediate mode!).
bool Taskbar::roundButton(const char* id, const char* label, unsigned int labelColor, float diameter) {
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const bool clicked = ImGui::InvisibleButton(id, ImVec2(diameter, diameter));
    const bool hovered = ImGui::IsItemHovered();
    const bool held = ImGui::IsItemActive();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 center(pos.x + diameter * 0.5f, pos.y + diameter * 0.5f);
    const ImU32 fill = held ? IM_COL32(60, 70, 82, 255)
                     : hovered ? IM_COL32(45, 53, 63, 255)
                               : IM_COL32(30, 36, 44, 255);
    dl->AddCircleFilled(center, diameter * 0.5f, fill);
    if (hovered) dl->AddCircle(center, diameter * 0.5f, labelColor, 0, 1.5f);

    const float size = config_.fontSize * 0.85f;
    const ImVec2 ts = Theme::BoldFont->CalcTextSizeA(size, FLT_MAX, 0.0f, label);
    dl->AddText(Theme::BoldFont, size, ImVec2(center.x - ts.x * 0.5f, center.y - ts.y * 0.5f),
                labelColor, label);
    return clicked;
}

void Taskbar::drawLogo() {
    const float h = height();
    const float logoSize = 44.0f;
    ImGui::SetCursorPos(ImVec2(12, (h - logoSize) * 0.5f));
    roundButton("##logo", "OS", Theme::Cyan, logoSize);
    tooltipAbove("CSOPESY OS v1.0", 0.0f);

    // Separator line after the logo
    const ImVec2 p = ImGui::GetItemRectMax();
    ImGui::GetWindowDrawList()->AddLine(ImVec2(p.x + 10, p.y - logoSize + 6), ImVec2(p.x + 10, p.y - 6),
                                        Theme::Border, 1.5f);

}

// ---------------------------------------------------------------------------
// App buttons
// ---------------------------------------------------------------------------
void Taskbar::drawAppButtons() {
    // The three required buttons: two unique apps + the Task Manager.
    static const AppButton apps[] = {
        { "explorer", "File Explorer", Icons::folder },
        { "notepad",  "Notepad",       Icons::document },
        { "taskmgr",  "Task Manager",  Icons::chart },
    };

    const float size = 46.0f;
    const float y = (height() - size) * 0.5f;
    float x = 12 + 44 + 24;   // after the logo and separator

    for (const AppButton& app : apps) {
        ImGui::SetCursorPos(ImVec2(x, y));
        if (appButton(app, size)) {
            UIManager::instance().activate(app.windowId);
        }
        x += size + 8;
    }
}

bool Taskbar::appButton(const AppButton& app, float size) {
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::PushID(app.windowId);
    const bool clicked = ImGui::InvisibleButton("##app", ImVec2(size, size));
    ImGui::PopID();
    const bool hovered = ImGui::IsItemHovered();
    const bool held = ImGui::IsItemActive();

    UIManager& ui = UIManager::instance();
    const bool open = ui.isShown(app.windowId);
    const bool focused = ui.isActive(app.windowId);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 max(pos.x + size, pos.y + size);
    if (held || hovered || focused) {
        dl->AddRectFilled(pos, max, held ? IM_COL32(60, 70, 82, 255)
                                  : focused ? IM_COL32(40, 52, 64, 255)
                                            : IM_COL32(45, 53, 63, 255), 8.0f);
    }
    app.drawIcon(dl, ImVec2(pos.x + size * 0.5f, pos.y + size * 0.47f), size * 0.62f);

    // "Running" indicator: short grey bar if open, long cyan bar if in front.
    if (open) {
        const float barW = focused ? size * 0.5f : size * 0.22f;
        const float cx = pos.x + size * 0.5f;
        dl->AddRectFilled(ImVec2(cx - barW * 0.5f, max.y - 3), ImVec2(cx + barW * 0.5f, max.y),
                          focused ? Theme::Cyan : Theme::TextMuted, 2.0f);
    }

    const char* state = !open ? "" : ui.isMinimized(app.windowId) ? " (minimized)" : " (open)";
    const std::string tip = std::string(app.tooltip) + state;
    tooltipAbove(tip.c_str());
    return clicked;
}

void Taskbar::drawTray() {
    const float h = height();
    const float btn = 44.0f;
    const float right = ImGui::GetWindowWidth() - 12;

    // Frame rate: proof that the screen really redraws every frame.
    char fps[32];
    std::snprintf(fps, sizeof(fps), "%.0f FPS", ImGui::GetIO().Framerate);
    const float fpsWidth = ImGui::CalcTextSize("000 FPS").x;
    ImGui::SetCursorPos(ImVec2(right - btn - 16 - fpsWidth, (h - ImGui::GetTextLineHeight()) * 0.5f));
    ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(Theme::TextMuted), "%s", fps);

    // PWR: the ONLY intended way to shut the OS down.
    ImGui::SetCursorPos(ImVec2(right - btn, (h - btn) * 0.5f));
    if (roundButton("##pwr", "PWR", Theme::Red, btn)) {
        askShutdown_ = true;      // ask first (see drawShutdownConfirm)
    }
    tooltipAbove("Shut down CSOPESY OS", 1.0f);
}

// ---------------------------------------------------------------------------
// Shutdown confirmation
// ---------------------------------------------------------------------------
void Taskbar::drawShutdownConfirm() {
    const char* popupId = "Shut Down##confirm";
    if (askShutdown_) {
        ImGui::OpenPopup(popupId);
        askShutdown_ = false;
    }

    // A MODAL pop-up: everything else is blocked (and dimmed) until it is answered.
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(display.x * 0.5f, display.y * 0.45f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 12));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
    if (ImGui::BeginPopupModal(popupId, nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
                               ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::TextUnformatted("Are you sure you want to shut down CSOPESY OS?");
        ImGui::Spacing();
        ImGui::Spacing();

        const ImVec2 buttonSize(130, 0);
        if (ImGui::Button("Shut Down", buttonSize)) {
            UIManager::instance().requestPowerOff();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", buttonSize) || ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar(2);
}
