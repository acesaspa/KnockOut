#include "Utils.h"


//MARK: Variables & Data
bool Utils::getFreeCamMode() {
	return false;
}

float Utils::cubeVertexData[] = {
	// positions          // normals           // texture coords
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
	 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
};
int Utils::cubeArrayLen = 288;

float Utils::planeVertexData[] = {
	//positions              //normals           //texture coords
   -300.0f,  0.0f,  300.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
	300.0f,  0.0f,  300.0f,  0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
	300.0f,  0.0f, -300.0f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f,

   -300.0f,  0.0f, -300.0f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
   -300.0f,  0.0f,  300.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
	300.0f,  0.0f, -300.0f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f
};
int Utils::planeArrayLen = 48;


float Utils::holesBoundingBoxData[] = { //TODO: measure using player pos & imgUI, 4 sided
	-16.f, 11, //city 1
	-15.8f, 29.f,
	-29.f, 29.f,
	-27.f, 15.f,

	11.4f, 2.f, //city 2
	23.f, 4.f,
	25.f, -11.f,
	15.5f, -16.f
};

int Utils::holesBBDatLen = 16;












//MARK: Functions

glm::mat4 Utils::getGlmMatFromPxMat(physx::PxMat44 pxMat) //converts a PhysX 4x4 matrix to a GLM 4x4 matrix
{
	glm::mat4 glmMat = glm::mat4(1.0f);

	glmMat[0].x = pxMat.column0[0];
	glmMat[0].y = pxMat.column0[1];
	glmMat[0].z = pxMat.column0[2];
	glmMat[0].w = pxMat.column0[3];

	glmMat[1].x = pxMat.column1[0];
	glmMat[1].y = pxMat.column1[1];
	glmMat[1].z = pxMat.column1[2];
	glmMat[1].w = pxMat.column1[3];

	glmMat[2].x = pxMat.column2[0];
	glmMat[2].y = pxMat.column2[1];
	glmMat[2].z = pxMat.column2[2];
	glmMat[2].w = pxMat.column2[3];

	glmMat[3].x = pxMat.column3[0];
	glmMat[3].y = pxMat.column3[1];
	glmMat[3].z = pxMat.column3[2];
	glmMat[3].w = pxMat.column3[3];

	return glmMat;
}

glm::vec3 Utils::getRigidDynamicPos(physx::PxRigidDynamic* pxRigid) {
	physx::PxTransform transMat = pxRigid->getGlobalPose();
	return glm::vec3(transMat.p[0], transMat.p[1], transMat.p[2]);
}

glm::vec3 Utils::getRigidStaticPos(physx::PxRigidStatic* pxRigid) {
	physx::PxTransform transMat = pxRigid->getGlobalPose();
	return glm::vec3(transMat.p[0], transMat.p[1], transMat.p[2]);
}

//std::vector<float> Utils::getBBcords(float cornerCoords[]) {
//	//TODO: standardize stuff
//	return;
//}



//MARK: Useful Stuff
	//model = glm::translate(model, cubePos); //model matrix converts the local coordinates (cubePosition) to the global world coordinates
	//model = glm::rotate(model, glm::radians(0.f), glm::vec3(0.0f, 1.0f, 0.0f));
	//model = glm::scale(model, glm::vec3(pxBounds.getDimensions().x, pxBounds.getDimensions().y, pxBounds.getDimensions().z));
