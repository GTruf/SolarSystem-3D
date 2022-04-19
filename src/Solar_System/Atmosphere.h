#ifndef SOLARSYSTEM_ATMOSPHERE_H
#define SOLARSYSTEM_ATMOSPHERE_H
#include "OuterShell.h"

struct AtmosphereInfo {
    MeshHolder atmosphereModel;
    Shader atmosphereShader;
    glm::vec3 atmosphereColor, mieTint;
    float scaleFactor, innerRadius, outerRadius;

    explicit AtmosphereInfo(MeshHolder model, const Shader& shader, float earthScaleFactor, const glm::vec3& atmosphereColor,
                            float innerRadius, float outerRadius, const glm::vec3& mieTint = glm::vec3(1.0)) : atmosphereModel(std::move(model)),
                            atmosphereShader(shader), scaleFactor(earthScaleFactor), atmosphereColor(atmosphereColor), mieTint(mieTint), innerRadius(innerRadius),
                            outerRadius(outerRadius) {}
};

class Atmosphere : public OuterShell {
public:
    explicit Atmosphere(const AtmosphereInfo& atmosphereInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime = true) override;
    glm::vec3 GetMieTint() const;
    float GetInnerRadius() const;
    float GetOuterRadius() const;
    float GetAtmosphereOuterBoundary() const;

private:
    glm::vec3 _atmosphereColor, _mieTint;
    float _innerRadius, _outerRadius, _atmosphereOuterBoundary = 2.0; // 2 is a radius of the earth 3d model in Blender (physical sphere boundary)
};

#endif //SOLARSYSTEM_ATMOSPHERE_H
