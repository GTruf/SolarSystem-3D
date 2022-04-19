#ifndef SOLARSYSTEM_EARTH_H
#define SOLARSYSTEM_EARTH_H
#include "../Planet.h"

class Earth : public Planet {
public:
    explicit Earth(const PlanetInfo& planetInfo, std::shared_ptr<Star> parentStar);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap, _specular;
};

#endif //SOLARSYSTEM_EARTH_H
