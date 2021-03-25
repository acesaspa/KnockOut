#include "AIBehavior.h"

float prevFrameDistance = 0.f;
unsigned int counter = 0;
unsigned int turnTime = 0;
bool turningRight = true;
float prevDistanceFromPlayer = 0.f;
float maxSpeed = 30.f;
float hitPointDist = 30.f;
float hitPointRad = 10.f;
float playerForVecMultiplier = 5.f;
bool attackingPoint = true; //true = attacking hit point, false = attacking player directly, needs to be set to true when switching to attacking state

glm::vec3 mapUpVec = glm::vec3(0.f, 0.5f, 1.f);
glm::vec3 mapDownVec = glm::vec3(0.f, 0.5f, -1.f);
glm::vec3 mapRightVec = glm::vec3(-1.f, 0.5f, 0.f);
glm::vec3 mapLeftVec = glm::vec3(1.f, 0.5f, 0.f);

int opponentState = 2; //0 = roaming, 1 = defending, 2 = attacking

void AIBehavior::frameUpdate(physx::PxVehicleDrive4WRawInputData* carInputDat, glm::vec3 carPos, glm::vec3 carForwardVec, glm::vec3 playerPos, glm::vec3 playerForwardVec, physx::PxVehicleDrive4W* carVeh4W) {

	playerForwardVector = playerForwardVec;
	//TODO: hit prediction instead
	float pFVlen = glm::sqrt(glm::pow(playerForwardVector.x, 2) + glm::pow(playerForwardVector.z, 2));
	playerPosition = playerPos + glm::vec3(playerForwardVector.x * (playerForVecMultiplier/pFVlen), 0.f, playerForwardVector.z * (playerForVecMultiplier / pFVlen));
	carPosition = carPos;
	carForwardVector = carForwardVec;
	carInputData = carInputDat;
	carVehicle4W = carVeh4W;
	counter++;

	behave();
}



void AIBehavior::behave() {

	//AVOIDING OBSTACLES
	if (shouldChangeCourse() == 1) {
		//carInputData->setAnalogHandbrake(true);
		carInputData->setAnalogSteer(1.f);
		setSpeed(maxSpeed * 0.4);
	}else {


		//ATTACKING
		if (opponentState == 2) { //prioritize attacking state over every other state
			//setSpeed(maxSpeed);
			carInputData->setAnalogAccel(1.f);

			if (attackingPoint) { //first get close to a "hit point" (point that will allow for an attack vector)
				std::cout << "point" << std::endl;
				float playerDistFromClosestEdge = FLT_MAX;
				float dist;
				glm::vec2 intersectionBary;
				glm::vec3 closestEdgeIntersection = glm::vec3(0.f,0.f,0.f);

				for (int i = 0; i < boundingBox.size(); i = i + 3) {
					if (glm::intersectRayTriangle(playerPosition, mapUpVec, boundingBox[i], boundingBox[i + 1], boundingBox[i + 2], intersectionBary, dist) && dist < playerDistFromClosestEdge && dist > 0.f) {
						playerDistFromClosestEdge = dist;
						float w = 1 - (intersectionBary.x + intersectionBary.y);
						closestEdgeIntersection = glm::vec3( //convert barycentric to world coord
							intersectionBary.x * boundingBox[i].x + intersectionBary.y * boundingBox[i + 1].x + w * boundingBox[i + 2].x,
							intersectionBary.x * boundingBox[i].y + intersectionBary.y * boundingBox[i + 1].y + w * boundingBox[i + 2].y,
							intersectionBary.x * boundingBox[i].z + intersectionBary.y * boundingBox[i + 1].z + w * boundingBox[i + 2].z);
					}

					if (glm::intersectRayTriangle(playerPosition, mapDownVec, boundingBox[i], boundingBox[i + 1], boundingBox[i + 2], intersectionBary, dist) && dist < playerDistFromClosestEdge && dist > 0.f) {
						playerDistFromClosestEdge = dist;
						float w = 1 - (intersectionBary.x + intersectionBary.y);
						closestEdgeIntersection = glm::vec3( //convert barycentric to world coord
							intersectionBary.x * boundingBox[i].x + intersectionBary.y * boundingBox[i + 1].x + w * boundingBox[i + 2].x,
							intersectionBary.x * boundingBox[i].y + intersectionBary.y * boundingBox[i + 1].y + w * boundingBox[i + 2].y,
							intersectionBary.x * boundingBox[i].z + intersectionBary.y * boundingBox[i + 1].z + w * boundingBox[i + 2].z);
					}

					if (glm::intersectRayTriangle(playerPosition, mapLeftVec, boundingBox[i], boundingBox[i + 1], boundingBox[i + 2], intersectionBary, dist) && dist < playerDistFromClosestEdge && dist > 0.f) {
						playerDistFromClosestEdge = dist;
						float w = 1 - (intersectionBary.x + intersectionBary.y);
						closestEdgeIntersection = glm::vec3( //convert barycentric to world coord
							intersectionBary.x * boundingBox[i].x + intersectionBary.y * boundingBox[i + 1].x + w * boundingBox[i + 2].x,
							intersectionBary.x * boundingBox[i].y + intersectionBary.y * boundingBox[i + 1].y + w * boundingBox[i + 2].y,
							intersectionBary.x * boundingBox[i].z + intersectionBary.y * boundingBox[i + 1].z + w * boundingBox[i + 2].z);
					}

					if (glm::intersectRayTriangle(playerPosition, mapRightVec, boundingBox[i], boundingBox[i + 1], boundingBox[i + 2], intersectionBary, dist) && dist < playerDistFromClosestEdge && dist > 0.f) {
						playerDistFromClosestEdge = dist;
						float w = 1 - (intersectionBary.x + intersectionBary.y);
						closestEdgeIntersection = glm::vec3( //convert barycentric to world coord
							intersectionBary.x * boundingBox[i].x + intersectionBary.y * boundingBox[i + 1].x + w * boundingBox[i + 2].x,
							intersectionBary.x * boundingBox[i].y + intersectionBary.y * boundingBox[i + 1].y + w * boundingBox[i + 2].y,
							intersectionBary.x * boundingBox[i].z + intersectionBary.y * boundingBox[i + 1].z + w * boundingBox[i + 2].z);
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
			//determine defense or roaming
			float curDistFromPlayer = calculateDistance(playerPosition.z, carPosition.z, playerPosition.x, carPosition.x);
			if (curDistFromPlayer < 25.f && prevDistanceFromPlayer > curDistFromPlayer) opponentState = 1;
			else opponentState = 0;
			prevDistanceFromPlayer = curDistFromPlayer;

			//ROAMING
			if (opponentState == 0) {
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
			if (opponentState == 1) {
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







int AIBehavior::shouldChangeCourse() { //determines whether it's close to & headed toward a hole, an object, or the edge of the map
	glm::vec2 intersectionBary;
	float dist = 0.f;

	for (int i = 0; i < boundingBox.size(); i = i + 3) {

		if (glm::intersectRayTriangle(carPosition, carForwardVector, boundingBox[i], boundingBox[i + 1], boundingBox[i + 2],intersectionBary, dist) && dist < 100.f && dist > 0.f) {
			return 1;
		}
	}

	for (int i = 0; i < Utils::holesBBDatLen; i = i + 9) {
		if (glm::intersectRayTriangle(carPosition, carForwardVector,
			glm::vec3(Utils::holesBoundingBoxData[i], Utils::holesBoundingBoxData[i + 1], Utils::holesBoundingBoxData[i + 2]),
			glm::vec3(Utils::holesBoundingBoxData[i + 3], Utils::holesBoundingBoxData[i + 4], Utils::holesBoundingBoxData[i + 5]),
			glm::vec3(Utils::holesBoundingBoxData[i + 6], Utils::holesBoundingBoxData[i + 7], Utils::holesBoundingBoxData[i + 8]),
			intersectionBary, dist) && dist < 50.f && dist > 0.f) {
			return 1;
		}
	}

	return 0;
}




void AIBehavior::setSpeed(float target) {
	float speed = carVehicle4W->computeForwardSpeed();
	//std::cout << "speed: " << speed << std::endl;
	if (speed < target) {
		carVehicle4W->mDriveDynData.forceGearChange(physx::PxVehicleGearsData::eFIRST);
		carInputData->setAnalogAccel(1.0f);
	}else {
		carVehicle4W->mDriveDynData.forceGearChange(physx::PxVehicleGearsData::eREVERSE);
		carInputData->setAnalogAccel(1.0f);
	}

	//if (numOfEdgesClose == 1) return 0;
	//else {

	//}

	return -1;
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