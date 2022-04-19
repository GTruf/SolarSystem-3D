#ifndef SOLARSYSTEM_TITAN_H
#define SOLARSYSTEM_TITAN_H
#include "../Satellite.h"

class Titan : public Satellite {
public:
    explicit Titan(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_TITAN_H
