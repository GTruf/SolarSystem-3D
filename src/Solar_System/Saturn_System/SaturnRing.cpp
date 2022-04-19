#include "SaturnRing.h"

SaturnRing::SaturnRing(const PlanetaryRingInfo& planetaryRingInfo, std::shared_ptr<Planet> parent) : PlanetaryRing(planetaryRingInfo, std::move(parent)) {
}

void SaturnRing::AdjustToParent() {
    LoadIdentityModelMatrix();
    Translate(_parentPlanet->GetPosition());
    //Rotate(13.35f, glm::vec3(0, 0, 1));
    Rotate(-26.7f, glm::vec3(0, 0, 1));
    Rotate(-15.f, glm::vec3(1, 0, 0));
    UpdateRingNormal();
    UpdateModelMatrix();
}

void SaturnRing::Render() const {
    GetShader().SetVec3("planetPos", _parentPlanet->GetPosition());
    GetShader().SetFloat("planetRadius", _parentPlanet->GetRadius());
    GetShader().SetInt("ringDiffuse", 0);
    glBindTextureUnit(0, _ringTexture.GetTexture());
    SpaceObject::Render();
}
