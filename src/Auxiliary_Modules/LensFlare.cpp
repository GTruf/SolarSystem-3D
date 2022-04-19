#include "LensFlare.h"

LensFlare::LensFlare(const Shader& shader, const TextureImage2D& lensTexture, const FlaresInfo& properties) : _lensFlareShader(shader), _lensTexture(lensTexture) {
    _numSprites = properties.sprites.size();

    constexpr glm::vec2 positions[4] = {
            glm::vec2(-1.0f, 1.0f),
            glm::vec2(-1.0f, -1.0f),
            glm::vec2(1.0f, -1.0f),
            glm::vec2(1.0f, 1.0f)
    };

    constexpr glm::vec2 uvs[4] = {
            glm::vec2(0.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f)
    };

    size_t xSprites = properties.spritesPerRow;
    size_t spriteDims = _lensTexture.GetWidth() / xSprites;
    if (spriteDims == 0)
        throw std::logic_error("Lens flare has sprite dims of 0!");
    // ui32 ySprites = m_texHeight / spriteDims;
    glm::vec2 uvSprite(spriteDims / static_cast<float>(_lensTexture.GetWidth()), spriteDims / static_cast<float>(_lensTexture.GetHeight()));

    constexpr uint16_t quadIndices[INDICES_PER_QUAD] = {0, 1, 2, 2, 3, 0};

    std::vector<FlareVertex> vertices(_numSprites * VERTS_PER_QUAD);
    std::vector<uint16_t> indices(_numSprites * INDICES_PER_QUAD);
    int index = 0;

    for (size_t i = 0; i < _numSprites; i++) {
        FlareSprite sprite = properties.sprites[i];
        glm::vec2 uvOffset = glm::vec2(sprite.textureIndex % xSprites, sprite.textureIndex / xSprites) * uvSprite;

        vertices[index].position = positions[0] * sprite.size;
        vertices[index + 1].position = positions[1] * sprite.size;
        vertices[index + 2].position = positions[2] * sprite.size;
        vertices[index + 3].position = positions[3] * sprite.size;
        vertices[index].uv = uvs[0] * uvSprite + uvOffset;
        vertices[index + 1].uv = uvs[1] * uvSprite + uvOffset;
        vertices[index + 2].uv = uvs[2] * uvSprite + uvOffset;
        vertices[index + 3].uv = uvs[3] * uvSprite + uvOffset;
        vertices[index].offset = sprite.offset;
        vertices[index + 1].offset = sprite.offset;
        vertices[index + 2].offset = sprite.offset;
        vertices[index + 3].offset = sprite.offset;
        index += 4;
    }
    // Set indices
    for (size_t i = 0; i < _numSprites; i++) {
        for (size_t j = 0; j < INDICES_PER_QUAD; j++) {
            indices[i * INDICES_PER_QUAD + j] = quadIndices[j] + i * VERTS_PER_QUAD;
        }
    }

    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ebo);

    glBindVertexArray(_vao); // Связывание с вершинным массивом
    glBindBuffer(GL_ARRAY_BUFFER, _vbo); // Связывание с вершинным буфером
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(FlareVertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo); // Связывание с элементным буфером (копируем индексы)
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);

    // Установка указателей вершинных атрибутов (указание параметров доступа вершинных атрибутов к VBO)
    // Позиции вершин
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(FlareVertex), (void*)offsetof(FlareVertex, position));
    // Текстурные координаты
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(FlareVertex), (void*)offsetof(FlareVertex, uv));
    // Смещение бликов
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(FlareVertex), (void*)offsetof(FlareVertex, offset));

    glBindVertexArray(0);
}

void LensFlare::Render(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& center, const glm::vec3& color, float aspectRatio, float size,
                       float intensity, const std::optional<RingCameraInfo>& ringCameraInfo) const
{
    if (size <= 0.0f || intensity <= 0.0f)
        return;

    glm::vec2 dims(size, size * aspectRatio);

    _lensFlareShader.Use();
    _lensFlareShader.SetMat4("projection", projection);
    _lensFlareShader.SetMat4("view", view);
    _lensFlareShader.SetVec3("center", center);
    _lensFlareShader.SetVec3("color", color);
    _lensFlareShader.SetVec2("dims", dims);
    _lensFlareShader.SetFloat("intensity", intensity);
    _lensFlareShader.SetInt("lensTexture", 0);
    _lensFlareShader.SetInt("ringDiffuse", 1);
    glBindTextureUnit(0, _lensTexture.GetTexture());

    _lensFlareShader.SetBool("isPlanetaryRingInView", ringCameraInfo.has_value());
    if (ringCameraInfo) {
        _lensFlareShader.SetVec3("cameraPosition", ringCameraInfo->cameraPosition);
        _lensFlareShader.SetVec3("ringCenter", ringCameraInfo->ringCenter);
        _lensFlareShader.SetVec3("ringNormal", ringCameraInfo->ringNormal);
        _lensFlareShader.SetVec2("ringInnerOuterRadiuses", ringCameraInfo->ringInnerOuterRadiuses);
        glBindTextureUnit(1, ringCameraInfo->ringDiffuse);
    }

    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, INDICES_PER_QUAD * _numSprites, GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);
}