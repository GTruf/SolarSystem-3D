#include "Saturn.h"

Saturn::Saturn(const PlanetInfo& planetInfo, std::shared_ptr<Star> parentStar) : Planet(planetInfo, std::move(parentStar)), _diffuses(planetInfo.diffuseTextures),
_normalMap(planetInfo.normalMap)
{
    Translate(_parentStar->GetPosition() + glm::vec3(0.0f, -100.f, 2450.0f)); // Init position for light space matrix
}

void Saturn::AdjustToParent(bool isRunTime) {
    static float rotationAngle = 0;

    if (isRunTime) {
        rotationAngle += 4 * 0.0125;
    }

    LoadIdentityModelMatrix();
    Translate(_parentStar->GetPosition() + glm::vec3(0.0f, -100.f, 2450.0f));
    Scale(glm::vec3(_earthSizeCoefficient));
    Rotate(-26.7f, glm::vec3(0, 0, 1));
    Rotate(-15.f, glm::vec3(1, 0, 0));
    Rotate(rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    UpdateModelMatrix();
}

void Saturn::Render() const {
    GetShader().SetBool("hasNightTexture", false);
    GetShader().SetBool("hasSpecularMap", false);
    GetShader().SetBool("hasSpecular", false);
    GetShader().SetBool("isUseSphereIntersect", false);
    GetShader().SetInt("mainDiffuseTexture", 0);
    GetShader().SetInt("normalMap", 1);
    GetShader().SetFloat("ambientFactor", 0.0f);

    glBindTextureUnit(0, _diffuses.at(0).GetTexture());
    glBindTextureUnit(1, _normalMap.GetTexture());

    SpaceObject::Render();
}
