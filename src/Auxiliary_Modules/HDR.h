#ifndef SOLARSYSTEM_HDR_H
#define SOLARSYSTEM_HDR_H
#include "Shader.h"

class HDR {
public:
    explicit HDR(const Shader& shader, uint16_t width, uint16_t height);
    void Render(float exposure, float gamma) const;
    GLuint GetHdrFBO() const;

private:
    Shader _hdrShader;
    GLuint _quadVao = 0, _quadVbo = 0, _hdrFrameBuffer = 0, _colorBuffer = 0, _rboDepth = 0;

    void InitQuadBuffers();
    void InitFBO(uint16_t width, uint16_t height);
};

#endif //SOLARSYSTEM_HDR_H
