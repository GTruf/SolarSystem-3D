#ifndef SOLARSYSTEM_SATELLITE_H
#define SOLARSYSTEM_SATELLITE_H
#include "SpaceObject.h"
#include "Star.h"
#include "../Auxiliary_Modules/TextureImage2D.h"
#include <vector>
#include <memory>

struct SatelliteInfo {
    MeshHolder satelliteModel;
    float earthSizeCoefficient;
    Shader satelliteShader;
    std::vector<TextureImage2D> diffuseTextures;
    TextureImage2D normalMap;
    TextureImage2D specularTexture;
    std::wstring engName;
    std::wstring otherLangName;

    explicit SatelliteInfo(MeshHolder model, float earthSizeCoefficient, const Shader& shader, std::vector<TextureImage2D> diffuses, const TextureImage2D& normalMap,
                       std::wstring engName = L"", std::wstring otherLangName = L"", const TextureImage2D& specular = TextureImage2D()) :
                       satelliteModel(std::move(model)), earthSizeCoefficient(earthSizeCoefficient), satelliteShader(shader),
                       diffuseTextures(std::move(diffuses)), normalMap(normalMap), specularTexture(specular), engName(std::move(engName)),
                       otherLangName(std::move(otherLangName)) {}
};

class Satellite : public SpaceObject {
public:
    explicit Satellite(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent);
    std::shared_ptr<SpaceObject> GetParent() const;
    float GetRadius() const;
    float GetEarthSizeCoefficient() const;
    virtual void AdjustToParent(bool isRunTime) = 0;

protected:
    std::shared_ptr<SpaceObject> _parent;
    float _radius = 2.0; // Radius of the earth 3d model in Blender
    float _earthSizeCoefficient;
};

#endif //SOLARSYSTEM_SATELLITE_H
