// TextureUtils.cpp
#include "stb_image.h"
#include "TextureUtils.h"
#include <iostream>

GLuint LoadTexture2D(const std::string& path, bool srgb, bool flipY)
{
    stbi_set_flip_vertically_on_load(flipY);

    int w, h, n;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &n, 0);
    if (!data)
    {
        std::cerr << "[LoadTexture2D] Failed: " << path << "\n";
        return 0;
    }

    GLenum format = GL_RGB;
    GLenum internalFormat = GL_RGB8;

    if (n == 1) { format = GL_RED; internalFormat = GL_R8; }
    else if (n == 3) { format = GL_RGB; internalFormat = srgb ? GL_SRGB8 : GL_RGB8; }
    else if (n == 4) { format = GL_RGBA; internalFormat = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8; }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
    return tex;
}
