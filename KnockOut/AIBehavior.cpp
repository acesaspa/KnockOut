#include "AIBehavior.h"


int counter = 0;
int attackCounter = 0;
bool attacking = false;
float prevFrameDistance = 0.f;

void AIBehavior::frameUpdate(physx::PxVehicleDrive4WRawInputData* carInputData, glm::vec3 carPos, glm::vec3 carForwardVec, glm::vec3 playerPos, glm::vec3 playerForwardVector,
	physx::PxVehicleDrive4W* opponentVehicle4W) {

	//TODO: remember it's turning and stop calculating

	//if (counter < 600) { //after 10 sec attack the player
	//	if (shouldTurn(carPos, carForwardVec) == 0) { //take a sec to turn
	//	//standard turn (only 1 edge is close - turn randomly)
	//		carInputData->setAnalogSteer(1.f);
	//	}
	//	else if (shouldTurn(carPos, carForwardVec) == 1) { //TODO: the corner decision logic seems pretty fcked
	//		//corner - turn left
	//		carInputData->setAnalogSteer(1.f);
	//	}
	//	else if (shouldTurn(carPos, carForwardVec) == 2) {
	//		//corner - turn right
	//		carInputData->setAnalogSteer(-1.f);
	//	}
	//	//else {
	//	//	//no turning necessary, after some time just turn arbitrarily
	//	//	carInputData->setAnalogSteer(0.f);
	//	//}
	//}
	//else { //attack the player
	//	turnTowardsPlayer(carPos, carForwardVec, playerPos, playerForwardVector, carInputData, counter);
	//}

	attackPlayer(carPos, carForwardVec, playerPos, playerForwardVector, carInputData, opponentVehicle4W);
}



void AIBehavior::attackPlayer(glm::vec3 opponentPos, glm::vec3 opponentForVec, glm::vec3 playerPos, glm::vec3 playerForVec,
	physx::PxVehicleDrive4WRawInputData* carInputData, physx::PxVehicleDrive4W* opponentVehicle4W) {

	float curDistance = calculateDistance(playerPos.z, opponentPos.z, playerPos.x, opponentPos.x);

	if (calculateDistance(playerPos.z, opponentPos.z, playerPos.x, opponentPos.x) < 15.f
		&& opponentVehicle4W->computeForwardSpeed() < 7.5f && curDistance > prevFrameDistance) { //opponent too slow & too close & player not exploiting this
		opponentVehicle4W->mDriveDynData.forceGearChange(physx::PxVehicleGearsData::eREVERSE);
		carInputData->setAnalogAccel(1.f);
		carInputData->setAnalogSteer(0.f);
	}
	else {

		opponentVehicle4W->mDriveDynData.forceGearChange(physx::PxVehicleGearsData::eFIRST);
		carInputData->setAnalogAccel(1.f);
		if (pointIsRight(glm::vec3(opponentPos.x, 0.f, opponentPos.z), glm::vec3(opponentPos.x + opponentForVec.x, 0.f, opponentPos.z + opponentForVec.z), glm::vec3(playerPos.x, 0.f, playerPos.z))) {
			carInputData->setAnalogSteer(-1.f);
		}
		else {
			carInputData->setAnalogSteer(1.f);
		}
	}

	prevFrameDistance = curDistance;
}







int AIBehavior::shouldChangeCourse(glm::vec3 pos, glm::vec3 forVec) { //determines whether it's close to & headed toward a hole, an object, or the edge of the map
	//only dealing with 2 dimens where y corresponds to our z

	int numOfEdgesClose = 0;

	float carSlope = calculateSlope(pos.z + forVec.z, pos.z, pos.x + forVec.x, pos.x);
	float carLinearComp = calculateLinearComp(pos.x, pos.z, carSlope);

	std::vector<float> edgeSlopes;
	std::vector<float> edgeLinComps;

	edgeSlopes.push_back(0.f); //right
	edgeLinComps.push_back(calculateLinearComp(-30.f, -25.f, edgeSlopes[0]));

	edgeSlopes.push_back(0.f); //bottom
	edgeLinComps.push_back(calculateLinearComp(25.f, -30.f, edgeSlopes[1]));

	edgeSlopes.push_back(0.f); //top
	edgeLinComps.push_back(calculateLinearComp(-25.f, 30.f, edgeSlopes[2]));

	edgeSlopes.push_back(0.f); //left
	edgeLinComps.push_back(calculateLinearComp(30.f, 25.f, edgeSlopes[3]));

	for (int i = 0; i < 4; i++) { //check if there is an intersection with any map edge & if the car is less then 5 units away

		float intersectionX;
		float intersectionY;

		if (i == 0) {
			intersectionX = -30.f;
			intersectionY = calculateIntersectionY(carSlope, intersectionX, carLinearComp);
		}
		else if (i == 1 || i == 2) {
			intersectionX = calculateIntersectionX(edgeLinComps[i], carLinearComp, carSlope, edgeSlopes[i]);
			intersectionY = calculateIntersectionY(carSlope, intersectionX, carLinearComp);
		}
		else {
			intersectionX = 30.f;
			intersectionY = calculateIntersectionY(carSlope, intersectionX, carLinearComp);
		}

		//if (noIntersection(intersectionX, intersectionY, carSlope, carLinearComp, edgeSlopes[i], edgeLinComps[i])) continue; //some edges won't intersect, gotta skip them
		glm::vec3 intersection = glm::vec3(intersectionX, 0.f, intersectionY); //the closest edge is the one we want to check
		float distance = calculateDistance(intersection.z, pos.z, intersection.x, pos.x);

		if (distance < 5.f) {
			//numOfEdgesClose++;
			return 0;
		}
	}
	
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
	return m1*x + b1;
}

float AIBehavior::calculateLinearComp(float x, float y, float m) { //y = mx + B
	return y - (m * x);
}

float AIBehavior::calculateSlope(float y2, float y1, float x2, float x1) { //y = Mx + b
	return (y2-y1) / (x2-x1);
}

float AIBehavior::calculateDistance(float y2, float y1, float x2, float x1) { //Pythagorean
	return glm::sqrt(glm::pow(x2-x1, 2) + glm::pow(y2-y1, 2));
}