#include "SpaceObject.h"

SpaceObject::SpaceObject(MeshHolder model, const Shader& shader, std::wstring engName, std::wstring otherLangName) : Transformable(shader),
    _objectModel(std::move(model)), _engName(std::move(engName)), _otherLangName(std::move(otherLangName))
{
}

void SpaceObject::Render() const {
    _objectModel.Draw(_shader);
}

MeshHolder SpaceObject::GetModel() const {
    return _objectModel;
}

const std::wstring& SpaceObject::GetEngName() const {
    return _engName;
}

const std::wstring& SpaceObject::GetOtherLangName() const {
    return _otherLangName;
}
