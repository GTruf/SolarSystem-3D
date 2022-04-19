#include "Sun.h"

Sun::Sun(const StarInfo& starInfo) : Star(starInfo) {
}

void Sun::TakeStarSystemCenter() {
    LoadIdentityModelMatrix();
    Scale(glm::vec3(0.5f));
    UpdateModelMatrix();
}