#ifndef SOLARSYSTEM_SATURNRING_H
#define SOLARSYSTEM_SATURNRING_H
#include "../PlanetaryRing.h"

class SaturnRing : public PlanetaryRing {
public:
    explicit SaturnRing(const PlanetaryRingInfo& planetaryRingInfo, std::shared_ptr<Planet> parent);
    void AdjustToParent() override;
    void Render() const override;
};

#endif //SOLARSYSTEM_SATURNRING_H
