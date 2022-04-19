#ifndef SOLARSYSTEM_OUTERSHELL_H
#define SOLARSYSTEM_OUTERSHELL_H
#include "SpaceObject.h"
#include <memory>

class OuterShell : public SpaceObject {
public:
    explicit OuterShell(MeshHolder model, const Shader& shader, std::shared_ptr<SpaceObject> parent, float earthScaleFactor);
    virtual void AdjustToParent(bool isRunTime) = 0;
    std::shared_ptr<SpaceObject> GetParent() const;

protected:
    std::shared_ptr<SpaceObject> _parent;
    float _scaleFactor;
};

#endif //SOLARSYSTEM_OUTERSHELL_H
