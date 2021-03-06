#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class Camera {
public:
	void mouseCallback(GLFWwindow* window, double xpos, double ypos);
	void updateCamera(float angleAroundY, glm::vec3 cubePos);
	glm::mat4 getViewMatrix();
	glm::vec3 getCameraPos();
	void setCameraPos(glm::vec3 newPos);
	bool getMouseVisible();
	void setMouseVisible(bool mouseVis);
};


#endif
