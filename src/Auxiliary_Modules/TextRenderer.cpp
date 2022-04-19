#include "TextRenderer.h"

TextRenderer::TextRenderer(FT_Library ft, const std::string& fontPath) : _ft(ft){
    LoadFont(fontPath);
    InitBuffers();
}

void TextRenderer::Render(const Shader& shader, const std::wstring& text, float x, float y, float scale, const glm::vec3& color) {
    shader.Use();
    shader.SetVec3("textColor", color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(_vao);

    for (const auto& c : text) {
        Character character = _characters.at(c);

        float xPos = x + character.bearing.x * scale;
        float yPos = y - (character.glyphSize.y - character.bearing.y) * scale;

        float w = character.glyphSize.x * scale;
        float h = character.glyphSize.y * scale;


        float vertices[6][4] = {
                { xPos, yPos + h, 0.0f, 0.0f },
                { xPos, yPos, 0.0f, 1.0f },
                { xPos + w, yPos, 1.0f, 1.0f },

                { xPos, yPos + h, 0.0f, 0.0f },
                { xPos + w, yPos, 1.0f, 1.0f },
                { xPos + w, yPos + h, 1.0f, 0.0f }
        };


        glBindTexture(GL_TEXTURE_2D, character.textureID);


        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);


        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Производим смещение для отображения следующего глифа (продвижение на 1/64 пикселя)
        x += (character.advance >> 6) * scale; // побитовый сдвиг на 6, чтобы получить значение в пикселях (2^6 = 64)
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}


void TextRenderer::Render(const Shader& shader, const std::deque<wchar_t>& text, float x, float y, float scale, const glm::vec3& color) {
    shader.Use();
    shader.SetVec3("textColor", color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(_vao);

    for (const auto& c : text) {
        Character character = _characters.at(c);

        float xPos = x + character.bearing.x * scale;
        float yPos = y - (character.glyphSize.y - character.bearing.y) * scale;

        float w = character.glyphSize.x * scale;
        float h = character.glyphSize.y * scale;

        // Обновляем VBO для каждого символа
        float vertices[6][4] = {
                { xPos, yPos + h, 0.0f, 0.0f },
                { xPos, yPos, 0.0f, 1.0f },
                { xPos + w, yPos, 1.0f, 1.0f },

                { xPos, yPos + h, 0.0f, 0.0f },
                { xPos + w, yPos, 1.0f, 1.0f },
                { xPos + w, yPos + h, 1.0f, 0.0f }
        };

        glBindTexture(GL_TEXTURE_2D, character.textureID);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Производим смещение для отображения следующего глифа (продвижение на 1/64 пикселя)
        x += (character.advance >> 6) * scale; // Побитовый сдвиг на 6, чтобы получить значение в пикселях (2^6 = 64)
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextRenderer::Render(const Shader &shader, const std::deque<std::wstring> &text, float x, float y, float scale, const glm::vec3 &color) {
    shader.Use();
    shader.SetVec3("textColor", color);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(_vao);

    for (const auto& string : text) {
        for (const auto& c : string) {
            Character character = _characters.at(c);

            float xPos = x + character.bearing.x * scale;
            float yPos = y - (character.glyphSize.y - character.bearing.y) * scale;

            float w = character.glyphSize.x * scale;
            float h = character.glyphSize.y * scale;

            // Обновляем VBO для каждого символа
            float vertices[6][4] = {
                    {xPos,     yPos + h, 0.0f, 0.0f},
                    {xPos,     yPos,     0.0f, 1.0f},
                    {xPos + w, yPos,     1.0f, 1.0f},

                    {xPos,     yPos + h, 0.0f, 0.0f},
                    {xPos + w, yPos,     1.0f, 1.0f},
                    {xPos + w, yPos + h, 1.0f, 0.0f}
            };

            glBindTexture(GL_TEXTURE_2D, character.textureID);

            glBindBuffer(GL_ARRAY_BUFFER, _vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Производим смещение для отображения следующего глифа (продвижение на 1/64 пикселя)
            x += (character.advance >> 6) * (scale); // Побитовый сдвиг на 6, чтобы получить значение в пикселях (2^6 = 64)
        }
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextRenderer::ReverseRender(const Shader& shader, const std::deque<std::wstring>& text, float x, float y, float scale, const glm::vec3& color) {
    shader.Use();
    shader.SetVec3("textColor", color);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(_vao);

    for (const auto& string : text) {
        for (const auto& c : std::wstring(string.rbegin(), string.rend())) {
            Character character = _characters.at(c);

            float xPos = x + (character.bearing.x - character.glyphSize.x) * scale;
            float yPos = y - (character.glyphSize.y - character.bearing.y) * scale;

            float w = character.glyphSize.x * scale;
            float h = character.glyphSize.y * scale;

            // Обновляем VBO для каждого символа
            float vertices[6][4] = {
                    {xPos,     yPos + h, 0.0f, 0.0f},
                    {xPos,     yPos,     0.0f, 1.0f},
                    {xPos + w, yPos,     1.0f, 1.0f},

                    {xPos,     yPos + h, 0.0f, 0.0f},
                    {xPos + w, yPos,     1.0f, 1.0f},
                    {xPos + w, yPos + h, 1.0f, 0.0f}
            };

            glBindTexture(GL_TEXTURE_2D, character.textureID);

            glBindBuffer(GL_ARRAY_BUFFER, _vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Производим смещение для отображения следующего глифа (продвижение на 1/64 пикселя)
            x -= (character.advance >> 6) * (scale); // Побитовый сдвиг на 6, чтобы получить значение в пикселях (2^6 = 64)
        }
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextRenderer::LoadFont(const std::string& fontPath) {
    FT_Face face;
    if (FT_New_Face(_ft, fontPath.c_str(), 0, &face))
        throw std::runtime_error("ERROR::FREETYPE: Failed to load font " + fontPath);

    FT_Set_Pixel_Sizes(face, 0, 48);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Отключаем ограничение выравнивания байтов

    for (size_t c = 0; c < 1151; c++) {
        // Загружаем глифы символов
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            throw std::runtime_error("ERROR::FREETYTPE: Failed to load Glyph");

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                face->glyph->advance.x
        };

        _characters.emplace(c, character);
    }

    FT_Done_Face(face);
}

void TextRenderer::InitBuffers() {
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
