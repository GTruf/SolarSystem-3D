#ifndef SOLARSYSTEM_MIRANDA_H
#define SOLARSYSTEM_MIRANDA_H
#include "../Satellite.h"

class Miranda : public Satellite {
public:
    explicit Miranda(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_MIRANDA_H
