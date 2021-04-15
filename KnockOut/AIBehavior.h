#ifndef AIBEHAVIOR_H
#define AIBEHAVIOR_H

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
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

class Opponent {
public:
	Opponent();
	void frameUpdate(physx::PxVehicleDrive4WRawInputData* carInputData, glm::vec3 carPos, glm::vec3 carForwardVec, glm::vec3 playerPos, glm::vec3 playerForwardVector,
		physx::PxVehicleDrive4W* opponentVehicle4W, physx::PxVehicleDrive4W* playerVeh4W);
	void updateLevelBB(std::vector<glm::vec3> lvlBB, bool lastSegment);
	void startEvac();
	void setAttacking();
	void setPowerUp(int pUp);
	bool hasPowerUp();
	std::vector<glm::vec3> testLocs;

private:
	bool pointIsRight(glm::vec3 a, glm::vec3 b, glm::vec3 c);
	void adjustCourse();
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
	void useAttack();
	void setNextWayPoint();
	void pursueCurrentWayPoint();
	void evacSegment();
	void attackPlayer();
	glm::vec2 getIntersection(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);

	glm::vec3 playerPosition;
	glm::vec3 playerForwardVector;
	glm::vec3 carPosition;
	glm::vec3 carForwardVector;
	std::vector<glm::vec3> levelBB;
	std::vector<glm::vec3> carBBs;
	physx::PxVehicleDrive4WRawInputData* carInputData;
	physx::PxVehicleDrive4W* carVehicle4W;
	physx::PxVehicleDrive4W* playerVehicle4W;


	enum State { noState, roaming, attacking, defending, evac };
	enum HasPowerUp { none, attack, defense, jump };
	HasPowerUp currentPowerUp = none;
	State currentState = roaming;
	State returnState = noState;

	unsigned int counter = 0;
	float prevFrameDistance = 0.f;
	float prevDistanceFromPlayer = 0.f;
	float maxSpeed = 30.f;
	float hitPointDist = 30.f;
	float evacCenterDist = 20.f;
	float playerForVecMultiplier = 5.f;
	float yMax = 50.f;
	float edgeDistLimit = 40.f;
	float holeDistLimit = 20.f;
	float sideDistanceLimit = 10.f;
	float currentSpeed = 0.7 * maxSpeed;
	float currentTurn = 0.f;
	bool attackingPoint = false;
	int turnTime = 0;
	bool turningRight = false;
	bool oneSegmentLeft = false;
	int lockCount = 0;
	float currentSpeedMultiplier = 0.f;
	float currentAnalogSteer = 0.f;

	glm::vec2 currentWayPoint = glm::vec2(0.f, 0.f);
	std::vector<glm::vec3> holeBBs;
	glm::vec3 carRightVec;
	glm::vec3 carLeftVec;

	glm::vec3 mapUpVec = glm::vec3(0.f, 0.5f, 1.f);
	glm::vec3 mapDownVec = glm::vec3(0.f, 0.5f, -1.f);
	glm::vec3 mapRightVec = glm::vec3(-1.f, 0.5f, 0.f);
	glm::vec3 mapLeftVec = glm::vec3(1.f, 0.5f, 0.f);

};

#endif