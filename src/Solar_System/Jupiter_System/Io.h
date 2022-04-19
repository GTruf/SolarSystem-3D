#ifndef SOLARSYSTEM_IO_H
#define SOLARSYSTEM_IO_H
#include "../Satellite.h"

class Io : public Satellite {
public:
    explicit Io(const SatelliteInfo& satelliteInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_IO_H
