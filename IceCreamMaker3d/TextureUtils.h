// TextureUtils.h
#pragma once
#include <string>
#include <GL/glew.h>

GLuint LoadTexture2D(const std::string& path, bool srgb = false, bool flipY = true);
