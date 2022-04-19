#ifndef SOLARSYSTEM_PHOBOS_H
#define SOLARSYSTEM_PHOBOS_H
#include "../Satellite.h"

class Phobos : public Satellite {
public:
    explicit Phobos(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_PHOBOS_H
