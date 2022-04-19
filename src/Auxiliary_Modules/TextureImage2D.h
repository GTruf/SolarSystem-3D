#ifndef SOLARSYSTEM_TEXTUREIMAGE2D_H
#define SOLARSYSTEM_TEXTUREIMAGE2D_H
#include "../../src/3rdparty/nv_dds.h"
#include <iostream>
#include <string>
#include <mutex>
#include <memory>
#include <thread>

using namespace nv_dds;

class TextureImage2D {
public:
    TextureImage2D() = default;
    explicit TextureImage2D(const std::string& path, GLint wrapParam = GL_REPEAT, GLint minFilter = GL_LINEAR_MIPMAP_LINEAR, GLint magFilter = GL_LINEAR_MIPMAP_LINEAR);
    ~TextureImage2D() = default;
    GLuint GetTexture() const;
    GLuint GetWidth() const;
    GLuint GetHeight() const;

private:
    GLuint _textureID = 0;
    GLuint _width = 0, _height = 0;

    void LoadTextureFromFile(const std::string& path, GLint wrapParam, GLint minFilter, GLint magFilter);
};

#endif //SOLARSYSTEM_TEXTUREIMAGE2D_H
