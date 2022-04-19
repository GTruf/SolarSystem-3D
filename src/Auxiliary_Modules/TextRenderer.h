#ifndef SOLARSYSTEM_TEXTRENDERER_H
#define SOLARSYSTEM_TEXTRENDERER_H
#include "Shader.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <deque>
#include <map>

struct Character {
    GLuint textureID;       // ID текстуры глифа
    glm::ivec2 glyphSize;   // Размер глифа
    glm::ivec2 bearing;     // Смещение от линии шрифта до верхнего/левого угла глифа
    int advance;            // Смещение до следующего глифа
};

class TextRenderer {
public:
    explicit TextRenderer(FT_Library ft,  const std::string& fontPath);
    void Render(const Shader& shader, const std::wstring& text, float x, float y, float scale, const glm::vec3& color);
    void Render(const Shader& shader, const std::deque<wchar_t>& text, float x, float y, float scale, const glm::vec3& color);
    void Render(const Shader& shader, const std::deque<std::wstring>& text, float x, float y, float scale, const glm::vec3& color);
    void ReverseRender(const Shader& shader, const std::deque<std::wstring>& text, float x, float y, float scale, const glm::vec3& color);

private:
    FT_Library _ft;
    std::map<wchar_t, Character> _characters;
    GLuint _vao = 0; // Объект вершинного массива (VAO)
    GLuint _vbo = 0; // Объект вершинного буфера (VBO)

    void LoadFont(const std::string& path);
    void InitBuffers();
};

#endif //SOLARSYSTEM_TEXTRENDERER_H
