#include "TextureImage2D.h"

TextureImage2D::TextureImage2D(const std::string& path, GLint wrapParam, GLint minFilter, GLint magFilter) {
    LoadTextureFromFile(path, wrapParam, minFilter, magFilter);
}

void TextureImage2D::LoadTextureFromFile(const std::string& path, GLint wrapParam, GLint minFilter, GLint magFilter) {
    glGenTextures(1, &_textureID);
    glBindTexture(GL_TEXTURE_2D, _textureID);

    try {
        CDDSImage image;
        image.load(path, false);
        image.upload_texture2D();
        _width = image.get_width();
        _height = image.get_height();
    }
    catch (const std::runtime_error& error) {
        throw std::runtime_error("Image " + path + " cannot be loaded");
    }

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapParam);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapParam);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
    std::cout << path << " Loaded" << std::endl;
}

GLuint TextureImage2D::GetTexture() const {
    return _textureID;
}

GLuint TextureImage2D::GetWidth() const {
    return _width;
}

GLuint TextureImage2D::GetHeight() const {
    return _height;
}