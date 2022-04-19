#ifndef SOLARSYSTEM_CHARON_H
#define SOLARSYSTEM_CHARON_H
#include "../Satellite.h"

class Charon : public Satellite {
public:
    explicit Charon(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap, _specularMap;
};

#endif //SOLARSYSTEM_CHARON_H
