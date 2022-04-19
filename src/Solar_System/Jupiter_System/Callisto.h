#ifndef SOLARSYSTEM_CALLISTO_H
#define SOLARSYSTEM_CALLISTO_H
#include "../Satellite.h"

class Callisto : public Satellite {
public:
    explicit Callisto(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_CALLISTO_H
