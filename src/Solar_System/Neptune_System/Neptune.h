#ifndef SOLARSYSTEM_NEPTUNE_H
#define SOLARSYSTEM_NEPTUNE_H
#include "../Planet.h"

class Neptune : public Planet {
public:
    explicit Neptune(const PlanetInfo& planetInfo, std::shared_ptr<Star> parentStar);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;

private:
    std::vector<TextureImage2D> _diffuses;
    TextureImage2D _normalMap;
};

#endif //SOLARSYSTEM_NEPTUNE_H
