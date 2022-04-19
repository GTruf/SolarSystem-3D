#include "Deimos.h"

Deimos::Deimos(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent) : Satellite(satelliteInfo, std::move(parent)),
    _diffuses(satelliteInfo.diffuseTextures), _normalMap(satelliteInfo.normalMap)
{
}

void Deimos::AdjustToParent(bool isRunTime) {
    static float x = 0, z = -2.95f;

    static float circleRadius = 0.001f;
    static float time = 0.0f;
    static float velocity = 3.5f;
    static float rotationAngle = 0.0f;

    if (isRunTime) {
        time += 0.0001f;
        rotationAngle -= 4 *  0.0315f;

        x += circleRadius * glm::cos(velocity * time);
        z += circleRadius * glm::sin(velocity * time);
    }

    LoadIdentityModelMatrix();
    Translate(_parent->GetPosition() + glm::vec3(x, 0.0f, z));
    Scale(glm::vec3(_earthSizeCoefficient));
    Rotate(rotationAngle, glm::vec3(0, 1, 0));
    UpdateModelMatrix();
}

void Deimos::Render() const {
    GetShader().SetBool("hasCloudTexture", false);
    GetShader().SetBool("hasNightTexture", false);
    GetShader().SetBool("hasSpecularMap", false);
    GetShader().SetBool("hasSpecular", false);
    GetShader().SetBool("isUseSphereIntersect", true);
    GetShader().SetInt("mainDiffuseTexture", 0);
    GetShader().SetInt("normalMap", 1);
    GetShader().SetFloat("ambientFactor", 0.0f);

    glBindTextureUnit(0, _diffuses.at(0).GetTexture());
    glBindTextureUnit(1, _normalMap.GetTexture());

    SpaceObject::Render();
}
