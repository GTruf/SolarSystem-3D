#ifndef SOLARSYSTEM_OBERON_H
#define SOLARSYSTEM_OBERON_H
#include "../Satellite.h"

class Oberon : public Satellite {
public:
    explicit Oberon(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_OBERON_H
