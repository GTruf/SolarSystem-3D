#ifndef SOLARSYSTEM_TITANIA_H
#define SOLARSYSTEM_TITANIA_H
#include "../Satellite.h"

class Titania : public Satellite {
public:
    explicit Titania(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_TITANIA_H
