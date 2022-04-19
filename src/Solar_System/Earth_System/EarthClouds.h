#ifndef SOLARSYSTEM_EARTHCLOUDS_H
#define SOLARSYSTEM_EARTHCLOUDS_H

#include "../Clouds.h"

class EarthClouds : public Clouds {
public:
    explicit EarthClouds(const CloudsInfo& cloudsInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;
};

#endif //SOLARSYSTEM_EARTHCLOUDS_H
