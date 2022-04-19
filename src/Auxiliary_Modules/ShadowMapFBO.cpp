#include "ShadowMapFBO.h"

ShadowMapFBO::ShadowMapFBO(uint16_t shadowMapWidth, uint16_t shadowMapHeight) : _shadowMapWidth(shadowMapWidth), _shadowMapHeight(shadowMapHeight) {
    InitFBO();
}

uint16_t ShadowMapFBO::GetShadowMapWidth() const {
    return _shadowMapWidth;
}

uint16_t ShadowMapFBO::GetShadowMapHeight() const {
    return _shadowMapHeight;
}

GLuint ShadowMapFBO::GetShadowMap() const {
    return _shadowMap;
}

GLuint ShadowMapFBO::GetFBO() const {
    return _frameBuffer;
}

void ShadowMapFBO::InitFBO() {
    glGenFramebuffers(1, &_frameBuffer);

    glGenTextures(1, &_shadowMap);
    glBindTexture(GL_TEXTURE_2D, _shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, _shadowMapWidth, _shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    constexpr float borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _shadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}