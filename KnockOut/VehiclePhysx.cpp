#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "..\KnockOut\Dependencies\imgui\imgui.h"
#include "..\KnockOut\Dependencies\GLFW\include\GLFW\glfw3.h"
#include "..\KnockOut\Dependencies\imgui\imgui_impl_glfw.h"
#include "..\KnockOut\Dependencies\imgui\imgui_impl_opengl3.h"
#include <iostream>
#include <vector>
#include <ctype.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
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

#include "VehiclePhysx.h"

using namespace physx;
using namespace snippetvehicle;

//MARK: Var
PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;
PxFoundation* gFoundation = NULL;
PxPhysics* gPhysics = NULL;
PxDefaultCpuDispatcher* gDispatcher = NULL;
PxScene* gScene = NULL;
PxCooking* gCooking = NULL;
PxMaterial* gMaterial = NULL;
PxPvd* gPvd = NULL;
VehicleSceneQueryData* gVehicleSceneQueryData = NULL;
PxBatchQuery* gBatchQuery = NULL;

PxVehicleDrivableSurfaceToTireFrictionPairs* gFrictionPairs = NULL;

PxRigidStatic* gGroundPlane = NULL;
//PxVehicleDrive4W* gVehicle4W1 = NULL;
//PxVehicleDrive4W* gVehicle4W2 = NULL;
//PxVehicleDrive4W* gVehicle4W3 = NULL;
//PxVehicleDrive4W* gVehicle4W4 = NULL;
int NumCars = 0;
PxVehicleDrive4W* Vehicles[4] = {NULL,NULL, NULL, NULL};
int GameStatus = 0;
PxRigidDynamic* gBox = NULL;
PxRigidDynamic* gBox2 = NULL;
PxRigidDynamic* gBox3 = NULL;
PxRigidStatic* meshBody1 = NULL;
PxRigidStatic* meshBody2 = NULL;
PxRigidStatic* meshBody3 = NULL;
bool changed = false;
bool					gIsVehicleInAir = true;


PxF32 gSteerVsForwardSpeedData[2 * 8] =
{
	0.0f,		0.75f,
	5.0f,		0.75f,
	30.0f,		0.125f,
	120.0f,		0.1f,
	PX_MAX_F32, PX_MAX_F32,
	PX_MAX_F32, PX_MAX_F32,
	PX_MAX_F32, PX_MAX_F32,
	PX_MAX_F32, PX_MAX_F32
};
PxFixedSizeLookupTable<8> gSteerVsForwardSpeedTable(gSteerVsForwardSpeedData, 4);

PxVehicleKeySmoothingData gKeySmoothingData =
{
	{
		6.0f,	//rise rate eANALOG_INPUT_ACCEL
		6.0f,	//rise rate eANALOG_INPUT_BRAKE		
		6.0f,	//rise rate eANALOG_INPUT_HANDBRAKE	
		2.5f,	//rise rate eANALOG_INPUT_STEER_LEFT
		2.5f,	//rise rate eANALOG_INPUT_STEER_RIGHT
	},
	{
		10.0f,	//fall rate eANALOG_INPUT_ACCEL
		10.0f,	//fall rate eANALOG_INPUT_BRAKE		
		10.0f,	//fall rate eANALOG_INPUT_HANDBRAKE	
		5.0f,	//fall rate eANALOG_INPUT_STEER_LEFT
		5.0f	//fall rate eANALOG_INPUT_STEER_RIGHT
	}
};

PxVehiclePadSmoothingData gPadSmoothingData =
{
	{
		6.0f,	//rise rate eANALOG_INPUT_ACCEL
		6.0f,	//rise rate eANALOG_INPUT_BRAKE		
		6.0f,	//rise rate eANALOG_INPUT_HANDBRAKE	
		2.5f,	//rise rate eANALOG_INPUT_STEER_LEFT
		2.5f,	//rise rate eANALOG_INPUT_STEER_RIGHT
	},
	{
		10.0f,	//fall rate eANALOG_INPUT_ACCEL
		10.0f,	//fall rate eANALOG_INPUT_BRAKE		
		10.0f,	//fall rate eANALOG_INPUT_HANDBRAKE	
		5.0f,	//fall rate eANALOG_INPUT_STEER_LEFT
		5.0f	//fall rate eANALOG_INPUT_STEER_RIGHT
	}
};

PxVehicleDrive4WRawInputData gVehicleInputData1;
PxVehicleDrive4WRawInputData gVehicleInputData2;
PxVehicleDrive4WRawInputData gVehicleInputData3;
PxVehicleDrive4WRawInputData gVehicleInputData4;

enum DriveMode
{
	eDRIVE_MODE_ACCEL_FORWARDS = 0,
	eDRIVE_MODE_ACCEL_REVERSE,
	eDRIVE_MODE_HARD_TURN_LEFT,
	eDRIVE_MODE_HANDBRAKE_TURN_LEFT,
	eDRIVE_MODE_HARD_TURN_RIGHT,
	eDRIVE_MODE_HANDBRAKE_TURN_RIGHT,
	eDRIVE_MODE_BRAKE,
	eDRIVE_MODE_NONE
};

DriveMode gDriveModeOrder[] =
{
	eDRIVE_MODE_BRAKE,
	eDRIVE_MODE_ACCEL_FORWARDS,
	eDRIVE_MODE_BRAKE,
	eDRIVE_MODE_ACCEL_REVERSE,
	eDRIVE_MODE_BRAKE,
	eDRIVE_MODE_HARD_TURN_LEFT,
	eDRIVE_MODE_BRAKE,
	eDRIVE_MODE_HARD_TURN_RIGHT,
	eDRIVE_MODE_ACCEL_FORWARDS,
	eDRIVE_MODE_HANDBRAKE_TURN_LEFT,
	eDRIVE_MODE_ACCEL_FORWARDS,
	eDRIVE_MODE_HANDBRAKE_TURN_RIGHT,
	eDRIVE_MODE_NONE
};

PxF32					gVehicleModeLifetime = 4.0f;
PxF32					gVehicleModeTimer = 0.0f;
PxU32					gVehicleOrderProgress = 0;
bool					gVehicleOrderComplete = false;
bool					gMimicKeyInputs = false;

void VehiclePhysx::updateNumCars() {
	PxBounds3 pxBounds2 = Vehicles[1]->getRigidDynamicActor()->getWorldBounds();
	PxTransform pos2 = Vehicles[1]->getRigidDynamicActor()->getGlobalPose();
	glm::vec3 cubePos2 = glm::vec3(pos2.p[0], pos2.p[1], pos2.p[2]);

	PxBounds3 pxBounds3 = Vehicles[2]->getRigidDynamicActor()->getWorldBounds();
	PxTransform pos3 = Vehicles[2]->getRigidDynamicActor()->getGlobalPose();
	glm::vec3 cubePos3 = glm::vec3(pos3.p[0], pos3.p[1], pos3.p[2]);

	PxBounds3 pxBounds4 = Vehicles[3]->getRigidDynamicActor()->getWorldBounds();
	PxTransform pos4 = Vehicles[3]->getRigidDynamicActor()->getGlobalPose();
	glm::vec3 cubePos4 = glm::vec3(pos4.p[0], pos4.p[1], pos4.p[2]);

	int temp = 0;
	if (cubePos2.y < -8 || cubePos2.y > 1000) {
		Vehicles[1]->getRigidDynamicActor()->setGlobalPose(PxTransform(0,1000000000, 0));
		Vehicles[1]->getRigidDynamicActor()->putToSleep();
		//std::cout << "test1\n";
		temp += 1;
	}
	if (cubePos3.y < -8 || cubePos3.y > 1000) {
		Vehicles[2]->getRigidDynamicActor()->setGlobalPose(PxTransform(0, 1000000000, 0));
		Vehicles[2]->getRigidDynamicActor()->putToSleep();
		//std::cout << "test2\n";
		temp += 1;
	}
	if (cubePos4.y < -8 || cubePos4.y > 1000) {
		Vehicles[3]->getRigidDynamicActor()->setGlobalPose(PxTransform(0, 1000000000, 0));
		Vehicles[3]->getRigidDynamicActor()->putToSleep();
		//std::cout << "test3\n";
		temp += 1;
	}

	if (temp != NumCars && temp!=3) {
		changed = true;
	}
	if (temp == 3) {
		GameStatus = 2;
	}

	NumCars = temp;
	//std::cout << NumCars << "\n";
}

int VehiclePhysx::getNumCars() {
	return NumCars;
}

bool VehiclePhysx::getChanged() {
	return changed;
}

void VehiclePhysx::setChanged(bool input) {
	changed = input;
}

VehicleDesc VehiclePhysx::initVehicleDesc(PxF32 mass)
{
	//Set up the chassis mass, dimensions, moment of inertia, and center of mass offset.
	//The moment of inertia is just the moment of inertia of a cuboid but modified for easier steering.
	//Center of mass offset is 0.65m above the base of the chassis and 0.25m towards the front.
	const PxF32 chassisMass = 300;
	const PxVec3 chassisDims(2.5f, 1.5f, 5.0f);
	const PxVec3 chassisMOI
	((chassisDims.y * chassisDims.y + chassisDims.z * chassisDims.z) * chassisMass / 8.0f,
		(chassisDims.x * chassisDims.x + chassisDims.z * chassisDims.z) * 0.8f * chassisMass / 8.0f,
		(chassisDims.x * chassisDims.x + chassisDims.y * chassisDims.y) * chassisMass / 8.0f);
	const PxVec3 chassisCMOffset(0.0f, -chassisDims.y * 0.5f + 0.65f, 0.25f);

	//Set up the wheel mass, radius, width, moment of inertia, and number of wheels.
	//Moment of inertia is just the moment of inertia of a cylinder.
	const PxF32 wheelMass = 10.0f;
	const PxF32 wheelRadius = 1.0f;
	const PxF32 wheelWidth = 0.2f;
	const PxF32 wheelMOI = 0.5f * wheelMass * wheelRadius * wheelRadius;
	const PxU32 nbWheels = 4;

	VehicleDesc vehicleDesc;

	vehicleDesc.chassisMass = chassisMass;
	vehicleDesc.chassisDims = chassisDims;
	vehicleDesc.chassisMOI = chassisMOI;
	vehicleDesc.chassisCMOffset = chassisCMOffset;
	vehicleDesc.chassisMaterial = gMaterial;
	vehicleDesc.chassisSimFilterData = PxFilterData(COLLISION_FLAG_CHASSIS, COLLISION_FLAG_CHASSIS_AGAINST, 0, 0);

	vehicleDesc.wheelMass = wheelMass;
	vehicleDesc.wheelRadius = wheelRadius;
	vehicleDesc.wheelWidth = wheelWidth;
	vehicleDesc.wheelMOI = wheelMOI;
	vehicleDesc.numWheels = nbWheels;
	vehicleDesc.wheelMaterial = gMaterial;
	vehicleDesc.wheelSimFilterData = PxFilterData(COLLISION_FLAG_WHEEL, COLLISION_FLAG_WHEEL_AGAINST, 0, 0);
	//vehicleDesc.wheelSimFilterData = PxFilterData(COLLISION_FLAG_WHEEL, COLLISION_FLAG_WHEEL_AGAINST, 0, 0);
	/*
	just a heads up, the above code 2 lines above this for chassisSimFilterData overrides the call further up.
	This means that the vehicle chassis WILL NOT collide with the ground when it flips over. If you want to fix this, simply comment
	that line and uncomment the one with wheelSimFilterData
	This does mean you will probably need code to flip the vehicle if it doesn't fall through
	*/
	return vehicleDesc;
}


void VehiclePhysx::applyGamepadInput(float leftStickX, float leftStickY, float rightStickX, float rightStickY) {
	if (leftStickY < -0.5) {
		Vehicles[0]->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
		gVehicleInputData1.setDigitalAccel(true);
	}
	else if (leftStickY > 0.5) {
		Vehicles[0]->mDriveDynData.forceGearChange(PxVehicleGearsData::eREVERSE);
		gVehicleInputData1.setDigitalAccel(true);
	}

	if (rightStickX < -0.5) {
		gVehicleInputData1.setDigitalSteerRight(true);
	}
	else if (rightStickX > 0.5) {
		gVehicleInputData1.setDigitalSteerLeft(true);
	}
}


void VehiclePhysx::startAccelerateForwardsMode()
{
	if (gMimicKeyInputs)
	{
		gVehicleInputData1.setDigitalAccel(true);
	}
	else
	{
		gVehicleInputData1.setAnalogAccel(1.0f);
	}
}

void VehiclePhysx::startAccelerateReverseMode()
{
	Vehicles[0]->mDriveDynData.forceGearChange(PxVehicleGearsData::eREVERSE);

	if (gMimicKeyInputs)
	{
		gVehicleInputData1.setDigitalAccel(true);
	}
	else
	{
		gVehicleInputData1.setAnalogAccel(1.0f);
	}
}

void VehiclePhysx::startBrakeMode()
{
	if (gMimicKeyInputs)
	{
		gVehicleInputData1.setDigitalBrake(true);
	}
	else
	{
		gVehicleInputData1.setAnalogBrake(1.0f);
	}
}

void VehiclePhysx::startTurnHardLeftMode()
{
	if (gMimicKeyInputs)
	{
		gVehicleInputData1.setDigitalAccel(0.1f);
		gVehicleInputData1.setDigitalSteerLeft(true);
	}
	else
	{
		gVehicleInputData1.setAnalogAccel(0.1f);
		gVehicleInputData1.setAnalogSteer(-1.0f);
	}
}

void VehiclePhysx::startTurnHardRightMode()
{
	if (gMimicKeyInputs)
	{
		gVehicleInputData1.setDigitalAccel(0.1f);
		gVehicleInputData1.setDigitalSteerRight(true);
	}
	else
	{
		gVehicleInputData1.setAnalogAccel(0.1f);
		gVehicleInputData1.setAnalogSteer(1.0f);
	}
}

void VehiclePhysx::startHandbrakeTurnLeftMode()
{
	if (gMimicKeyInputs)
	{
		gVehicleInputData1.setDigitalSteerLeft(true);
		gVehicleInputData1.setDigitalHandbrake(true);
	}
	else
	{
		gVehicleInputData1.setAnalogSteer(-1.0f);
		gVehicleInputData1.setAnalogHandbrake(1.0f);
	}
}

void VehiclePhysx::startHandbrakeTurnRightMode()
{
	if (gMimicKeyInputs)
	{
		gVehicleInputData1.setDigitalSteerRight(true);
		gVehicleInputData1.setDigitalHandbrake(true);
	}
	else
	{
		gVehicleInputData1.setAnalogSteer(1.0f);
		gVehicleInputData1.setAnalogHandbrake(1.0f);
	}
}


void VehiclePhysx::releaseAllControls()
{
	if (gMimicKeyInputs)
	{
		gVehicleInputData1.setDigitalAccel(false);
		gVehicleInputData1.setDigitalSteerLeft(false);
		gVehicleInputData1.setDigitalSteerRight(false);
		gVehicleInputData1.setDigitalBrake(false);
		gVehicleInputData1.setDigitalHandbrake(false);
	}
	else
	{
		gVehicleInputData1.setAnalogAccel(0.0f);
		gVehicleInputData1.setAnalogSteer(0.0f);
		gVehicleInputData1.setAnalogBrake(0.0f);
		gVehicleInputData1.setAnalogHandbrake(0.0f);
	}
}

void VehiclePhysx::removeGround(std::vector<Mesh*> groundMeshes) {
	if (meshBody1->getScene() != NULL) {
		gScene->removeActor(*meshBody1);
	}
	if (meshBody2->getScene() != NULL) {
		gScene->removeActor(*meshBody2);
	}
	if (meshBody3->getScene() != NULL) {
		gScene->removeActor(*meshBody3);
	}
	//gScene->removeActor(*meshBody2);
	//gScene->removeActor(*meshBody3);
	//gScene->removeActor()
	//meshBody->release();
	cookGroundMeshes(groundMeshes);
}

void VehiclePhysx::initPhysics(std::vector<Mesh*> groundMeshes)
{
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -30.f, 0.0f);

	PxU32 numWorkers = 1;
	gDispatcher = PxDefaultCpuDispatcherCreate(numWorkers);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = VehicleFilterShader; //VehicleFilterShader PxDefaultSimulationFilterShader

	gScene = gPhysics->createScene(sceneDesc);
	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	gMaterial = gPhysics->createMaterial(0.6f, 0.6f, 1.f);

	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));

	/////////////////////////////////////////////

	PxInitVehicleSDK(*gPhysics);
	PxVehicleSetBasisVectors(PxVec3(0, 1, 0), PxVec3(0, 0, 1));
	PxVehicleSetUpdateMode(PxVehicleUpdateMode::eVELOCITY_CHANGE);

	//Create the batched scene queries for the suspension raycasts.
	gVehicleSceneQueryData = VehicleSceneQueryData::allocate(1, PX_MAX_NB_WHEELS, 1, 1, WheelSceneQueryPreFilterBlocking, NULL, gAllocator);
	gBatchQuery = VehicleSceneQueryData::setUpBatchedSceneQuery(0, *gVehicleSceneQueryData, gScene);

	//Create the friction table for each combination of tire and surface type.
	gFrictionPairs = createFrictionPairs(gMaterial);

	//Create/cook level surface.
	PxFilterData groundPlaneSimFilterData(COLLISION_FLAG_GROUND, COLLISION_FLAG_GROUND_AGAINST, 0, 0);
	gGroundPlane = createDrivablePlane(groundPlaneSimFilterData, gMaterial, gPhysics);
	//gScene->addActor(*gGroundPlane); //TODO: remove, or whatever...
	cookGroundMeshes(groundMeshes);


	//Create a vehicle that will drive on the plane.
	for (int i = 0; i < 4; i++) {
		VehicleDesc vehicleDesc = initVehicleDesc(1000);
		Vehicles[i] = createVehicle4W(vehicleDesc, gPhysics, gCooking);
		PxTransform startTransform(PxVec3(-10.f*i, (vehicleDesc.chassisDims.y * 0.5f + vehicleDesc.wheelRadius + 2.0f), -10.f), PxQuat(PxIdentity));
		Vehicles[i]->getRigidDynamicActor()->setGlobalPose(startTransform);
		gScene->addActor(*Vehicles[i]->getRigidDynamicActor());
	}


	//Create a box for the vehicle to collide with
	PxTransform localTm(PxVec3(0, 20.0f, 0.0f));
	gBox = gPhysics->createRigidDynamic(localTm);
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(0.5f, 0.5f, 0.5f), *gMaterial);
	PxFilterData myData = PxFilterData();
	myData.word0 = 14;
	myData.word1 = 2;
	shape->setSimulationFilterData(myData);
	gBox->attachShape(*shape);
	gBox->setMass(50.f);
	gScene->addActor(*gBox);

	PxTransform localTm2(PxVec3(0.f, 3.0f, 10.0f));
	gBox2 = gPhysics->createRigidDynamic(localTm2);
	PxShape* shape2 = gPhysics->createShape(PxBoxGeometry(0.5f, 0.5f, 0.5f), *gMaterial);
	PxFilterData myData2 = PxFilterData();
	myData2.word0 = 14;
	myData2.word1 = 2;
	shape2->setSimulationFilterData(myData2);
	gBox2->attachShape(*shape2);
	gBox2->setMass(50.f);
	gScene->addActor(*gBox2);

	PxTransform localTm3(PxVec3(0, 6.0f, 10.0f));
	gBox3 = gPhysics->createRigidDynamic(localTm3);
	PxShape* shape3 = gPhysics->createShape(PxBoxGeometry(0.5f, 0.5f, 0.5f), *gMaterial);
	PxFilterData myData3 = PxFilterData();
	myData3.word0 = 14;
	myData3.word1 = 2;
	shape3->setSimulationFilterData(myData3);
	gBox3->attachShape(*shape3);
	gBox3->setMass(50.f);
	gScene->addActor(*gBox3);

	//Set the vehicle to rest in first gear.
	//Set the vehicle to use auto-gears.
	for (int i = 0; i < 4; i++) {
		Vehicles[i]->setToRestState();
		Vehicles[i]->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
		Vehicles[i]->mDriveDynData.setUseAutoGears(true);
	}

	gVehicleModeTimer = 0.0f;
	gVehicleOrderProgress = 0;
	startBrakeMode();
}

void VehiclePhysx::incrementDrivingMode(const PxF32 timestep)
{
	gVehicleModeTimer += timestep;
	if (gVehicleModeTimer > gVehicleModeLifetime)
	{
		//If the mode just completed was eDRIVE_MODE_ACCEL_REVERSE then switch back to forward gears.
		if (eDRIVE_MODE_ACCEL_REVERSE == gDriveModeOrder[gVehicleOrderProgress])
		{
			Vehicles[0]->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
		}

		//Increment to next driving mode.
		gVehicleModeTimer = 0.0f;
		gVehicleOrderProgress++;
		releaseAllControls();

		//If we are at the end of the list of driving modes then start again.
		if (eDRIVE_MODE_NONE == gDriveModeOrder[gVehicleOrderProgress])
		{
			gVehicleOrderProgress = 0;
			gVehicleOrderComplete = true;
		}

		//Start driving in the selected mode.
		DriveMode eDriveMode = gDriveModeOrder[gVehicleOrderProgress];
		switch (eDriveMode)
		{
		case eDRIVE_MODE_ACCEL_FORWARDS:
			startAccelerateForwardsMode();
			break;
		case eDRIVE_MODE_ACCEL_REVERSE:
			startAccelerateReverseMode();
			break;
		case eDRIVE_MODE_HARD_TURN_LEFT:
			startTurnHardLeftMode();
			break;
		case eDRIVE_MODE_HANDBRAKE_TURN_LEFT:
			startHandbrakeTurnLeftMode();
			break;
		case eDRIVE_MODE_HARD_TURN_RIGHT:
			startTurnHardRightMode();
			break;
		case eDRIVE_MODE_HANDBRAKE_TURN_RIGHT:
			startHandbrakeTurnRightMode();
			break;
		case eDRIVE_MODE_BRAKE:
			startBrakeMode();
			break;
		case eDRIVE_MODE_NONE:
			break;
		};

		//If the mode about to start is eDRIVE_MODE_ACCEL_REVERSE then switch to reverse gears.
		if (eDRIVE_MODE_ACCEL_REVERSE == gDriveModeOrder[gVehicleOrderProgress])
		{
			Vehicles[0]->mDriveDynData.forceGearChange(PxVehicleGearsData::eREVERSE);
		}
	}
}

PxVec3 VehiclePhysx::getRotation() {
	return Vehicles[0]->getRigidDynamicActor()->getGlobalPose().q.getBasisVector0();
	//return gVehicle4W->getRigidDynamicActor()->getGlobalPose().q.getAngle();
}

void VehiclePhysx::stepPhysics()
{

	const PxF32 timestep = 1.0f / 60.0f;

	//Cycle through the driving modes to demonstrate how to accelerate/reverse/brake/turn etc.
	//incrementDrivingMode(timestep);

	//Update the control inputs for the vehicle.
	if (gMimicKeyInputs)
	{
		PxVehicleDrive4WSmoothDigitalRawInputsAndSetAnalogInputs(gKeySmoothingData, gSteerVsForwardSpeedTable, gVehicleInputData1, timestep, gIsVehicleInAir, *Vehicles[0]);
	}
	else
	{
		PxVehicleDrive4WSmoothAnalogRawInputsAndSetAnalogInputs(gPadSmoothingData, gSteerVsForwardSpeedTable, gVehicleInputData1, timestep, gIsVehicleInAir, *Vehicles[0]);
	}

	PxVehicleDrive4WSmoothAnalogRawInputsAndSetAnalogInputs(gPadSmoothingData, gSteerVsForwardSpeedTable, gVehicleInputData2, timestep, gIsVehicleInAir, *Vehicles[1]);
	PxVehicleDrive4WSmoothAnalogRawInputsAndSetAnalogInputs(gPadSmoothingData, gSteerVsForwardSpeedTable, gVehicleInputData3, timestep, gIsVehicleInAir, *Vehicles[2]);
	PxVehicleDrive4WSmoothAnalogRawInputsAndSetAnalogInputs(gPadSmoothingData, gSteerVsForwardSpeedTable, gVehicleInputData4, timestep, gIsVehicleInAir, *Vehicles[3]);


	//FALL OFF
	/*
	PxVehicleWheels* vehicles2[2] = { gVehicle4W,gVehicle4W2 };
	PxBounds3 pxBounds = vehicles2[0]->getRigidDynamicActor()->getWorldBounds();
	PxTransform pos = vehicles2[0]->getRigidDynamicActor()->getGlobalPose();
	glm::vec3 cubePos = glm::vec3(pos.p[0], pos.p[1], pos.p[2]);

	PxBounds3 pxBounds2 = vehicles2[1]->getRigidDynamicActor()->getWorldBounds();
	PxTransform pos2 = vehicles2[1]->getRigidDynamicActor()->getGlobalPose();
	glm::vec3 cubePos2 = glm::vec3(pos2.p[0], pos2.p[1], pos2.p[2]);
	*/
	/*
	if (pos.p[2] >= 300 || pos.p[2] <= -300 || pos.p[0] >= 300 || pos.p[0] <= -300) {
		PxQuat vehicleQuaternion = gVehicle4W->getRigidDynamicActor()->getGlobalPose().q;
		gScene->removeActor(*gVehicle4W->getRigidDynamicActor());
		VehicleDesc vehicleDesc = initVehicleDesc();
		PxTransform startTransform(PxVec3(0, (vehicleDesc.chassisDims.y * 0.5f + vehicleDesc.wheelRadius + 1.0f), 0), PxQuat(vehicleQuaternion));
		gVehicle4W->getRigidDynamicActor()->setGlobalPose(startTransform);
		gScene->addActor(*gVehicle4W->getRigidDynamicActor());
	}
	if (pos2.p[2] >= 300 || pos2.p[2] <= -300 || pos2.p[0] >= 300 || pos2.p[0] <= -300) {
		PxQuat vehicleQuaternion = gVehicle4W2->getRigidDynamicActor()->getGlobalPose().q;
		gScene->removeActor(*gVehicle4W2->getRigidDynamicActor());
		VehicleDesc vehicleDesc = initVehicleDesc();
		PxTransform startTransform(PxVec3(15.f, (vehicleDesc.chassisDims.y * 0.5f + vehicleDesc.wheelRadius + 1.0f), 0), PxQuat(vehicleQuaternion));
		gVehicle4W2->getRigidDynamicActor()->setGlobalPose(startTransform);
		gScene->addActor(*gVehicle4W2->getRigidDynamicActor());
	}
	*/

	for (int i = 0; i < 4; i++) {
		//Raycasts.
		PxVehicleWheels* vehicles[1] = { Vehicles[i] };
		PxRaycastQueryResult* raycastResults = gVehicleSceneQueryData->getRaycastQueryResultBuffer(0);
		const PxU32 raycastResultsSize = gVehicleSceneQueryData->getQueryResultBufferSize();
		PxVehicleSuspensionRaycasts(gBatchQuery, 1, vehicles, raycastResultsSize, raycastResults);


		//Vehicle update.
		const PxVec3 grav = gScene->getGravity();
		PxWheelQueryResult wheelQueryResults[PX_MAX_NB_WHEELS];
		PxVehicleWheelQueryResult vehicleQueryResults[1] = { {wheelQueryResults, Vehicles[i]->mWheelsSimData.getNbWheels()} };

		PxVehicleUpdates(timestep, grav, *gFrictionPairs, 1, vehicles, vehicleQueryResults);
		//Work out if the vehicle is in the air.
		gIsVehicleInAir = Vehicles[i]->getRigidDynamicActor()->isSleeping() ? false : PxVehicleIsInAir(vehicleQueryResults[0]);

	}


	//Scene update.
	gScene->simulate(timestep);
	gScene->fetchResults(true);

	//std::cout << "Speed: " << gVehicle4W->computeForwardSpeed() << " accel: " << gVehicleInputData.getAnalogAccel() << std::endl;
}

void VehiclePhysx::cleanupPhysics()
{
	//gBox->free();
	for (int i = 0; i < 4; i++) {
		Vehicles[i]->getRigidDynamicActor()->release();
		Vehicles[i]->free();
	}
	PX_RELEASE(gBox);
	PX_RELEASE(gGroundPlane);
	PX_RELEASE(gBatchQuery);
	gVehicleSceneQueryData->free(gAllocator);
	PX_RELEASE(gFrictionPairs);
	PxCloseVehicleSDK();

	PX_RELEASE(gMaterial);
	PX_RELEASE(gCooking);
	PX_RELEASE(gScene);
	PX_RELEASE(gDispatcher);
	PX_RELEASE(gPhysics);
	if (gPvd)
	{
		PxPvdTransport* transport = gPvd->getTransport();
		gPvd->release();	gPvd = NULL;
		PX_RELEASE(transport);
	}
	PX_RELEASE(gFoundation);

	printf("SnippetVehicle4W done.\n");
}

PxMat44 VehiclePhysx::getVehicleTrans(int index) {
	PxVehicleWheels* vehicles[1] = { Vehicles[index-1] };
	PxBounds3 pxBounds = vehicles[0]->getRigidDynamicActor()->getWorldBounds();
	PxTransform pos = vehicles[0]->getRigidDynamicActor()->getGlobalPose();
	glm::vec3 cubePos = glm::vec3(pos.p[0], pos.p[1], pos.p[2]);
	PxMat44 pxTransMatrix = PxMat44(pos);
	return pxTransMatrix;
}

float VehiclePhysx::getAngleAroundY() {
	PxVehicleWheels* vehicles[1] = { Vehicles[0] };
	PxQuat vehicleQuaternion = vehicles[0]->getRigidDynamicActor()->getGlobalPose().q;
	PxVec3 v_dir = vehicleQuaternion.getBasisVector2();
	glm::vec3 dir = glm::normalize(glm::vec3(v_dir.x, 0, v_dir.z));
	float angleAroundY = glm::degrees(atan2(dir.z, dir.x));
	return angleAroundY;
}

glm::vec3 VehiclePhysx::getVehiclePos(int index) {
	PxVehicleWheels* vehicles[1] = { Vehicles[index-1] };
	PxBounds3 pxBounds = vehicles[0]->getRigidDynamicActor()->getWorldBounds();
	PxTransform pos = vehicles[0]->getRigidDynamicActor()->getGlobalPose();
	glm::vec3 cubePos = glm::vec3(pos.p[0], pos.p[1], pos.p[2]);
	return cubePos;
}

glm::vec3 VehiclePhysx::getGroundPos() {
	PxTransform pxGroundPos = gGroundPlane->getGlobalPose();
	glm::vec3 groundPos = glm::vec3(pxGroundPos.p[0], pxGroundPos.p[1], pxGroundPos.p[2]);
	return groundPos;
}

glm::vec3 VehiclePhysx::getBoxPos(int index) {
	glm::vec3 boxPos = glm::vec3(0,0,0);
	if (index == 1) {
		PxTransform pxBoxPos = gBox->getGlobalPose();
		glm::vec3 boxPos = glm::vec3(pxBoxPos.p[0], pxBoxPos.p[1], pxBoxPos.p[2]);
	}
	if (index == 2) {
		PxTransform pxBoxPos2 = gBox2->getGlobalPose();
		glm::vec3 boxPos = glm::vec3(pxBoxPos2.p[0], pxBoxPos2.p[1], pxBoxPos2.p[2]);
	}
	if (index == 3) {
		PxTransform pxBoxPos3 = gBox3->getGlobalPose();
		glm::vec3 boxPos = glm::vec3(pxBoxPos3.p[0], pxBoxPos3.p[1], pxBoxPos3.p[2]);
	}
	return boxPos;
}

PxTransform VehiclePhysx::getBoxTrans(int index) {

	if (index == 1) {
		return gBox->getGlobalPose();
	}
	if (index == 2) {
		return gBox2->getGlobalPose();
	}
	if (index == 3) {
		return gBox3->getGlobalPose();
	}
}

PxPhysics* VehiclePhysx::getPhysx() {
	return gPhysics;
}

PxScene* VehiclePhysx::getScene() {
	return gScene;
}

PxCooking* VehiclePhysx::getCooking() {
	return gCooking;
}

void VehiclePhysx::setGMimicKeyInputs(bool input) {
	gMimicKeyInputs = input;
}

void VehiclePhysx::forceGearChange(int input) {
	Vehicles[0]->mDriveDynData.forceGearChange(input);
}







//MARK: AI
PxVehicleDrive4WRawInputData* VehiclePhysx::getVehDat(int i) {
	if(i == 1) return &gVehicleInputData2;
	if(i == 2) return &gVehicleInputData3;
	if(i == 3) return &gVehicleInputData4;
}

glm::vec3 VehiclePhysx::getOpponentPos(int i) {
	PxVehicleWheels* vehicles[1] = { Vehicles[i] };
	PxBounds3 pxBounds = vehicles[0]->getRigidDynamicActor()->getWorldBounds();
	PxTransform pos = vehicles[0]->getRigidDynamicActor()->getGlobalPose();
	glm::vec3 cubePos = glm::vec3(pos.p[0], pos.p[1], pos.p[2]);
	return cubePos;
}

glm::vec3 VehiclePhysx::getOpponentForVec(int i) {
	PxVehicleWheels* vehicles[1] = { Vehicles[i] };
	PxQuat vehicleQuaternion = vehicles[0]->getRigidDynamicActor()->getGlobalPose().q;
	PxVec3 vDir = vehicleQuaternion.getBasisVector2();
	return glm::vec3(vDir.x, vDir.y, vDir.z);
}

PxVehicleDrive4W* VehiclePhysx::getVehicle4W(int i) {
	return Vehicles[i];
}

glm::vec3 VehiclePhysx::getPlayerForVec() {
	PxVehicleWheels* vehicles[1] = { Vehicles[0] };
	PxQuat vehicleQuaternion = vehicles[0]->getRigidDynamicActor()->getGlobalPose().q;
	PxVec3 vDir = vehicleQuaternion.getBasisVector2();
	return glm::vec3(vDir.x, vDir.y, vDir.z);
}





//MARK: Cooooking
void VehiclePhysx::cookGroundMeshes(std::vector<Mesh*> groundMeshes) {
	//std::cout << groundMeshes.size() << "\n";
	for (int i = 0; i < groundMeshes.size(); i++) {
		cookGroundMesh(groundMeshes[i],i);
	}
}

void VehiclePhysx::cookGroundMesh(Mesh* meshToCook,int i) {
	PxMaterial* gMaterial = gPhysics->createMaterial(0.8f, 0.8f, 0.1f); //create some material
	PxTriangleMeshDesc meshDesc; //mesh cooking from a triangle mesh

	std::vector<PxVec3> vertices = meshToCook->getActualVertices();
	std::vector<PxU32> indices = meshToCook->getVertexIndices();

	meshDesc.points.count = vertices.size(); //total number of vertices
	meshDesc.points.stride = sizeof(PxVec3);
	meshDesc.points.data = reinterpret_cast<const void*>(vertices.data());

	meshDesc.triangles.count = indices.size() / 3; //total number of triangles (each index = 1 vertex, so divide by 3 to get the num of triangles)
	meshDesc.triangles.stride = 3 * sizeof(PxU32);
	meshDesc.triangles.data = reinterpret_cast<const void*>(indices.data());

	//std::cout << "actual verts: " << meshDesc.points.count << std::endl;
	//std::cout << "actual inds: " << meshDesc.triangles.count * 3 << std::endl;

	//PxCookingParams params = gCooking->getParams();
	////TODO: potentially do this
	//gCooking->setParams(params);

	//PxFilterData myData = PxFilterData();
	//myData.word0 = 14;
	//myData.word1 = 2;

	PxFilterData groundPlaneSimFilterData(COLLISION_FLAG_GROUND, COLLISION_FLAG_GROUND_AGAINST, 0, 0);

	PxTriangleMesh* triMesh = NULL;
	PxU32 meshSize = 0;
	triMesh = gCooking->createTriangleMesh(meshDesc, gPhysics->getPhysicsInsertionCallback()); //insert the cooked mesh directly into PxPhysics
	if (i == 0) {
		meshBody1 = NULL;
		meshBody1 = gPhysics->createRigidStatic(PxTransform(PxVec3(0, 0, 0))); //create a rigid body for the cooked mesh
		PxShape* meshShape = gPhysics->createShape(PxTriangleMeshGeometry(triMesh), *gMaterial); //create a shape from the cooked mesh

		PxFilterData qryFilterData;
		setupDrivableSurface(qryFilterData);
		meshShape->setQueryFilterData(qryFilterData);
		meshShape->setSimulationFilterData(groundPlaneSimFilterData);

		meshShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);

		meshBody1->attachShape(*meshShape); //attach the shape to the body
		gScene->addActor(*meshBody1); //and add it to the scene
		triMesh->release(); //clean up
	}
	if (i == 1) {
		meshBody2 = NULL;
		meshBody2 = gPhysics->createRigidStatic(PxTransform(PxVec3(0, 0, 0))); //create a rigid body for the cooked mesh
		PxShape* meshShape = gPhysics->createShape(PxTriangleMeshGeometry(triMesh), *gMaterial); //create a shape from the cooked mesh

		PxFilterData qryFilterData;
		setupDrivableSurface(qryFilterData);
		meshShape->setQueryFilterData(qryFilterData);
		meshShape->setSimulationFilterData(groundPlaneSimFilterData);

		meshShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);

		meshBody2->attachShape(*meshShape); //attach the shape to the body
		gScene->addActor(*meshBody2); //and add it to the scene
		triMesh->release(); //clean up
	}
	if (i == 2) {
		meshBody3 = NULL;
		meshBody3 = gPhysics->createRigidStatic(PxTransform(PxVec3(0, 0, 0))); //create a rigid body for the cooked mesh
		PxShape* meshShape = gPhysics->createShape(PxTriangleMeshGeometry(triMesh), *gMaterial); //create a shape from the cooked mesh

		PxFilterData qryFilterData;
		setupDrivableSurface(qryFilterData);
		meshShape->setQueryFilterData(qryFilterData);
		meshShape->setSimulationFilterData(groundPlaneSimFilterData);

		meshShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);

		meshBody3->attachShape(*meshShape); //attach the shape to the body
		gScene->addActor(*meshBody3); //and add it to the scene
		triMesh->release(); //clean up
	}
}

int VehiclePhysx::getGameStatus() {
	return GameStatus;
}

void VehiclePhysx::checkGameOver() {
	PxBounds3 pxBounds1 = Vehicles[0]->getRigidDynamicActor()->getWorldBounds();
	PxTransform pos1 = Vehicles[0]->getRigidDynamicActor()->getGlobalPose();
	glm::vec3 cubePos1 = glm::vec3(pos1.p[0], pos1.p[1], pos1.p[2]);

	PxBounds3 pxBounds2 = Vehicles[0]->getRigidDynamicActor()->getWorldBounds();
	PxTransform pos2 = Vehicles[0]->getRigidDynamicActor()->getGlobalPose();
	glm::vec3 cubePos2 = glm::vec3(pos2.p[0], pos2.p[1], pos2.p[2]);

	PxBounds3 pxBounds3 = Vehicles[0]->getRigidDynamicActor()->getWorldBounds();
	PxTransform pos3 = Vehicles[0]->getRigidDynamicActor()->getGlobalPose();
	glm::vec3 cubePos3 = glm::vec3(pos3.p[0], pos3.p[1], pos3.p[2]);

	PxBounds3 pxBounds4 = Vehicles[0]->getRigidDynamicActor()->getWorldBounds();
	PxTransform pos4 = Vehicles[0]->getRigidDynamicActor()->getGlobalPose();
	glm::vec3 cubePos4 = glm::vec3(pos4.p[0], pos4.p[1], pos4.p[2]);

	/*
	Playing: 0
	Game Over: 1
	You Win: 2
	*/

	if (cubePos1.y < -10) {
		GameStatus = 1;
	}

	if (((cubePos1.y < -10 && cubePos2.y < -10 && cubePos3.y < -10)||(cubePos1.y > 1000 && cubePos2.y > 1000 && cubePos3.y > 1000)) && cubePos1.y > 0) {
		GameStatus = 2;
	}
}

void VehiclePhysx::setGameStatus(int status) {
	GameStatus = status;
}

void VehiclePhysx::reset() {
	
	for (int i = 0; i < 4; i++) {
		gScene->removeActor(*Vehicles[i]->getRigidDynamicActor());
		VehicleDesc vehicleDesc = initVehicleDesc(1000);
		Vehicles[i] = createVehicle4W(vehicleDesc, gPhysics, gCooking);
		PxTransform startTransform(PxVec3(-10.f * i, (vehicleDesc.chassisDims.y * 0.5f + vehicleDesc.wheelRadius + 2.0f), -10.f), PxQuat(PxIdentity));
		Vehicles[i]->getRigidDynamicActor()->setGlobalPose(startTransform);
		gScene->addActor(*Vehicles[i]->getRigidDynamicActor());
	}
}

void VehiclePhysx::applyForce(PxVec3 force, int index) {

	Vehicles[index-1]->getRigidDynamicActor()->addForce(force);
}

void VehiclePhysx::stopVehicle(int index) {
	if (index == 1) {
		Vehicles[0]->getRigidDynamicActor()->setLinearVelocity(PxVec3(0.f, 0.f, 0.f));
	}
	if (index == 2) {
		Vehicles[1]->getRigidDynamicActor()->setLinearVelocity(PxVec3(0.f, 0.f, 0.f));
		gVehicleInputData2.setAnalogHandbrake(1.f);
	}
	if (index == 3) {
		Vehicles[2]->getRigidDynamicActor()->setLinearVelocity(PxVec3(0.f, 0.f, 0.f));
		gVehicleInputData3.setAnalogHandbrake(1.f);
	}
	if (index == 4) {
		Vehicles[3]->getRigidDynamicActor()->setLinearVelocity(PxVec3(0.f, 0.f, 0.f));
		gVehicleInputData4.setAnalogHandbrake(1.f);
	}
}