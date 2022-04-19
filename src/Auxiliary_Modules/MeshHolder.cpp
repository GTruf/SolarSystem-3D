#include "MeshHolder.h"

MeshHolder::MeshHolder(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::runtime_error("ERROR::ASSIMP:: " + std::string(importer.GetErrorString()));
    }

    _directory = path.substr(0, path.find_last_of('/'));

    ProcessNode(scene->mRootNode, scene);
}

void MeshHolder::Draw(const Shader& shader) const {
    for(const auto& mesh : _meshes)
        mesh.Draw(shader);
}

void MeshHolder::ProcessNode(aiNode* node, const aiScene* scene) {
    // Обрабатываем каждую сетку, расположенную в текущем узле
    for(size_t i = 0; i < node->mNumMeshes; i++) {
        // Обработать все полигональные сетки в узле(если есть)
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        _meshes.push_back(ProcessMesh(mesh, scene));
    }
    // Выполнить ту же обработку и для каждого потомка узла
    for(size_t i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene);
    }
}

Mesh MeshHolder::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<size_t> indices;
    std::vector<Texture> textures;

    // Обход каждой вершины меша
    for(size_t i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;
        //std::cout << "_position: " << vertex.position.x << " " << vertex.position.y << " " << vertex.position.z << std::endl;

        if (mesh->HasNormals()) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;
            //std::cout << "Normal: " << vertex.normal.x << " " << vertex.normal.y << " " << vertex.normal.z << std::endl;
        }

        if(mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.textureCoords = vec;
            //std::cout << "TextureCoords: " << vertex.textureCoords.x << " " << vertex.textureCoords.y << std::endl;

            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.tangent = vector;
            //std::cout << "Tangent: " << vertex.tangent.x << " " << vertex.tangent.y << " " << vertex.tangent.z << std::endl;

            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.bitangent = vector;
            //std::cout << "Bitangent: " << vertex.bitangent.x << " " << vertex.bitangent.y << " " << vertex.bitangent.z << std::endl;
        }
        else
            vertex.textureCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }

    for(size_t i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for(size_t j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // 1. Диффузные карты
    std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    // 2. Отражательные карты
    std::vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. Карты нормалей
    std::vector<Texture> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    // 4. Карты высот
    std::vector<Texture> heightMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    return Mesh(vertices, indices, textures);
}

std::vector<Texture> MeshHolder::LoadMaterialTextures(aiMaterial* mat, const aiTextureType& type, const std::string& typeName) {
    std::vector<Texture> textures;
    for(size_t i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for(const auto& loadedTexture : _loadedTextures) {
            if(loadedTexture.path == str) {
                textures.push_back(loadedTexture);
                skip = true;
                break;
            }
        }
        if(!skip) { // Если текстура не загружалась - загружаем
            Texture texture;
            texture.id = TextureFromFile(str.C_Str());
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            _loadedTextures.push_back(texture);
        }
    }
    return textures;
}

size_t TextureFromFile(const std::string& path) {
    return 0;
}
