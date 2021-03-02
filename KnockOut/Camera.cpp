#include "Camera.h"

float yaw = 90.0f;
float pitch = -15.0f;
float lastX = 400, lastY = 300;
float cameraDistance = 8.0f;
float angleAroundTarget = 0.0f;
bool firstMouse = true;
bool mouseVisible = false;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 15.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::vec3();
glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 direction;






void Camera::setMouseVisible(bool mouseVis) {
	mouseVisible = mouseVis;
}

bool Camera::getMouseVisible() {
	return mouseVisible;
}

glm::vec3 Camera::getCameraPos() {
	return cameraPos;
}

void Camera::setCameraPos(glm::vec3 newPos) {
	cameraPos = newPos;
}

glm::mat4 Camera::getViewMatrix() {
	glm::mat4 view = glm::mat4(1.f);
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	return view;
}

void Camera::updateCamera(float angleAroundY, glm::vec3 cubePos) {

	cameraPos.y = cubePos.y + 2.f; //calculatePos
	float xOffset = cameraDistance * cos(glm::radians(angleAroundY + angleAroundTarget));
	float zOffset = cameraDistance * sin(glm::radians(angleAroundY + angleAroundTarget));
	cameraPos.x = cubePos.x - xOffset;
	cameraPos.z = cubePos.z - zOffset;

	yaw = angleAroundY + angleAroundTarget;

	cameraFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch)); //update
	cameraFront.y = sin(glm::radians(pitch));
	cameraFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(cameraFront);
	cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
	cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
}

void Camera::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;
	//disabling camera pitch movement triggered by mouse

	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}