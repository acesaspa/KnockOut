#include "PowerUp.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

PowerUp::PowerUp(glm::vec3 location, int type) {

    Location.x = location.x;
    Location.y = location.y;
    Location.z = location.z;

    Type = type;

    Player = 0;
    isCollected = false;
}

void PowerUp::foo()
{
}