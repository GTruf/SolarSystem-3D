#ifndef SOLARSYSTEM_EUROPA_H
#define SOLARSYSTEM_EUROPA_H
#include "../Satellite.h"

class Europa : public Satellite {
public:
    explicit Europa(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_EUROPA_H
