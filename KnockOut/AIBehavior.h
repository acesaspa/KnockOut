#ifndef AIBEHAVIOR_H
#define AIBEHAVIOR_H

#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include "../include/physx/PxPhysicsAPI.h"
#include "../include/physx/snippetcommon/SnippetPrint.h"
#include "../include/physx/snippetcommon/SnippetPVD.h"
#include "../include/physx/snippetutils/SnippetUtils.h"
#include "../include/physx/vehicle/PxVehicleUtil.h"
#include "../include/physx/snippetvehiclecommon/SnippetVehicleSceneQuery.h"
#include "../include/physx/snippetvehiclecommon/SnippetVehicleFilterShader.h"
#include "../include/physx/snippetvehiclecommon/SnippetVehicleTireFriction.h"
#include "../include/physx/snippetvehiclecommon/SnippetVehicleCreate.h"
#include "../include/physx/snippetcommon/SnippetPrint.h"
#include "../include/physx/snippetcommon/SnippetPVD.h"
#include "../include/physx/snippetutils/SnippetUtils.h"

class AIBehavior {
public:
	void frameUpdate(physx::PxVehicleDrive4WRawInputData* carInputData, glm::vec3 carPos, glm::vec3 carForwardVec);

private:
	bool shouldTurn(glm::vec3 pos, glm::vec3 forVec);
	float calculateSlope(float y2, float y1, float x2, float x1);
	float calculateDistance(float y2, float y1, float x2, float x1);
	float calculateLinearComp(float x, float y, float m);
	float calculateIntersectionY(float m1, float x, float b1);
	float calculateIntersectionX(float b2, float b1, float m1, float m2);
	bool noIntersection(float x, float y, float m1, float b1, float m2, float b2);
};

#endif
