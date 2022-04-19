#ifndef SOLARSYSTEM_URANUSRING_H
#define SOLARSYSTEM_URANUSRING_H
#include "../PlanetaryRing.h"

class UranusRing : public PlanetaryRing {
public:
    explicit UranusRing(const PlanetaryRingInfo& planetaryRingInfo, std::shared_ptr<Planet> parent);
    void AdjustToParent() override;
    void Render() const override;
};

#endif //SOLARSYSTEM_URANUSRING_H
