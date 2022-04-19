#include "OuterShell.h"

OuterShell::OuterShell(MeshHolder model, const Shader& shader, std::shared_ptr<SpaceObject> parent, float earthScaleFactor) :
    SpaceObject(std::move(model), shader), _parent(std::move(parent)), _scaleFactor(earthScaleFactor)
{
}

std::shared_ptr<SpaceObject> OuterShell::GetParent() const {
    return _parent;
}
