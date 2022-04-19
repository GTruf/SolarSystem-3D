#ifndef SOLARSYSTEM_URANUS_H
#define SOLARSYSTEM_URANUS_H
#include "../Planet.h"

class Uranus : public Planet {
public:
    explicit Uranus(const PlanetInfo& planetInfo, std::shared_ptr<Star> parentStar);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_URANUS_H
