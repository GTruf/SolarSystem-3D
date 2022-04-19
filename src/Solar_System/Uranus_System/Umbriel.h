#ifndef SOLARSYSTEM_UMBRIEL_H
#define SOLARSYSTEM_UMBRIEL_H
#include "../Satellite.h"

class Umbriel : public Satellite {
public:
    explicit Umbriel(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_UMBRIEL_H
