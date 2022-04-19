#ifndef SATURN_URANUSCLOUDS_H
#define SATURN_URANUSCLOUDS_H
#include "../Clouds.h"

class UranusClouds : public Clouds {
public:
    explicit UranusClouds(const CloudsInfo& cloudsInfo, std::shared_ptr<SpaceObject> parent);
    void AdjustToParent(bool isRunTime) override;
    void Render() const override;
};

#endif //SATURN_URANUSCLOUDS_H
