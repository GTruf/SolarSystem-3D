#include "UranusClouds.h"

UranusClouds::UranusClouds(const CloudsInfo& cloudsInfo, std::shared_ptr<SpaceObject> parent) : Clouds(cloudsInfo, std::move(parent))
{
}

void UranusClouds::AdjustToParent(bool isRunTime) {
    static float rotationAngle = 0;

    if (isRunTime) {
        rotationAngle += 6 * 0.009575;
    }

    LoadIdentityModelMatrix();
    Translate(_parent->GetPosition());
    Scale(glm::vec3(_scaleFactor));
    //Rotate(97.8f, glm::vec3(1, 0, 0));
    Rotate(81.2f, glm::vec3(1, 0, 0));
    Rotate(rotationAngle, glm::vec3(0, 1, 0));
    //Rotate(-23.4f, glm::vec3(0, 0, 1));
    UpdateModelMatrix();
}

void UranusClouds::Render() const {
    GetShader().SetFloat("ambientFactor", 0.0);
    GetShader().SetInt("mainDiffuseTexture", 0);
    GetShader().SetInt("cloudsNormalMap", 1);

    glBindTextureUnit(0, _diffuse.GetTexture());
    glBindTextureUnit(1, _normal.GetTexture());

    SpaceObject::Render();
}
