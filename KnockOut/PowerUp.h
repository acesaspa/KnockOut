#ifndef PowerUp_H
#define PowerUp_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class PowerUp
{
public:
	PowerUp(glm::vec3 location, int type);
	void foo();
	int Type;
	glm::vec3 Location;
	int Player;
	bool isCollected;
};

#endif 