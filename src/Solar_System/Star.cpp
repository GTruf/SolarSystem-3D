#include "Star.h"

Star::Star(const StarInfo& starInfo) : SpaceObject(starInfo.starModel, starInfo.starShader, starInfo.engName, starInfo.otherLangName),
    _starTemperature(starInfo.starTemperature), _starRadius(starInfo.starRadius), _starSpectrumTexture(starInfo.starSpectrum), _glowShader(starInfo.glowStarShader),
    _glowTintMult(starInfo.glowTintMult)
{
    InitBuffers();
    _starOcclusion.fill(0);
    _starTemperatureColorUCoordinate = glm::clamp((_starTemperature - 800.0f) / 29200.f, 0.0f, 1.0f);
    CalculateShiftColor();
}

void Star::RenderGlow(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& vs, float aspect, float distance,
                      const std::optional<RingCameraInfo>& ringCameraInfo, float starTemperature)
{
    if (starTemperature != _starTemperature) {
        _starTemperature = starTemperature;
        _starTemperatureColorUCoordinate = glm::clamp((_starTemperature - 800.0f) / 29200.f, 0.0f, 1.0f);
        CalculateShiftColor();
    }

    CalculateGlowSize(distance);
    _currentGlowSize *= _visibility * 0.7f;
    _currentGlowSize = glm::max(0.0001f, _currentGlowSize);
    const glm::vec2 glowDimensions(_currentGlowSize, _currentGlowSize * aspect);

    _glowShader.Use();
    _glowShader.SetMat4("projection", projection);
    _glowShader.SetMat4("view", view);
    _glowShader.SetVec3("center", GetPosition());
    _glowShader.SetVec3("colorMult", _glowTintMult);
    _glowShader.SetVec2("dims", glowDimensions * 0.5f);
    _glowShader.SetFloat("uColorMap", _starTemperatureColorUCoordinate);
    _glowShader.SetFloat("noiseZ", (vs.x + vs.y - vs.z) * 0.25f);
    _glowShader.SetInt("colorMap", 0);
    _glowShader.SetInt("ringDiffuse", 1);
    glBindTextureUnit(0, _starSpectrumTexture.GetTexture());

    _glowShader.SetBool("isPlanetaryRingInView", ringCameraInfo.has_value());
    if (ringCameraInfo) {
        _glowShader.SetVec3("cameraPosition", ringCameraInfo->cameraPosition);
        _glowShader.SetVec3("ringCenter", ringCameraInfo->ringCenter);
        _glowShader.SetVec3("ringNormal", ringCameraInfo->ringNormal);
        _glowShader.SetVec2("ringInnerOuterRadiuses", ringCameraInfo->ringInnerOuterRadiuses);
        glBindTextureUnit(1, ringCameraInfo->ringDiffuse);
    }

    glBindVertexArray(_glowVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);
}

void Star::SetVisibility(float visibility) {
    _visibility = visibility;
}

float Star::GetStarTemperatureInKelvin() const {
    return _starTemperature;
}

float Star::GetStarRadius() const {
    return _starRadius;
}

float Star::GetTemperatureColorUCoordinate() const {
    return _starTemperatureColorUCoordinate;
}

float Star::GetVisibility() const {
    return _visibility;
}

GLuint Star::GetStarSpectrumTexture() const {
    return _starSpectrumTexture.GetTexture();
}

float Star::GetCurrentGlowSize() const {
    return _currentGlowSize;
}

glm::vec3 Star::GetShiftColor() const {
    return _starShiftColor;
}

glm::vec3 Star::GetGlowTintMult() const {
    return _glowTintMult;
}

GLuint& Star::GetStarOcclusionValue(size_t index) {
    return _starOcclusion.at(index);
}

std::array<GLuint, 2>& Star::GetStarOcclusion() {
    return _starOcclusion;
}

void Star::CalculateShiftColor() {
    _starShiftColor = glm::vec3(_starTemperature * (0.0534 / 255.0) - (43.0 / 255.0),
                                _starTemperature * (0.0628 / 255.0) - (77.0 / 255.0),
                                _starTemperature * (0.0735 / 255.0) - (115.0 / 255.0));
}

void Star::CalculateGlowSize(float distance) {
    // We approximate the luminosity of the star by comparing our temperature and diameter to that of the sun,
    // since we know the luminosity, diameter, and temperature of the sun.

    static constexpr float DSUN = 1392684.0;
    static constexpr float TSUN = 5778.0;

    float d = distance; // Distance
    float D = _starRadius * 2 * DSUN;
    float L = (D * D) * std::pow(_starTemperature / TSUN, 4.0); // Luminosity
    _currentGlowSize = 0.016 * std::pow(L, 0.25) / std::pow(d, 0.5) * 0.001667; // Size
}

void Star::InitBuffers() {
    constexpr glm::vec2 cPositions[4] = {
            glm::vec2(-1.0f, 1.0f),
            glm::vec2(-1.0f, -1.0f),
            glm::vec2(1.0f, -1.0f),
            glm::vec2(1.0f, 1.0f)
    };

    constexpr uint16_t cIndices[6] = {0, 1, 2, 2, 3, 0};

    glGenVertexArrays(1, &_glowVao);
    glGenBuffers(1, &_glowVbo);
    glGenBuffers(1, &_glowEbo);

    glBindVertexArray(_glowVao);
    glBindBuffer(GL_ARRAY_BUFFER, _glowVbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(cPositions), cPositions, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _glowEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cIndices), cIndices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
}
