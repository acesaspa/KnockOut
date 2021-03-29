#ifndef AIBEHAVIOR_H
#define AIBEHAVIOR_H

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/type_ptr.hpp>
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

#include "Utils.h"

class AIBehavior {
public:
	AIBehavior();
	void frameUpdate(physx::PxVehicleDrive4WRawInputData* carInputData, glm::vec3 carPos, glm::vec3 carForwardVec, glm::vec3 playerPos, glm::vec3 playerForwardVector,
		physx::PxVehicleDrive4W* opponentVehicle4W);

	std::vector<glm::vec3> levelBB;

	std::vector<glm::vec3> testLocs;

private:
	bool pointIsRight(glm::vec3 a, glm::vec3 b, glm::vec3 c);
	int shouldChangeCourse();
	void attackPoint(glm::vec3 pointToAttack);
	void behave();
	void setSpeed(float target);
	float calculateAngleBetweenLines(float m2, float m1);
	float calculateSlope(float y2, float y1, float x2, float x1);
	float calculateDistance(float y2, float y1, float x2, float x1);
	float calculateLinearComp(float x, float y, float m);
	float calculateIntersectionY(float m1, float x, float b1);
	float calculateIntersectionX(float b2, float b1, float m1, float m2);
	bool noIntersection(float x, float y, float m1, float b1, float m2, float b2);
	void initHoleBBs();

	glm::vec3 playerPosition;
	glm::vec3 playerForwardVector;
	glm::vec3 carPosition;
	glm::vec3 carForwardVector;
	physx::PxVehicleDrive4WRawInputData* carInputData;
	physx::PxVehicleDrive4W* carVehicle4W;


};

#endif