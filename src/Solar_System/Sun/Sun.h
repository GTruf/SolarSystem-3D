#ifndef SOLARSYSTEM_SUN_H
#define SOLARSYSTEM_SUN_H
#include "../Star.h"

class Sun : public Star {
public:
    explicit Sun(const StarInfo& starInfo);
    void TakeStarSystemCenter() override;
};


#endif //SOLARSYSTEM_SUN_H
