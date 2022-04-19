#ifndef SOLARSYSTEM_SATURN_H
#define SOLARSYSTEM_SATURN_H
#include "../Planet.h"

class Saturn : public Planet {
public:
    explicit Saturn(const PlanetInfo& planetInfo, std::shared_ptr<Star> parentStar);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_SATURN_H
