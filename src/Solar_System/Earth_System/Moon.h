#ifndef SOLARSYSTEM_MOON_H
#define SOLARSYSTEM_MOON_H
#include "../Satellite.h"

class Moon : public Satellite {
public:
    explicit Moon(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_MOON_H
