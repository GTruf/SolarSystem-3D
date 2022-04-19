#ifndef SOLARSYSTEM_TRITON_H
#define SOLARSYSTEM_TRITON_H
#include "../Satellite.h"

class Triton : public Satellite {
public:
    explicit Triton(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_TRITON_H
