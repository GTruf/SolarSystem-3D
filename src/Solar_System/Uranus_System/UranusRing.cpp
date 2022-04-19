#include "UranusRing.h"

UranusRing::UranusRing(const PlanetaryRingInfo& planetaryRingInfo, std::shared_ptr<Planet> parent) : PlanetaryRing(planetaryRingInfo, std::move(parent))
{
}

void UranusRing::AdjustToParent() {
    LoadIdentityModelMatrix();
    Translate(_parentPlanet->GetPosition());
    //Rotate(97.8f, glm::vec3(1, 0, 0));
    Rotate(81.2f, glm::vec3(1, 0, 0));
    //Rotate(50.2f, glm::vec3(1, 0, 0));
    UpdateRingNormal();
    UpdateModelMatrix();
}

void UranusRing::Render() const {
    GetShader().SetVec3("planetPos", _parentPlanet->GetPosition());
    GetShader().SetFloat("planetRadius", _parentPlanet->GetRadius());
    GetShader().SetInt("ringDiffuse", 0);
    glBindTextureUnit(0, _ringTexture.GetTexture());
    SpaceObject::Render();
}
