#ifndef UTILS_H
#define UTILS_H

#include <glm/gtc/type_ptr.hpp>
#include <../include/physx/PxPhysicsAPI.h>

static class Utils {
public:
	static int cubeArrayLen;
	static int planeArrayLen;
	static float cubeVertexData[];
	static float planeVertexData[];
	static glm::mat4 getGlmMatFromPxMat(physx::PxMat44 pxMat);
};

#endif
