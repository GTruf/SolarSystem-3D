#ifndef SOLARSYSTEM_RHEA_H
#define SOLARSYSTEM_RHEA_H
#include "../Satellite.h"

class Rhea : public Satellite {
public:
    explicit Rhea(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_RHEA_H
