#ifndef SOLARSYSTEM_SHADOWMAPFBO_H
#define SOLARSYSTEM_SHADOWMAPFBO_H
#include <GL/glew.h>
#include <vector>

class ShadowMapFBO {
public:
    explicit ShadowMapFBO(uint16_t shadowMapWidth, uint16_t shadowMapHeight);
    uint16_t GetShadowMapWidth() const;
    uint16_t GetShadowMapHeight() const;
    GLuint GetShadowMap() const;
    GLuint GetFBO() const;

private:
    uint16_t _shadowMapWidth, _shadowMapHeight;
    GLuint _shadowMap = 0, _frameBuffer = 0;

    void InitFBO();
};

#endif //SOLARSYSTEM_SHADOWMAPFBO_H
