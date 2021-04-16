#include "AIBehavior.h"

Opponent::Opponent() {
	initHoleBBs();
	//testLocs.push_back(glm::vec3(0.f,0.f,0.f));
}

void Opponent::setAttacking() {
	currentState = attacking;
}


void Opponent::frameUpdate(physx::PxVehicleDrive4WRawInputData* carInputDat, glm::vec3 carPos, glm::vec3 carForwardVec, glm::vec3 playerPos, glm::vec3 playerForwardVec,
	physx::PxVehicleDrive4W* carVeh4W,
	physx::PxVehicleDrive4W* playerVeh4W) {

	playerForwardVector = playerForwardVec;
	playerPosition = playerPos;
	carPosition = carPos;
	carForwardVector = carForwardVec;
	carLeftVec = glm::cross(glm::vec3(carPosition.x, 5.f, carPosition.z), carForwardVector);
	carRightVec = glm::cross(carForwardVector, glm::vec3(carPosition.x, 5.f, carPosition.z));
	carInputData = carInputDat;
	carVehicle4W = carVeh4W;
	playerVehicle4W = playerVeh4W;
	counter++;

	behave();
}


void Opponent::behave() {
	adjustCourse();
}

//TODO: make sure the AI can't pick up another one




//OBSTACLE AVOIDANCE
void Opponent::adjustCourse() { //finds the closest
	glm::vec2 intersectionBary;
	glm::vec3 intersectionWorld;
	float dist = 0.f;
	float closestDist = FLT_MAX;
	int turnDir = 0; //0 = don't, 1 = right, 2 = left
	bool holesInPath = false;

	if (lockCount < 1) { //if our behavior not locked
		for (int i = 0; i < levelBB.size(); i = i + 3) { //CLOSE TO THE EDGE OF THE CURRENT SEGMENT
			if (glm::intersectRayTriangle(carPosition, carForwardVector, levelBB[i], levelBB[i + 1], levelBB[i + 2], intersectionBary, dist) && dist < holeDistLimit && dist > 0.f) {

				if (dist < closestDist && dist > 0.f) {
					closestDist = dist;
					float w = 1 - (intersectionBary.x + intersectionBary.y);
					intersectionWorld = glm::vec3( //convert barycentric to world coord
						intersectionBary.x * levelBB[i].x + intersectionBary.y * levelBB[i + 1].x + w * levelBB[i + 2].x,
						intersectionBary.x * levelBB[i].y + intersectionBary.y * levelBB[i + 1].y + w * levelBB[i + 2].y,
						intersectionBary.x * levelBB[i].z + intersectionBary.y * levelBB[i + 1].z + w * levelBB[i + 2].z);
					float p12 = calculateDistance(intersectionWorld.z, levelBB[i].z, intersectionWorld.x, levelBB[i].x);
					float p13 = calculateDistance(intersectionWorld.z, carPosition.z, intersectionWorld.x, carPosition.x);
					float p23 = calculateDistance(levelBB[i].z, carPosition.z, levelBB[i].x, carPosition.x);
					float angle = glm::acos(glm::pow(p12, 2) + glm::pow(p13, 2) - glm::pow(p23, 2)) / (2 * p12 * p13);

					if (pointIsRight(carPosition, intersectionWorld, levelBB[i])) {
						if (angle < 90.f) turnDir = 2; //turn left
						else turnDir = 1; //turn right
					}
					else {
						if (angle < 90.f) turnDir = 1; //turn right
						else turnDir = 2; //turn left
					}
				}
			}
		}

		for (int i = 0; i < holeBBs.size(); i = i + 3) { //CLOSE TO AN EDGE OF A HOLE
			if (glm::intersectRayTriangle(carPosition, carForwardVector, holeBBs[i], holeBBs[i + 1], holeBBs[i + 2], intersectionBary, dist) && dist < holeDistLimit && dist > 0.f) {

				if (dist < closestDist && dist > 0.f) {
					closestDist = dist;
					float w = 1 - (intersectionBary.x + intersectionBary.y);
					intersectionWorld = glm::vec3( //convert barycentric to world coord
						intersectionBary.x * holeBBs[i].x + intersectionBary.y * holeBBs[i + 1].x + w * holeBBs[i + 2].x,
						intersectionBary.x * holeBBs[i].y + intersectionBary.y * holeBBs[i + 1].y + w * holeBBs[i + 2].y,
						intersectionBary.x * holeBBs[i].z + intersectionBary.y * holeBBs[i + 1].z + w * holeBBs[i + 2].z);
					float p12 = calculateDistance(intersectionWorld.z, holeBBs[i].z, intersectionWorld.x, holeBBs[i].x);
					float p13 = calculateDistance(intersectionWorld.z, carPosition.z, intersectionWorld.x, carPosition.x);
					float p23 = calculateDistance(holeBBs[i].z, carPosition.z, holeBBs[i].x, carPosition.x);
					float angle = glm::acos(glm::pow(p12, 2) + glm::pow(p13, 2) - glm::pow(p23, 2)) / (2 * p12 * p13);

					if (pointIsRight(carPosition, intersectionWorld, holeBBs[i])) {
						if (angle < 90.f) turnDir = 2; //turn left
						else turnDir = 1; //turn right
					}
					else {
						if (angle < 90.f) turnDir = 1; //turn right
						else turnDir = 2; //turn left
					}
				}
			}
		}

		for (int i = 0; i < holeBBs.size(); i = i + 3) { //HOLES between the car and the waypoint
			if (glm::intersectRayTriangle(carPosition, glm::vec3(currentWayPoint.x, 0.f, currentWayPoint.y), holeBBs[i], holeBBs[i + 1], holeBBs[i + 2], intersectionBary, dist) && glm::abs(dist) < holeDistLimit) {
				float w = 1 - (intersectionBary.x + intersectionBary.y);
				intersectionWorld = glm::vec3( //convert barycentric to world coord
					intersectionBary.x * holeBBs[i].x + intersectionBary.y * holeBBs[i + 1].x + w * holeBBs[i + 2].x,
					intersectionBary.x * holeBBs[i].y + intersectionBary.y * holeBBs[i + 1].y + w * holeBBs[i + 2].y,
					intersectionBary.x * holeBBs[i].z + intersectionBary.y * holeBBs[i + 1].z + w * holeBBs[i + 2].z);

				float wayPointDist = calculateDistance(carPosition.z, currentWayPoint.y, carPosition.x, currentWayPoint.x);
				if (calculateDistance(intersectionWorld.z, carPosition.z, intersectionWorld.x, carPosition.x) < wayPointDist &&
					calculateDistance(intersectionWorld.z, currentWayPoint.y, intersectionWorld.x, currentWayPoint.x) < wayPointDist) {
					holesInPath = true;
					break;
				}
			}
		}


		if (closestDist < 10.f) { //SUPER CLOSE, BACK OFF
			lockCount = 45;
			currentSpeedMultiplier = -1.f;
			currentAnalogSteer = -1.f;
			setSpeed(currentSpeedMultiplier * maxSpeed);
			carInputData->setAnalogSteer(currentAnalogSteer);
		}
		else if (closestDist < 20.f) { //SOMEWHAT CLOSE, SLOW DOWN & TURN
			currentSpeedMultiplier = 0.3;
			if (turnDir == 1) currentAnalogSteer = -1.f;
			else if (turnDir == 2) currentAnalogSteer = 1.f;
			setSpeed(currentSpeedMultiplier * maxSpeed);
			carInputData->setAnalogSteer(currentAnalogSteer);
		}
		else if (holesInPath && currentPowerUp == none) { //NO EDGES OR HOLES CLOSE BUT HOLES BETWEEN THE CAR AND THE CURRENT WAYPOINT
			currentSpeedMultiplier = 0.5;
			currentAnalogSteer = -0.3f;
			setSpeed(currentSpeedMultiplier * maxSpeed);
			carInputData->setAnalogSteer(currentAnalogSteer);
		}
		else { //CLEAR PATH & NOTHING CLOSE
			if (currentPowerUp == none) {
				pursueCurrentWayPoint();
			}
			else if (currentPowerUp == attack) {
				attackPlayer();
			}
		}
	}
	else { //BEHAVIOR LOCKDOWN (just keep going with the currently locked behavior)
		setSpeed(currentSpeedMultiplier * maxSpeed);
		carInputData->setAnalogSteer(currentAnalogSteer);
		lockCount--;
	}
}











void Opponent::attackPlayer() {
	float curDistance = calculateDistance(playerPosition.z, carPosition.z, playerPosition.x, carPosition.x);
	glm::vec2 intersection = getIntersection(playerPosition, playerPosition+playerForwardVector, carPosition, carPosition+carForwardVector);
	float interCarPosDist = calculateDistance(intersection.y, playerPosition.z, intersection.x, playerPosition.x);
	float carVecPlayerPosDist = calculateDistance(carPosition.z + carForwardVector.z, playerPosition.z, carPosition.x + carForwardVector.x, playerPosition.x);

	if (curDistance < 10.f && //gotta be close
		interCarPosDist < 2.f && //have to be aimed at player's car
		carVecPlayerPosDist < curDistance) { //and have to be aimed at it in front of the car, not behind
		lockCount = 30;
		useAttack();
		currentPowerUp = none;
	}
	else if (curDistance < 15.f && carVehicle4W->computeForwardSpeed() < 7.5f && curDistance > prevFrameDistance) { //opponent too slow & too close & player not exploiting this
		setSpeed(-maxSpeed);
		carInputData->setAnalogSteer(0.f);
	}
	else {
		carVehicle4W->mDriveDynData.forceGearChange(physx::PxVehicleGearsData::eFIRST);
		setSpeed(maxSpeed);
		if (pointIsRight(glm::vec3(carPosition.x, 0.f, carPosition.z), glm::vec3(carPosition.x + carForwardVector.x, 0.f, carPosition.z + carForwardVector.z), glm::vec3(playerPosition.x, 0.f, playerPosition.z))) {
			carInputData->setAnalogSteer(-0.7f); //turn right
		}
		else {
			carInputData->setAnalogSteer(0.7f); //turn left
		}
	}

	prevFrameDistance = curDistance;
}

void Opponent::setPowerUp(int pUp) {
	switch (pUp) {
	case 1:
		break;
	case 2:
		currentPowerUp = attack;
		break;
	case 3:
		break;
	}
}

void Opponent::useAttack() {
	glm::mat4 rotation = glm::rotate(glm::mat4{ 1.f }, float(-M_PI / 2.f), glm::vec3(0, 1, 0));
	physx::PxVec3 pre = (carVehicle4W->getRigidDynamicActor()->getGlobalPose().q.getBasisVector0() + physx::PxVec3(0.f, 0.02f, 0.f));
	glm::vec4 rot = glm::vec4(pre.x, pre.y, pre.z, 0.f);
	glm::vec4 rotated = rotation * rot;
	carVehicle4W->getRigidDynamicActor()->addForce(950000 * physx::PxVec3(rotated.x, rotated.y, rotated.z));
}

bool Opponent::hasPowerUp() {
	if (currentPowerUp == none) return false;
	else return true;
}















//MARK: DONE & GONE

void Opponent::setNextWayPoint() {
	if(oneSegmentLeft) currentWayPoint = Utils::aiPoints[rand() % 13]; //only use the city waypoints if we're down to the last car
	else currentWayPoint = Utils::aiPoints[rand() % Utils::aiPoints.size()];
}

void Opponent::pursueCurrentWayPoint() {
	float curDistance = calculateDistance(currentWayPoint.y, carPosition.z, currentWayPoint.x, carPosition.x);

	if (curDistance < 15.f) { //way point reached, set a new one
		setNextWayPoint();
	}
	else {
		carVehicle4W->mDriveDynData.forceGearChange(physx::PxVehicleGearsData::eFIRST);
		setSpeed(0.8 * maxSpeed);
		if (pointIsRight(glm::vec3(carPosition.x, 0.f, carPosition.z), glm::vec3(carPosition.x + carForwardVector.x, 0.f, carPosition.z + carForwardVector.z), glm::vec3(currentWayPoint.x, 0.f, currentWayPoint.y))) {
			carInputData->setAnalogSteer(-0.7f); //turn right
		}
		else {
			carInputData->setAnalogSteer(0.7f); //turn left
		}
	}

	prevFrameDistance = curDistance;
}

void Opponent::setSpeed(float target) {
	float speed = carVehicle4W->computeForwardSpeed();
	if (speed < target) {
		carVehicle4W->mDriveDynData.forceGearChange(physx::PxVehicleGearsData::eFIRST);
		carInputData->setAnalogAccel(1.0f);
	}else {
		carVehicle4W->mDriveDynData.forceGearChange(physx::PxVehicleGearsData::eREVERSE);
		carInputData->setAnalogAccel(1.0f);
	}
}

void Opponent::initHoleBBs() {
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

void Opponent::updateLevelBB(std::vector<glm::vec3> lvlBB, bool lastSegment) {
	levelBB = lvlBB;
	oneSegmentLeft = lastSegment;
}

void Opponent::startEvac() {
	std::cout << "evac started" << std::endl;
	currentWayPoint = glm::vec3(0.f, 0.f, 0.f);
}

bool Opponent::pointIsRight(glm::vec3 linePoint1, glm::vec3 linePoint2, glm::vec3 pointToCheck) {
	return ((linePoint2.x - linePoint1.x) * (pointToCheck.z - linePoint1.z) - (linePoint2.z - linePoint1.z) * (pointToCheck.x - linePoint1.x)) > 0;
}

float Opponent::calculateAngleBetweenLines(float m2, float m1) {
	float degs = glm::degrees(glm::abs((m2 - m1) / (1 + m2 * m1)));
	//return fmod(degs, 360.f);
	return degs;
}

bool Opponent::noIntersection(float x, float y, float m1, float b1, float m2, float b2) {
	if ((m1 * x) + b1 != (m2 * x) + b2) return true;
	else return false;
}

glm::vec2 Opponent::getIntersection(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {

	float A1 = p1.z - p0.z;
	float B1 = p0.x - p1.x;
	float C1 = A1 * p0.x + B1 * p0.z;
	float A2 = p3.z - p2.z;
	float B2 = p2.x - p3.x;
	float C2 = A2 * p2.x + B2 * p2.z;
	float denominator = A1 * B2 - A2 * B1;

	return glm::vec2((B2 * C1 - B1 * C2) / denominator, (A1 * C2 - A2 * C1) / denominator);
}

float Opponent::calculateIntersectionX(float b2, float b1, float m1, float m2) {
	return (b2 - b1) / (m1 - m2);
}

float Opponent::calculateIntersectionY(float m1, float x, float b1) {
	return m1 * x + b1;
}

float Opponent::calculateLinearComp(float x, float y, float m) { //y = mx + B
	return y - (m * x);
}

float Opponent::calculateSlope(float y2, float y1, float x2, float x1) { //y = Mx + b
	return (y2 - y1) / (x2 - x1);
}

float Opponent::calculateDistance(float y2, float y1, float x2, float x1) { //Pythagorean
	return glm::sqrt(glm::pow(x2 - x1, 2) + glm::pow(y2 - y1, 2));
}


//HIT PREDICTION
//float pFVlen = glm::sqrt(glm::pow(playerForwardVector.x, 2) + glm::pow(playerForwardVector.z, 2));
//float predictionDistance = playerVehicle4W->computeForwardSpeed() * (calculateDistance(playerPosition.z, carPosition.z, playerPosition.x, carPosition.x) / carVehicle4W->computeForwardSpeed());
//std::cout << "prediction: " << predictionDistance << std::endl;
//float t = predictionDistance / pFVlen;
//
//testLocs[0] = glm::vec3((1 - t) * playerPosition.x + t * playerForwardVector.x, playerPosition.y,
//	(1 - t) * playerPosition.z + t * playerForwardVector.z);
//
////https://math.stackexchange.com/questions/175896/finding-a-point-along-a-line-a-certain-distance-away-from-another-point

