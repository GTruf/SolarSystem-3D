#ifndef SOLARSYSTEM_MESH_H
#define SOLARSYSTEM_MESH_H
#include "Shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/types.h>
#include <string>
#include <utility>
#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 textureCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

struct Texture {
    size_t id;
    std::string type;
    aiString path;
};

class Mesh {
public:
    explicit Mesh(std::vector<Vertex> vertices, std::vector<size_t> indices, std::vector<Texture> textures);
    void Draw(const Shader& shader) const; // Отрисовка (рендеринг) меша

private:
    std::vector<Vertex> _vertices; // Вершины
    std::vector<size_t> _indices; // Индексы
    std::vector<Texture> _textures; // Текстуры
    GLuint _vbo; // Объект вершинного буфера (VBO)
    GLuint _vao; // Объект вершинного массива (VAO)
    GLuint _ebo; // Объект элементного буфера (EBO)

    // Инициализация всех буферных объектов / массивов
    void SetupMesh();
};

#endif //SOLARSYSTEM_MESH_H
