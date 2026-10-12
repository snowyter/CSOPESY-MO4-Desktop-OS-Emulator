#pragma once
// Icons.h
// ---------------------------------------------------------------------------
// Small icons drawn with ImGui draw commands (rectangles, lines, circles),
// so the project needs no icon image files. Each function draws an icon
// centred on `c`, fitting in a square of side `s`.
// Used by the taskbar and by File Explorer's file list.
// ---------------------------------------------------------------------------
#include "imgui.h"

namespace Icons {
    void folder(ImDrawList* dl, ImVec2 c, float s);
    void document(ImDrawList* dl, ImVec2 c, float s);     // text file / Notepad
    void chart(ImDrawList* dl, ImVec2 c, float s);        // Task Manager
    void image(ImDrawList* dl, ImVec2 c, float s);        // picture file
    void drive(ImDrawList* dl, ImVec2 c, float s);        // disk (This PC)
}
