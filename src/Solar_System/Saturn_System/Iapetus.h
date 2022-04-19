#ifndef SOLARSYSTEM_IAPETUS_H
#define SOLARSYSTEM_IAPETUS_H
#include "../Satellite.h"

class Iapetus : public Satellite {
public:
    explicit Iapetus(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_IAPETUS_H
