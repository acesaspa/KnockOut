//#include "Camera.h"
//
//void Camera::processInput(GLFWwindow* window) {
//	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
//		glfwSetWindowShouldClose(window, true);
//	}
//
//	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
//		if (mouseVisible) {
//			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
//			mouseVisible = false;
//		}
//		else {
//			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
//			mouseVisible = true;
//		}
//	}
//
//	releaseAllControls();
//
//	float cameraSpeed = 10 * deltaTime;
//	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += cameraSpeed * cameraFront;
//	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= cameraSpeed * cameraFront;
//	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
//	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
//
//	if ((glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_UP) == GLFW_REPEAT)) {
//		gMimicKeyInputs = true;
//		gVehicle4W->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
//		startAccelerateForwardsMode();
//	}
//	if ((glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_REPEAT)) {
//		gMimicKeyInputs = true;
//		gVehicle4W->mDriveDynData.forceGearChange(PxVehicleGearsData::eREVERSE);
//		startAccelerateReverseMode();
//	}
//	if ((glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_REPEAT)) {
//		gMimicKeyInputs = true;
//		startBrakeMode();
//	}
//	if ((glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_REPEAT)) {
//		gMimicKeyInputs = true;
//		startTurnHardRightMode();
//	}
//	if ((glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_REPEAT)) {
//		gMimicKeyInputs = true;
//		startTurnHardLeftMode();
//	}
//}
//
//
//
//
//void Camera::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
//	if (firstMouse) {
//		lastX = xpos;
//		lastY = ypos;
//		firstMouse = false;
//	}
//
//	float xoffset = xpos - lastX;
//	float yoffset = lastY - ypos;
//	lastX = xpos;
//	lastY = ypos;
//
//	float sensitivity = 0.1f;
//	xoffset *= sensitivity;
//	yoffset *= sensitivity;
//
//	yaw += xoffset;
//	pitch += yoffset;
//
//	if (pitch > 89.0f) pitch = 89.0f;
//	if (pitch < -89.0f) pitch = -89.0f;
//
//	glm::vec3 direction;
//	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
//	direction.y = sin(glm::radians(pitch));
//	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
//	cameraFront = glm::normalize(direction);
//}
//
//
//void Camera::releaseAllControls()
//{
//	if (gMimicKeyInputs)
//	{
//		gVehicleInputData.setDigitalAccel(false);
//		gVehicleInputData.setDigitalSteerLeft(false);
//		gVehicleInputData.setDigitalSteerRight(false);
//		gVehicleInputData.setDigitalBrake(false);
//		gVehicleInputData.setDigitalHandbrake(false);
//	}
//	else
//	{
//		gVehicleInputData.setAnalogAccel(0.0f);
//		gVehicleInputData.setAnalogSteer(0.0f);
//		gVehicleInputData.setAnalogBrake(0.0f);
//		gVehicleInputData.setAnalogHandbrake(0.0f);
//	}
//}