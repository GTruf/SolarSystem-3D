#ifndef SOLARSYSTEM_SKYBOX_H
#define SOLARSYSTEM_SKYBOX_H
#include "../Auxiliary_Modules/Shader.h"
#include "../../src/3rdparty//nv_dds.h"
#include <vector>

using namespace nv_dds;

class SkyBox {
public:
    explicit SkyBox(const std::vector<std::string>& faces);
    void Render(const Shader& shader) const;

private:
    GLuint _textureID = 0;
    GLuint _vao = 0, _vbo = 0;

    void InitBuffers();
    void LoadCubeMap(const std::vector<std::string>& faces);
};

#endif //SOLARSYSTEM_SKYBOX_H
