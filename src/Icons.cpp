// Icons.cpp
#include "Icons.h"

namespace Icons {

void folder(ImDrawList* dl, ImVec2 c, float s) {
    const float w = s * 0.86f, h = s * 0.64f;
    const ImVec2 p0(c.x - w * 0.5f, c.y - h * 0.5f);
    const ImVec2 p1(c.x + w * 0.5f, c.y + h * 0.5f);
    const float r = s * 0.06f;
    // back tab
    dl->AddRectFilled(ImVec2(p0.x, p0.y - h * 0.14f), ImVec2(p0.x + w * 0.42f, p0.y + h * 0.2f),
                      IM_COL32(214, 150, 30, 255), r);
    // body
    dl->AddRectFilled(p0, p1, IM_COL32(245, 190, 60, 255), r);
    // front flap, slightly darker at the bottom
    dl->AddRectFilled(ImVec2(p0.x, p0.y + h * 0.22f), p1, IM_COL32(252, 205, 80, 255), r);
}

void document(ImDrawList* dl, ImVec2 c, float s) {
    const float w = s * 0.62f, h = s * 0.8f;
    const ImVec2 p0(c.x - w * 0.5f, c.y - h * 0.5f);
    const ImVec2 p1(c.x + w * 0.5f, c.y + h * 0.5f);
    dl->AddRectFilled(p0, p1, IM_COL32(236, 240, 245, 255), s * 0.06f);
    // text lines
    for (int i = 0; i < 4; ++i) {
        const float y = p0.y + h * (0.24f + 0.17f * i);
        const float len = (i == 3) ? 0.45f : 0.72f;
        dl->AddLine(ImVec2(p0.x + w * 0.16f, y), ImVec2(p0.x + w * (0.16f + len), y),
                    IM_COL32(70, 130, 200, 255), s * 0.05f);
    }
}

void chart(ImDrawList* dl, ImVec2 c, float s) {
    const float w = s * 0.86f, h = s * 0.66f;
    const ImVec2 p0(c.x - w * 0.5f, c.y - h * 0.5f);
    const ImVec2 p1(c.x + w * 0.5f, c.y + h * 0.5f);
    dl->AddRectFilled(p0, p1, IM_COL32(16, 60, 90, 255), s * 0.08f);
    // a CPU-graph style line
    const ImVec2 pts[] = {
        ImVec2(p0.x + w * 0.08f, p0.y + h * 0.72f), ImVec2(p0.x + w * 0.28f, p0.y + h * 0.48f),
        ImVec2(p0.x + w * 0.44f, p0.y + h * 0.62f), ImVec2(p0.x + w * 0.62f, p0.y + h * 0.24f),
        ImVec2(p0.x + w * 0.78f, p0.y + h * 0.44f), ImVec2(p0.x + w * 0.92f, p0.y + h * 0.34f),
    };
    dl->AddPolyline(pts, 6, IM_COL32(57, 197, 207, 255), ImDrawFlags_None, s * 0.07f);
}

void image(ImDrawList* dl, ImVec2 c, float s) {
    const float w = s * 0.8f, h = s * 0.62f;
    const ImVec2 p0(c.x - w * 0.5f, c.y - h * 0.5f);
    const ImVec2 p1(c.x + w * 0.5f, c.y + h * 0.5f);
    dl->AddRectFilled(p0, p1, IM_COL32(110, 180, 235, 255), s * 0.05f);
    dl->AddTriangleFilled(ImVec2(p0.x, p1.y), ImVec2(p0.x + w * 0.4f, p0.y + h * 0.4f),
                          ImVec2(p0.x + w * 0.8f, p1.y), IM_COL32(60, 150, 70, 255));
    dl->AddCircleFilled(ImVec2(p0.x + w * 0.75f, p0.y + h * 0.3f), s * 0.08f, IM_COL32(255, 230, 120, 255));
}

void drive(ImDrawList* dl, ImVec2 c, float s) {
    const float w = s * 0.86f, h = s * 0.42f;
    const ImVec2 p0(c.x - w * 0.5f, c.y - h * 0.5f);
    const ImVec2 p1(c.x + w * 0.5f, c.y + h * 0.5f);
    dl->AddRectFilled(p0, p1, IM_COL32(150, 160, 172, 255), s * 0.06f);
    dl->AddRectFilled(ImVec2(p0.x + w * 0.08f, p1.y - h * 0.32f), ImVec2(p1.x - w * 0.08f, p1.y - h * 0.18f),
                      IM_COL32(60, 200, 120, 255), s * 0.02f);
}

} // namespace Icons
