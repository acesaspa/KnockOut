#include "AIBehavior.h"


enum State {roaming, attacking, defending};
State currentState = roaming;

unsigned int counter = 0;
float prevFrameDistance = 0.f;
float prevDistanceFromPlayer = 0.f;
float maxSpeed = 30.f;
float hitPointDist = 30.f;
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


std::vector<glm::vec3> holeBBs;
glm::vec3 carRightVec;
glm::vec3 carLeftVec;

glm::vec3 mapUpVec = glm::vec3(0.f, 0.5f, 1.f);
glm::vec3 mapDownVec = glm::vec3(0.f, 0.5f, -1.f);
glm::vec3 mapRightVec = glm::vec3(-1.f, 0.5f, 0.f);
glm::vec3 mapLeftVec = glm::vec3(1.f, 0.5f, 0.f);


AIBehavior::AIBehavior() {
	initHoleBBs();
	for (int i = 0; i < holeBBs.size(); i++) {
		testLocs.push_back(holeBBs[i]);
	}
	for (int i = 0; i < levelBB.size(); i++) {
		testLocs.push_back(levelBB[i]);
	}
}



void AIBehavior::frameUpdate(physx::PxVehicleDrive4WRawInputData* carInputDat, glm::vec3 carPos, glm::vec3 carForwardVec, glm::vec3 playerPos, glm::vec3 playerForwardVec, physx::PxVehicleDrive4W* carVeh4W) {

	playerForwardVector = playerForwardVec;
	//TODO: hit prediction instead
	float pFVlen = glm::sqrt(glm::pow(playerForwardVector.x, 2) + glm::pow(playerForwardVector.z, 2));
	playerPosition = playerPos + glm::vec3(playerForwardVector.x * (playerForVecMultiplier/pFVlen), 0.f, playerForwardVector.z * (playerForVecMultiplier / pFVlen));
	carPosition = carPos;
	carForwardVector = carForwardVec;
	carLeftVec = glm::cross(glm::vec3(carPosition.x, 5.f, carPosition.z), carForwardVector);
	carRightVec = glm::cross(carForwardVector, glm::vec3(carPosition.x, 5.f, carPosition.z));
	carInputData = carInputDat;
	carVehicle4W = carVeh4W;
	counter++;


	behave();
}


void AIBehavior::behave() {
	int courseResult = shouldChangeCourse();

	//HOLE AVOIDANCE
	if (courseResult == 1) {
		carInputData->setAnalogSteer(1.f);
		setSpeed(maxSpeed * 0.2);
	}
	else {

		//ATTACKING
		if (currentState == attacking) { //prioritize attacking state over every other state
			setSpeed(maxSpeed);

			if (attackingPoint) { //first get close to a "hit point" (point that will allow for an attack vector)
				float playerDistFromClosestEdge = FLT_MAX;
				float dist;
				glm::vec2 intersectionBary;
				glm::vec3 closestEdgeIntersection = glm::vec3(0.f, 0.f, 0.f);

				for (int i = 0; i < levelBB.size(); i = i + 3) {
					if (glm::intersectRayTriangle(playerPosition, mapUpVec, levelBB[i], levelBB[i + 1], levelBB[i + 2], intersectionBary, dist) && dist < playerDistFromClosestEdge && dist > 0.f) {
						playerDistFromClosestEdge = dist;
						float w = 1 - (intersectionBary.x + intersectionBary.y);
						closestEdgeIntersection = glm::vec3( //convert barycentric to world coord
							intersectionBary.x * levelBB[i].x + intersectionBary.y * levelBB[i + 1].x + w * levelBB[i + 2].x,
							intersectionBary.x * levelBB[i].y + intersectionBary.y * levelBB[i + 1].y + w * levelBB[i + 2].y,
							intersectionBary.x * levelBB[i].z + intersectionBary.y * levelBB[i + 1].z + w * levelBB[i + 2].z);
					}

					if (glm::intersectRayTriangle(playerPosition, mapDownVec, levelBB[i], levelBB[i + 1], levelBB[i + 2], intersectionBary, dist) && dist < playerDistFromClosestEdge && dist > 0.f) {
						playerDistFromClosestEdge = dist;
						float w = 1 - (intersectionBary.x + intersectionBary.y);
						closestEdgeIntersection = glm::vec3( //convert barycentric to world coord
							intersectionBary.x * levelBB[i].x + intersectionBary.y * levelBB[i + 1].x + w * levelBB[i + 2].x,
							intersectionBary.x * levelBB[i].y + intersectionBary.y * levelBB[i + 1].y + w * levelBB[i + 2].y,
							intersectionBary.x * levelBB[i].z + intersectionBary.y * levelBB[i + 1].z + w * levelBB[i + 2].z);
					}

					if (glm::intersectRayTriangle(playerPosition, mapLeftVec, levelBB[i], levelBB[i + 1], levelBB[i + 2], intersectionBary, dist) && dist < playerDistFromClosestEdge && dist > 0.f) {
						playerDistFromClosestEdge = dist;
						float w = 1 - (intersectionBary.x + intersectionBary.y);
						closestEdgeIntersection = glm::vec3( //convert barycentric to world coord
							intersectionBary.x * levelBB[i].x + intersectionBary.y * levelBB[i + 1].x + w * levelBB[i + 2].x,
							intersectionBary.x * levelBB[i].y + intersectionBary.y * levelBB[i + 1].y + w * levelBB[i + 2].y,
							intersectionBary.x * levelBB[i].z + intersectionBary.y * levelBB[i + 1].z + w * levelBB[i + 2].z);
					}

					if (glm::intersectRayTriangle(playerPosition, mapRightVec, levelBB[i], levelBB[i + 1], levelBB[i + 2], intersectionBary, dist) && dist < playerDistFromClosestEdge && dist > 0.f) {
						playerDistFromClosestEdge = dist;
						float w = 1 - (intersectionBary.x + intersectionBary.y);
						closestEdgeIntersection = glm::vec3( //convert barycentric to world coord
							intersectionBary.x * levelBB[i].x + intersectionBary.y * levelBB[i + 1].x + w * levelBB[i + 2].x,
							intersectionBary.x * levelBB[i].y + intersectionBary.y * levelBB[i + 1].y + w * levelBB[i + 2].y,
							intersectionBary.x * levelBB[i].z + intersectionBary.y * levelBB[i + 1].z + w * levelBB[i + 2].z);
					}
				}


				glm::vec3 ipVector = glm::vec3(playerPosition.x - closestEdgeIntersection.x + playerPosition.x, 0.f, playerPosition.z - closestEdgeIntersection.z + playerPosition.z); //project hit point on the other side of the player at the hitPointDist
				glm::vec3 hitPointVec = glm::vec3(ipVector.x * (hitPointDist / glm::length(ipVector)), 0.f, ipVector.z * (hitPointDist / glm::length(ipVector)));
				glm::vec3 hitPoint = glm::vec3(glm::sqrt(glm::pow(hitPointDist, 2) - glm::pow(hitPointVec.z, 2)), 0.f, glm::sqrt(glm::pow(hitPointDist, 2) - glm::pow(hitPointVec.x, 2)));

				attackPoint(hitPoint);

				if (calculateDistance(hitPoint.z, carPosition.z, hitPoint.x, carPosition.x) < hitPointDist) attackingPoint = false;
			}
			else {
				attackPoint(playerPosition); //hit point already covered -> attack player
			}



		//OTHER BEHAVIORS	
		}else {

			//ATTACK OPPORTUNITY
			if (playerPosition.x < -70 || playerPosition.x > 70 || playerPosition.z < -70 || playerPosition.z > 70) {
				std::cout << "attacking" << std::endl;
				currentState = attacking;
			}
			else {
				std::cout << "roaming" << std::endl;
				currentState = roaming;
			}


			//determine defense or roaming
			float curDistFromPlayer = calculateDistance(playerPosition.z, carPosition.z, playerPosition.x, carPosition.x);
			if (curDistFromPlayer < 25.f && prevDistanceFromPlayer > curDistFromPlayer) currentState = defending;
			else currentState = roaming;
			prevDistanceFromPlayer = curDistFromPlayer;

			//ROAMING
			if (currentState == roaming) {
				if (counter % 600 && rand() % 2 == 0) { //randomly turn
					turnTime = counter + rand() % 240 + 120;
					if (rand() % 2 == 0) turningRight = true; //and if it does turn there's a 50 50 chance it'll turn right or left
					else turningRight = false;
				}

				if (counter < turnTime) { //keep turning for up to 6 seconds (2 guaranteed + extra 0-240 frames extra)
					if (turningRight) carInputData->setAnalogSteer(-1.f);
					else carInputData->setAnalogSteer(1.f);
				}
				else carInputData->setAnalogSteer(0.f);

				carVehicle4W->mDriveDynData.forceGearChange(physx::PxVehicleGearsData::eFIRST); //by default just go straight
				setSpeed(maxSpeed * 0.7);
			}


			//DEFENDING
			if (currentState == defending) {
				setSpeed(maxSpeed);
				if (pointIsRight(glm::vec3(carPosition.x, 0.f, carPosition.z), glm::vec3(carPosition.x + carForwardVector.x, 0.f, carPosition.z + carForwardVector.z), glm::vec3(playerPosition.x, 0.f, playerPosition.z))) {
					carInputData->setAnalogSteer(1.f);
				}
				else {
					carInputData->setAnalogSteer(-1.f);
				}
			}
		}
	}
}



//OBSTACLE AVOIDANCE
int AIBehavior::shouldChangeCourse() { //determines whether it's close to & headed toward a hole, an object, or the edge of the map
	glm::vec2 intersectionBary;
	float dist = 0.f;


	for (int i = 0; i < levelBB.size(); i = i + 3) {

		if (glm::intersectRayTriangle(carPosition, carForwardVector, levelBB[i], levelBB[i + 1], levelBB[i + 2],intersectionBary, dist) && glm::abs(dist) < edgeDistLimit) {
			std::cout << "close" << std::endl;
			return 1;
		}
	}

	for (int i = 0; i < holeBBs.size(); i = i + 3) {
		if (glm::intersectRayTriangle(carPosition, carForwardVector, holeBBs[i], holeBBs[i + 1], holeBBs[i + 2], intersectionBary, dist) && glm::abs(dist) < holeDistLimit) {
			std::cout << "close"<< std::endl;
			return 1;
		}
	}

	return 0;
}










void AIBehavior::attackPoint(glm::vec3 pointToAttack) {

	float curDistance = calculateDistance(pointToAttack.z, carPosition.z, pointToAttack.x, carPosition.x);

	if (curDistance < 15.f
		&& carVehicle4W->computeForwardSpeed() < 7.5f && curDistance > prevFrameDistance) { //opponent too slow & too close & player not exploiting this
		carVehicle4W->mDriveDynData.forceGearChange(physx::PxVehicleGearsData::eREVERSE);
		setSpeed(maxSpeed);
		carInputData->setAnalogSteer(0.f);
	}
	else {
		carVehicle4W->mDriveDynData.forceGearChange(physx::PxVehicleGearsData::eFIRST);
		setSpeed(maxSpeed);
		if (pointIsRight(glm::vec3(carPosition.x, 0.f, carPosition.z), glm::vec3(carPosition.x + carForwardVector.x, 0.f, carPosition.z + carForwardVector.z), glm::vec3(pointToAttack.x, 0.f, pointToAttack.z))) {
			carInputData->setAnalogSteer(-0.7f);
		}
		else {
			carInputData->setAnalogSteer(0.7f);
		}
	}

	prevFrameDistance = curDistance;
}

void AIBehavior::setSpeed(float target) {
	float speed = carVehicle4W->computeForwardSpeed();
	if (speed < target) {
		carVehicle4W->mDriveDynData.forceGearChange(physx::PxVehicleGearsData::eFIRST);
		carInputData->setAnalogAccel(1.0f);
	}else {
		carVehicle4W->mDriveDynData.forceGearChange(physx::PxVehicleGearsData::eREVERSE);
		carInputData->setAnalogAccel(1.0f);
	}
}

void AIBehavior::initHoleBBs() {
	for (int i = 0; i < Utils::holesBBDatLen; i = i + 8) {
		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i], yMax, Utils::holesBoundingBoxData[i + 1]));
		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i], 0.f, Utils::holesBoundingBoxData[i + 1]));
		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i + 2], 0.f, Utils::holesBoundingBoxData[i + 3]));

		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i + 2], yMax, Utils::holesBoundingBoxData[i + 3]));
		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i + 2], 0.f, Utils::holesBoundingBoxData[i + 3]));
		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i], yMax, Utils::holesBoundingBoxData[i + 1]));



		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i + 4], yMax, Utils::holesBoundingBoxData[i + 5]));
		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i + 4], 0.f, Utils::holesBoundingBoxData[i + 5]));
		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i + 2], 0.f, Utils::holesBoundingBoxData[i + 3]));

		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i + 2], yMax, Utils::holesBoundingBoxData[i + 3]));
		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i + 2], 0.f, Utils::holesBoundingBoxData[i + 3]));
		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i + 4], yMax, Utils::holesBoundingBoxData[i + 5]));



		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i + 6], yMax, Utils::holesBoundingBoxData[i + 7]));
		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i + 6], 0.f, Utils::holesBoundingBoxData[i + 7]));
		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i + 4], 0.f, Utils::holesBoundingBoxData[i + 5]));

		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i + 4], yMax, Utils::holesBoundingBoxData[i + 5]));
		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i + 4], 0.f, Utils::holesBoundingBoxData[i + 5]));
		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i + 6], yMax, Utils::holesBoundingBoxData[i + 7]));



		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i], yMax, Utils::holesBoundingBoxData[i + 1]));
		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i], 0.f, Utils::holesBoundingBoxData[i + 1]));
		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i + 6], 0.f, Utils::holesBoundingBoxData[i + 7]));

		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i + 6], yMax, Utils::holesBoundingBoxData[i + 7]));
		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i + 6], 0.f, Utils::holesBoundingBoxData[i + 7]));
		holeBBs.push_back(glm::vec3(Utils::holesBoundingBoxData[i], yMax, Utils::holesBoundingBoxData[i + 1]));

	}
}

bool AIBehavior::pointIsRight(glm::vec3 linePoint1, glm::vec3 linePoint2, glm::vec3 pointToCheck) {
	return ((linePoint2.x - linePoint1.x) * (pointToCheck.z - linePoint1.z) - (linePoint2.z - linePoint1.z) * (pointToCheck.x - linePoint1.x)) > 0;
}

float AIBehavior::calculateAngleBetweenLines(float m2, float m1) {
	float degs = glm::degrees(glm::abs((m2 - m1) / (1 + m2 * m1)));
	//return fmod(degs, 360.f);
	return degs;
}

bool AIBehavior::noIntersection(float x, float y, float m1, float b1, float m2, float b2) {
	if ((m1 * x) + b1 != (m2 * x) + b2) return true;
	else return false;
}

float AIBehavior::calculateIntersectionX(float b2, float b1, float m1, float m2) {
	return (b2 - b1) / (m1 - m2);
}

float AIBehavior::calculateIntersectionY(float m1, float x, float b1) {
	return m1 * x + b1;
}

float AIBehavior::calculateLinearComp(float x, float y, float m) { //y = mx + B
	return y - (m * x);
}

float AIBehavior::calculateSlope(float y2, float y1, float x2, float x1) { //y = Mx + b
	return (y2 - y1) / (x2 - x1);
}

float AIBehavior::calculateDistance(float y2, float y1, float x2, float x1) { //Pythagorean
	return glm::sqrt(glm::pow(x2 - x1, 2) + glm::pow(y2 - y1, 2));
}

