#ifndef SOLARSYSTEM_ARIEL_H
#define SOLARSYSTEM_ARIEL_H
#include "../Satellite.h"

class Ariel : public Satellite {
public:
    explicit Ariel(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_ARIEL_H
