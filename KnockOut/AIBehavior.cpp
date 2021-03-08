#include "AIBehavior.h"


int counter = 0;

void AIBehavior::frameUpdate(physx::PxVehicleDrive4WRawInputData* carInputData, glm::vec3 carPos, glm::vec3 carForwardVec, glm::vec3 playerPos, glm::vec3 playerForwardVector) {
	counter++;
	carInputData->setAnalogAccel(0.5f);
	carInputData->setAnalogSteer(-1.f);

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

	turnTowardsPlayer(carPos, carForwardVec, playerPos, playerForwardVector, carInputData, counter);
}



void AIBehavior::turnTowardsPlayer(glm::vec3 opponentPos, glm::vec3 opponentForVec, glm::vec3 playerPos, glm::vec3 playerForVec, physx::PxVehicleDrive4WRawInputData* carInputData, int counter) {
	float opponentSlope = calculateSlope(opponentPos.z + opponentForVec.z, opponentPos.z, opponentPos.x + opponentForVec.x, opponentPos.x);
	float playerSlope = calculateSlope(playerPos.z + playerForVec.z, playerPos.z, playerPos.x + playerForVec.x, playerPos.x);

	float opponentLinComp = calculateLinearComp(opponentPos.x, opponentPos.z, opponentSlope);
	float playerLinComp = calculateLinearComp(playerPos.x, playerPos.z, playerSlope);

	intersectionX = calculateIntersectionX(opponentLinComp, playerLinComp, playerSlope, opponentSlope); //where vector lines intersect
	intersectionY = calculateIntersectionY(opponentSlope, intersectionX, opponentLinComp);

	float distCarToIntersection = calculateDistance(intersectionY, opponentPos.z, intersectionX, opponentPos.x);
	float distVecToIntersection = calculateDistance(intersectionY, opponentPos.z + opponentForVec.z, intersectionX, opponentPos.x + opponentForVec.x);
	float distInterToPlayerOrigin = calculateDistance(intersectionY, playerPos.z, intersectionX, playerPos.x);
	float tolerance = 2.f;


	float angle = calculateAngleBetweenLines(calculateSlope(playerPos.z + playerForVec.z, playerPos.z, playerPos.x + playerForVec.x, playerPos.x),
		calculateSlope(opponentPos.z + opponentForVec.z, opponentPos.z, opponentPos.x + opponentForVec.x, opponentPos.x));


	//std::cout << "dist car to inter: " << distCarToIntersection << " dist vec to inter: " << distVecToIntersection << " dist inter to player or: " << distInterToPlayerOrigin << std::endl;
	if (distCarToIntersection > distVecToIntersection && distInterToPlayerOrigin <= tolerance) { //intersection forward & within tol -> WE HAVE AN ATTACK VECTOR
		std::cout << "GOT ATTACK VECTOR" << std::endl;
		carInputData->setAnalogSteer(0.f);
		carInputData->setAnalogAccel(1.f);
	}
	else if (counter%20 == 0) { //else slow down, and turn in the right direction
		//carInputData->setAnalogAccel(0.5f);
		if (pointIsRight(glm::vec3(opponentPos.x, 0.f, opponentPos.z), glm::vec3(intersectionX, 0.f, intersectionY), glm::vec3(playerPos.x, 0.f, playerPos.z))) {

			//TURNING RIGHT
			carInputData->setAnalogAccel(1.f);
			carInputData->setAnalogSteer(-0.5f);
			std::cout << "player right, turning right" << std::endl;
		}
		else {

			//TURNING LEFT
			carInputData->setAnalogAccel(1.f);
			carInputData->setAnalogSteer(0.5f);
			std::cout << "player left, turning left" << std::endl;
		}
	}
}







int AIBehavior::shouldTurn(glm::vec3 pos, glm::vec3 forVec) {
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