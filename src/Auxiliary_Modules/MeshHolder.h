#ifndef SOLARSYSTEM_MESHKEEPER_H
#define SOLARSYSTEM_MESHKEEPER_H
#include "Shader.h"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

size_t TextureFromFile(const std::string& path); // Not used at all

class MeshHolder {
public:
    explicit MeshHolder(const std::string& path);
    void Draw(const Shader& shader) const; // Отрисовка модели (мешей)

private:
    std::vector<Texture> _loadedTextures;
    std::vector<Mesh> _meshes;
    std::string _directory;

    // Обрабатывает узел рекурсивно. Обрабатывает каждую отдельную сетку, расположенную в узле, и повторяет этот процесс на своих дочерних узлах (если есть).
    void ProcessNode(aiNode* node, const aiScene* scene);
    Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, const aiTextureType& type, const std::string& typeName);
};

#endif //SOLARSYSTEM_MESHKEEPER_H
