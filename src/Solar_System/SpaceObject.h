#ifndef SOLARSYSTEM_SPACEOBJECT_H
#define SOLARSYSTEM_SPACEOBJECT_H
#include "../Auxiliary_Modules/MeshHolder.h"
#include "Transformable.h"

class SpaceObject : public Transformable {
public:
    explicit SpaceObject(MeshHolder model, const Shader& shader, std::wstring engName = L"", std::wstring otherLangName = L"");
    virtual void Render() const;
    MeshHolder GetModel() const;
    const std::wstring& GetEngName() const;
    const std::wstring& GetOtherLangName() const;

protected:
    MeshHolder _objectModel;
    std::wstring _engName, _otherLangName;
    glm::mat4 _lightSpaceMatrix = glm::mat4(1.0);
};

#endif //SOLARSYSTEM_SPACEOBJECT_H
