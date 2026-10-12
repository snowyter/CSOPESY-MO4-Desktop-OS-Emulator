// Theme.cpp
#include "Theme.h"

namespace Theme {

ImFont* RegularFont = nullptr;
ImFont* BoldFont = nullptr;

void applyStyle() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding    = 8.0f;
    style.FrameRounding     = 5.0f;
    style.GrabRounding      = 5.0f;
    style.TabRounding       = 5.0f;
    style.ScrollbarRounding = 6.0f;
    style.WindowBorderSize  = 1.0f;
    style.WindowPadding     = ImVec2(12, 10);
    style.FramePadding      = ImVec2(8, 5);
    style.ItemSpacing       = ImVec2(8, 7);
    style.WindowTitleAlign  = ImVec2(0.0f, 0.5f);

    ImVec4* c = style.Colors;
    c[ImGuiCol_Text]             = ImVec4(0.90f, 0.93f, 0.95f, 1.00f);
    c[ImGuiCol_TextDisabled]     = ImVec4(0.55f, 0.58f, 0.62f, 1.00f);
    c[ImGuiCol_WindowBg]         = ImVec4(0.086f, 0.106f, 0.133f, 1.00f);
    c[ImGuiCol_ChildBg]          = ImVec4(0.05f, 0.067f, 0.09f, 1.00f);
    c[ImGuiCol_PopupBg]          = ImVec4(0.086f, 0.106f, 0.133f, 0.98f);
    c[ImGuiCol_Border]           = ImVec4(0.19f, 0.21f, 0.24f, 1.00f);
    c[ImGuiCol_TitleBg]          = ImVec4(0.05f, 0.067f, 0.09f, 1.00f);
    c[ImGuiCol_TitleBgActive]    = ImVec4(0.06f, 0.18f, 0.20f, 1.00f);
    c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.05f, 0.067f, 0.09f, 1.00f);
    c[ImGuiCol_FrameBg]          = ImVec4(0.12f, 0.15f, 0.19f, 1.00f);
    c[ImGuiCol_FrameBgHovered]   = ImVec4(0.16f, 0.20f, 0.25f, 1.00f);
    c[ImGuiCol_FrameBgActive]    = ImVec4(0.18f, 0.24f, 0.30f, 1.00f);
    c[ImGuiCol_Button]           = ImVec4(0.14f, 0.18f, 0.23f, 1.00f);
    c[ImGuiCol_ButtonHovered]    = ImVec4(0.13f, 0.36f, 0.40f, 1.00f);
    c[ImGuiCol_ButtonActive]     = ImVec4(0.16f, 0.50f, 0.55f, 1.00f);
    c[ImGuiCol_Header]           = ImVec4(0.13f, 0.30f, 0.34f, 1.00f);
    c[ImGuiCol_HeaderHovered]    = ImVec4(0.15f, 0.38f, 0.43f, 1.00f);
    c[ImGuiCol_HeaderActive]     = ImVec4(0.17f, 0.47f, 0.52f, 1.00f);
    c[ImGuiCol_Tab]              = ImVec4(0.12f, 0.15f, 0.19f, 1.00f);
    c[ImGuiCol_TabHovered]       = ImVec4(0.19f, 0.25f, 0.30f, 1.00f);
    c[ImGuiCol_TabSelected]      = ImVec4(0.13f, 0.32f, 0.36f, 1.00f);
    c[ImGuiCol_TabSelectedOverline] = ImVec4(0.22f, 0.77f, 0.81f, 1.00f);   // cyan line on the open tab
    c[ImGuiCol_TableHeaderBg]    = ImVec4(0.12f, 0.15f, 0.19f, 1.00f);
    c[ImGuiCol_TableRowBgAlt]    = ImVec4(1.00f, 1.00f, 1.00f, 0.03f);
    c[ImGuiCol_PlotLines]        = ImVec4(0.22f, 0.77f, 0.81f, 1.00f);
    c[ImGuiCol_PlotHistogram]    = ImVec4(0.89f, 0.70f, 0.25f, 1.00f);
    c[ImGuiCol_CheckMark]        = ImVec4(0.22f, 0.77f, 0.81f, 1.00f);
    c[ImGuiCol_SliderGrab]       = ImVec4(0.22f, 0.77f, 0.81f, 1.00f);
}

} // namespace Theme
