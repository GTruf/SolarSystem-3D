#ifndef SOLARSYSTEM_PLANETARYRING_H
#define SOLARSYSTEM_PLANETARYRING_H
#include "Planet.h"

struct PlanetaryRingInfo {
    MeshHolder ringModel;
    float ringInnerRadius, ringOuterRadius;
    Shader ringShader;
    TextureImage2D ringDiffuse;

    explicit PlanetaryRingInfo(MeshHolder model, float innerRadius, float outerRadius, const Shader& shader, const TextureImage2D& diffuse) :
                               ringModel(std::move(model)), ringInnerRadius(innerRadius), ringOuterRadius(outerRadius), ringShader(shader),
                               ringDiffuse(diffuse) {}
};

class PlanetaryRing : public SpaceObject {
public:
    explicit PlanetaryRing(const PlanetaryRingInfo& planetaryRingInfo, std::shared_ptr<Planet> parent);
    virtual void AdjustToParent() = 0;
    std::shared_ptr<Planet> GetParent() const;
    float GetInnerRadius() const;
    float GetOuterRadius() const;
    glm::vec3 GetRingNormal() const;
    GLuint GetRingTexture() const;

protected:
    TextureImage2D _ringTexture;
    std::shared_ptr<Planet> _parentPlanet;
    float _ringInnerRadius, _ringOuterRadius;
    glm::vec3 _upVector = glm::vec3(0, 1, 0), _ringNormal;

    void UpdateRingNormal();
};

#endif //SOLARSYSTEM_PLANETARYRING_H
