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
	VehicleDesc initVehicleDesc(PxF32 mass);
	PxMat44 getVehicleTrans(int index);
	float getAngleAroundY();
	glm::vec3 getVehiclePos(int index);
	glm::vec3 getGroundPos();
	//glm::vec3 getBoxPos(int index);
	//PxTransform getBoxTrans(int index);
	PxPhysics* getPhysx();
	PxScene* getScene();
	PxCooking* getCooking();
	void setGMimicKeyInputs(bool input);
	void forceGearChange(int input);
	void cookGroundMeshes(std::vector<Mesh*> groundMeshes);
	int getGameStatus();
	void checkGameOver();
	void reset();
	void applyForce(PxVec3 force, int index);
	PxVec3 getRotation();
	void stopVehicle(int index);
	void removeGround(std::vector<Mesh*> groundMeshes);
	void setGameStatus(int status);
	int getNumCars();
	void updateNumCars();
	bool getChanged();
	void setChanged(bool input);
	void applyGamepadInput(float leftStickX, float leftStickY, float rightStickX, float rightStickY);

	PxVehicleDrive4WRawInputData* getVehDat(int i);
	glm::vec3 getOpponentPos(int i);
	glm::vec3 getOpponentForVec(int i);
	glm::vec3 getPlayerForVec();
	PxVehicleDrive4W* getVehicle4W(int i);

private:
	void cookGroundMesh(Mesh* meshToCook, int i);

};

#endif 

