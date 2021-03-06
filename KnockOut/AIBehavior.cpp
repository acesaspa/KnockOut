#include "AIBehavior.h"


int counter = 0;

void AIBehavior::frameUpdate(physx::PxVehicleDrive4WRawInputData* carInputData, glm::vec3 carPos, glm::vec3 carForwardVec) {
	counter++;
	carInputData->setAnalogAccel(0.3f);

	//TODO: remember it's turning and stop calculating
	if (shouldTurn(carPos, carForwardVec)) { //take a sec to turn
		carInputData->setAnalogSteer(1.f);
	}
	else {
		carInputData->setAnalogSteer(0.f);
	}
}

bool AIBehavior::shouldTurn(glm::vec3 pos, glm::vec3 forVec) {
	//only dealing with 2 dimens where y corresponds to our z

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
			return true;
		}
	}
	
	return false;
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