#pragma once
// Theme.h
// ---------------------------------------------------------------------------
// Colours and fonts shared by every part of the desktop, so the whole OS
// looks consistent. Change a colour here and it changes everywhere.
// ---------------------------------------------------------------------------
#include "imgui.h"

namespace Theme {

    // ---- Palette (dark "terminal" look, same accents as MO3) ----
    constexpr ImU32 Text       = IM_COL32(230, 237, 243, 255);
    constexpr ImU32 TextMuted  = IM_COL32(139, 148, 158, 255);
    constexpr ImU32 Cyan       = IM_COL32( 57, 197, 207, 255);
    constexpr ImU32 Green      = IM_COL32( 63, 185,  80, 255);
    constexpr ImU32 Yellow     = IM_COL32(227, 179,  65, 255);
    constexpr ImU32 Red        = IM_COL32(248,  81,  73, 255);
    constexpr ImU32 PanelDark  = IM_COL32( 13,  17,  23, 235);   // taskbar
    constexpr ImU32 PanelMid   = IM_COL32( 22,  27,  34, 200);   // clock box
    constexpr ImU32 Border     = IM_COL32( 48,  54,  61, 255);

    // Fonts loaded at startup (see GUIApplication). Bold falls back to regular.
    extern ImFont* RegularFont;
    extern ImFont* BoldFont;

    // Applies the colours/rounding to ImGui's own widgets (windows, buttons...).
    void applyStyle();

} // namespace Theme
