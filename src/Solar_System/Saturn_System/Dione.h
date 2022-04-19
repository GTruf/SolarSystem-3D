#ifndef SOLARSYSTEM_DIONE_H
#define SOLARSYSTEM_DIONE_H
#include "../Satellite.h"

class Dione : public Satellite {
public:
    explicit Dione(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_DIONE_H
