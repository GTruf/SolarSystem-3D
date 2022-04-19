#ifndef SOLARSYSTEM_NEPTUNECLOUDS_H
#define SOLARSYSTEM_NEPTUNECLOUDS_H

#include "../Clouds.h"

class NeptuneClouds : public Clouds {
public:
    explicit NeptuneClouds(const CloudsInfo& cloudsInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;
};

#endif //SOLARSYSTEM_NEPTUNECLOUDS_H

