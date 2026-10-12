// Texture.cpp
#include "Texture.h"

// stb_image is a single-header library: exactly ONE .cpp file defines this
// before including it, which turns the header into the actual code.
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#include "stb_image.h"

#include <GLFW/glfw3.h>   // also brings in the basic OpenGL functions (gl.h)

bool loadTexture(const std::string& path, Texture& out, std::string& error) {
    int width = 0, height = 0, channels = 0;

    // Decode the file into raw pixels. The last argument (4) asks for RGBA.
    unsigned char* pixels = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!pixels) {
        const char* reason = stbi_failure_reason();
        if (!reason) error = "unknown error";
        else if (std::string(reason) == "can't fopen") error = "file not found";
        else error = reason;   // e.g. "unknown image type" for an unsupported format
        return false;
    }

    // Upload the pixels to the graphics card.
    GLuint id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  // smooth when shrunk
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  // smooth when enlarged
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(pixels);   // the graphics card has its own copy now

    out.glId = id;
    out.width = width;
    out.height = height;
    return true;
}

void freeTexture(Texture& texture) {
    if (texture.glId != 0) {
        GLuint id = texture.glId;
        glDeleteTextures(1, &id);
    }
    texture = Texture{};
}
