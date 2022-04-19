#ifndef SOLARSYSTEM_MIMAS_H
#define SOLARSYSTEM_MIMAS_H
#include "../Satellite.h"

class Mimas : public Satellite {
public:
    explicit Mimas(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_MIMAS_H
