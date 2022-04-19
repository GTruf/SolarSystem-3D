#ifndef SOLARSYSTEM_ENCELADUS_H
#define SOLARSYSTEM_ENCELADUS_H
#include "../Satellite.h"

class Enceladus : public Satellite {
public:
    explicit Enceladus(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_ENCELADUS_H
