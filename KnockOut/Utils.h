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
	static glm::vec3 getRigidStaticPos(physx::PxRigidStatic* pxRigid);
	static glm::vec3 getRigidDynamicPos(physx::PxRigidDynamic* pxRigid);
};

#endif
