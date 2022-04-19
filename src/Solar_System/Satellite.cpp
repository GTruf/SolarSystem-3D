#include "Satellite.h"

Satellite::Satellite(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent) :
    SpaceObject(satelliteInfo.satelliteModel, satelliteInfo.satelliteShader, satelliteInfo.engName, satelliteInfo.otherLangName), _parent(std::move(parent)),
    _earthSizeCoefficient(satelliteInfo.earthSizeCoefficient)
{
    _radius *= _earthSizeCoefficient;
}

std::shared_ptr<SpaceObject> Satellite::GetParent() const {
    return _parent;
}

float Satellite::GetRadius() const {
    return _radius;
}

float Satellite::GetEarthSizeCoefficient() const {
    return _earthSizeCoefficient;
}
