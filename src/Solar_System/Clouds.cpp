#include "Clouds.h"

Clouds::Clouds(const CloudsInfo& cloudsInfo, std::shared_ptr<SpaceObject> parent) : OuterShell(cloudsInfo.cloudsModel, cloudsInfo.cloudsShader, std::move(parent),
    cloudsInfo.scaleFactor), _diffuse(cloudsInfo.diffuseMap), _normal(cloudsInfo.normalMap)
{
}
