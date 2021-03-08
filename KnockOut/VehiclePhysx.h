#ifndef VEHICLEPHYSX_H
#define VEHICLEPHYSX_H

#include "GL/glew.h"
#include <string>
#include "Mesh.h"

using std::string;
using namespace physx;
using namespace snippetvehicle;

class VehiclePhysx
{
public:
	void stepPhysics();
	void cleanupPhysics();
	void incrementDrivingMode(const PxF32 timestep);
	void initPhysics(std::vector<Mesh*> groundMeshes);
	void releaseAllControls();
	void startHandbrakeTurnRightMode();
	void startHandbrakeTurnLeftMode();
	void startTurnHardRightMode();
	void startTurnHardLeftMode();
	void startBrakeMode();
	void startAccelerateReverseMode();
	void startAccelerateForwardsMode();
	VehicleDesc initVehicleDesc();
	PxMat44 getVehicleTrans(int index);
	float getAngleAroundY();
	glm::vec3 getVehiclePos();
	glm::vec3 getGroundPos();
	glm::vec3 getBoxPos(int index);
	PxTransform getBoxTrans(int index);
	PxPhysics* getPhysx();
	PxScene* getScene();
	PxCooking* getCooking();
	void setGMimicKeyInputs(bool input);
	void forceGearChange(int input);
	void cookGroundMeshes(std::vector<Mesh*> groundMeshes);
	int getGameStatus();
	void checkGameOver();
	void reset();

	PxVehicleDrive4WRawInputData* getVehDat();
	glm::vec3 getOpponentPos();
	glm::vec3 getOpponentForVec();
	glm::vec3 getPlayerForVec();

private:
	void cookGroundMesh(Mesh* meshToCook);

};

#endif 

