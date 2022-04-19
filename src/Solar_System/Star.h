#ifndef SOLARSYSTEM_STAR_H
#define SOLARSYSTEM_STAR_H
#include "SpaceObject.h"
#include "../Auxiliary_Modules/TextureImage2D.h"
#include <vector>
#include <optional>

struct StarInfo {
    MeshHolder starModel;
    Shader starShader;
    Shader glowStarShader;
    TextureImage2D starSpectrum;
    float starTemperature;
    float starRadius;
    std::wstring engName;
    std::wstring otherLangName;
    glm::vec3 glowTintMult;

    explicit StarInfo(MeshHolder model, const Shader& shader, const Shader& glowShader, const TextureImage2D& spectrum, float temperature,
                      float radius, const glm::vec3& glowTintMult, std::wstring engName = L"", std::wstring otherLangName = L"") :
                      starModel(std::move(model)), starShader(shader), glowStarShader(glowShader), starSpectrum(spectrum),
                      starTemperature(temperature), starRadius(radius), engName(std::move(engName)), otherLangName(std::move(otherLangName)),
                      glowTintMult(glowTintMult) {}
};

struct RingCameraInfo {
    glm::vec3 cameraPosition;
    glm::vec3 ringCenter; // Center of disk in eye space
    glm::vec3 ringNormal; // Disk plane normal in eye space
    glm::vec2 ringInnerOuterRadiuses; // x = Inner, y = Outer
    GLuint ringDiffuse; // Ring diffuse map
};

class Star : public SpaceObject {
public:
    explicit Star(const StarInfo& starInfo);
    virtual void TakeStarSystemCenter() = 0;
    void RenderGlow(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& vs, float aspect, float distance,
                    const std::optional<RingCameraInfo>& ringCameraInfo, float starTemperature = 5778.0f);
    void SetVisibility(float visibility);
    float GetStarTemperatureInKelvin() const;
    float GetStarRadius() const;
    float GetTemperatureColorUCoordinate() const;
    float GetVisibility() const;
    float GetCurrentGlowSize() const;
    GLuint GetStarSpectrumTexture() const;
    glm::vec3 GetShiftColor() const;
    glm::vec3 GetGlowTintMult() const;
    GLuint& GetStarOcclusionValue(size_t index);
    std::array<GLuint, 2>& GetStarOcclusion();

private:
    float _starTemperature;
    float _starRadius;
    float _starTemperatureColorUCoordinate = 0.0; // u ('x') coordinate for star spectrum color texture for shader uv-coordinate [u ('x') координата для текстуры с цветами спектрумов звёзд для uv-координат в шейдере]
    float _visibility = 1.0;
    float _currentGlowSize = 0.0;
    glm::vec3 _starShiftColor = glm::vec3(), _glowTintMult;
    TextureImage2D _starSpectrumTexture;
    std::array<GLuint, 2> _starOcclusion;
    GLuint _glowVao = 0, _glowVbo = 0, _glowEbo = 0;
    GLuint _coronaVao = 0, _coronaVbo = 0, _coronaEbo = 0;
    Shader _glowShader;

    void InitBuffers();
    void CalculateShiftColor();
    void CalculateGlowSize(float distance);
};

#endif //SOLARSYSTEM_STAR_H
