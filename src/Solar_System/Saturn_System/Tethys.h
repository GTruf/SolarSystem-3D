#ifndef SOLARSYSTEM_TETHYS_H
#define SOLARSYSTEM_TETHYS_H
#include "../Satellite.h"

class Tethys : public Satellite {
public:
    explicit Tethys(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_TETHYS_H
