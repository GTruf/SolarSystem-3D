#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<size_t> indices, std::vector<Texture> textures)
    : _vao(0), _vbo(0), _ebo(0), _vertices(std::move(vertices)), _indices(std::move(indices)), _textures(std::move(textures))
{
    SetupMesh();
}

// Отрисовка (рендеринг) меша
void Mesh::Draw(const Shader& shader) const {
    // Not used
    size_t diffuseNumber = 1;
    size_t specularNumber = 1;
    size_t normalNumber = 1;
    size_t heightNumber = 1;

    for(size_t i = 0; i < _textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // Активировать текстурный блок перед связкой
        std::string number;
        const std::string name = _textures[i].type;
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNumber++);
        else if (name == "texture_specular")
            number = std::to_string(specularNumber++);
        else if (name == "texture_normal")
            number = std::to_string(normalNumber++);
        else if (name == "texture_height")
            number = std::to_string(heightNumber++);

        // Устанавливаем сэмплер на правильный текстурный блок
        glUniform1i(glGetUniformLocation(shader.GetProgramId(), (name + number).c_str()), i);
        glBindTexture(GL_TEXTURE_2D, _textures[i].id); // Связываем текстуру
    }

    // Непосредственная отрисовка меша
    glBindVertexArray(_vao); // Связывание с вершинным массивом
    glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, nullptr); // Отрисовка меша при помощи треугольников
    glBindVertexArray(0); // Отвязывание вершинного массива

    // Возврат к значению по умолчанию
    glActiveTexture(GL_TEXTURE0);
}

void Mesh::SetupMesh() {
    // Генерация буферов / массивов
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ebo);

    glBindVertexArray(_vao); // Связывание с вершинным массивом
    glBindBuffer(GL_ARRAY_BUFFER, _vbo); // Связывание с вершинным буфером

    // Копируем в вершинный буфер вершины и указываем о статической обработке данных видеокартой
    glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(Vertex), _vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo); // Связывание с элементным буфером (копируем индексы)
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(size_t), _indices.data(), GL_STATIC_DRAW);

    // Установка указателей вершинных атрибутов (указание параметров доступа вершинных атрибутов к VBO)
    // Позиции вершин
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)nullptr);
    // Нормали вершин
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // Координаты вершинных текстур
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, textureCoords));
    // Тангент к вершинам
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    // Битангент к вершинам
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));

    glBindVertexArray(0);
}
