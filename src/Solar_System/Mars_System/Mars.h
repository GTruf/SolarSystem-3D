#ifndef SOLARSYSTEM_MARS_H
#define SOLARSYSTEM_MARS_H
#include "../Planet.h"

class Mars : public Planet {
public:
    explicit Mars(const PlanetInfo& planetInfo, std::shared_ptr<Star> parentStar);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_MARS_H
