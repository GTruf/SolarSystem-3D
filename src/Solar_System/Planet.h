#ifndef SOLARSYSTEM_PLANET_H
#define SOLARSYSTEM_PLANET_H
#include "SpaceObject.h"
#include "Star.h"
#include "../Auxiliary_Modules/TextureImage2D.h"
#include <vector>
#include <memory>

struct PlanetInfo {
    MeshHolder planetModel;
    float earthSizeCoefficient;
    Shader planetShader;
    std::vector<TextureImage2D> diffuseTextures;
    TextureImage2D normalMap;
    TextureImage2D specularTexture;
    std::wstring engName;
    std::wstring otherLangName;

    explicit PlanetInfo(MeshHolder model, float earthSizeCoefficient, const Shader& shader, std::vector<TextureImage2D> diffuses, const TextureImage2D& normalMap,
                        std::wstring engName = L"", std::wstring otherLangName = L"", const TextureImage2D& specular = TextureImage2D()) :
                        planetModel(std::move(model)), earthSizeCoefficient(earthSizeCoefficient), planetShader(shader), diffuseTextures(std::move(diffuses)),
                        normalMap(normalMap), specularTexture(specular), engName(std::move(engName)), otherLangName(std::move(otherLangName)) {}
};

class Planet : public SpaceObject {
public:
    explicit Planet(const PlanetInfo& planetInfo, std::shared_ptr<Star> parentStar);
    virtual void AdjustToParent(bool isRunTime) = 0;
    float GetRadius() const;
    float GetEarthSizeCoefficient() const;

protected:
    std::shared_ptr<Star> _parentStar;
    float _radius = 2.0; // Radius of the earth 3d model in Blender
    float _earthSizeCoefficient;
};

#endif //SOLARSYSTEM_PLANET_H
