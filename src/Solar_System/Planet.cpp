#include "Planet.h"

Planet::Planet(const PlanetInfo& planetInfo, std::shared_ptr<Star> parentStar) :
    SpaceObject(planetInfo.planetModel, planetInfo.planetShader, planetInfo.engName, planetInfo.otherLangName), _parentStar(std::move(parentStar)),
    _earthSizeCoefficient(planetInfo.earthSizeCoefficient)
{
    _radius *= _earthSizeCoefficient;
}

float Planet::GetRadius() const {
    return _radius;
}

float Planet::GetEarthSizeCoefficient() const {
    return _earthSizeCoefficient;
}

