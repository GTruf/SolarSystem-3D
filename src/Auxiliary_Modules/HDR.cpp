#include "HDR.h"

HDR::HDR(const Shader& shader, uint16_t width, uint16_t height) : _hdrShader(shader) {
    InitQuadBuffers();
    InitFBO(width, height);
}

void HDR::Render(float exposure, float gamma) const {
    _hdrShader.Use();
    _hdrShader.SetInt("hdrBuffer", 0);
    glBindTextureUnit(0, _colorBuffer);
    _hdrShader.SetBool("hdr", true);
    _hdrShader.SetFloat("exposure", exposure);
    _hdrShader.SetFloat("gamma", gamma);

    glBindVertexArray(_quadVao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

GLuint HDR::GetHdrFBO() const {
    return _hdrFrameBuffer;
}

void HDR::InitQuadBuffers() {
    constexpr float quadVertices[] = {
             // positions           // texture Coords
            -1.0f,  1.0f, 0.0f,     0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f,     0.0f, 0.0f,
             1.0f,  1.0f, 0.0f,     1.0f, 1.0f,
             1.0f, -1.0f, 0.0f,     1.0f, 0.0f
    };

    glGenVertexArrays(1, &_quadVao);
    glGenBuffers(1, &_quadVbo);

    glBindVertexArray(_quadVao);
    glBindBuffer(GL_ARRAY_BUFFER, _quadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

void HDR::InitFBO(uint16_t width, uint16_t height) {
    glGenFramebuffers(1, &_hdrFrameBuffer);

    glGenTextures(1, &_colorBuffer);
    glBindTexture(GL_TEXTURE_2D, _colorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenRenderbuffers(1, &_rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, _rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, _hdrFrameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorBuffer, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _rboDepth);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
