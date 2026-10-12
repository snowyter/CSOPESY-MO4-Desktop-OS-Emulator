#pragma once
// Texture.h
// ---------------------------------------------------------------------------
// Loads an image file (jpg/png/bmp) from disk into the graphics card as an
// OpenGL texture, so ImGui can draw it (used for the wallpaper).
// ---------------------------------------------------------------------------
#include <string>

#include "imgui.h"

struct Texture {
    unsigned int glId = 0;      // OpenGL's number for this texture (0 = none)
    int width = 0;
    int height = 0;

    bool valid() const { return glId != 0; }
    ImTextureID imguiId() const { return static_cast<ImTextureID>(glId); }
};

// Returns true on success. On failure, `error` says why (e.g. file not found).
bool loadTexture(const std::string& path, Texture& out, std::string& error);

// Frees the texture's memory on the graphics card.
void freeTexture(Texture& texture);
