#ifndef SOLARSYSTEM_GANYMEDE_H
#define SOLARSYSTEM_GANYMEDE_H
#include "../Satellite.h"

class Ganymede : public Satellite {
public:
    explicit Ganymede(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_GANYMEDE_H
