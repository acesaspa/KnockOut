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
PxVehicleDrive4W* gVehicle4W = NULL;
PxVehicleDrive4W* gVehicle4W2 = NULL;
int GameStatus = 0;
PxRigidDynamic* gBox = NULL;
PxRigidDynamic* gBox2 = NULL;
PxRigidDynamic* gBox3 = NULL;
PxRigidStatic* meshBody = NULL;

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

PxVehicleDrive4WRawInputData gVehicleInputData;
PxVehicleDrive4WRawInputData gVehicleInputData2;

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


void VehiclePhysx::startAccelerateForwardsMode()
{
	if (gMimicKeyInputs)
	{
		gVehicleInputData.setDigitalAccel(true);
	}
	else
	{
		gVehicleInputData.setAnalogAccel(1.0f);
	}
}

void VehiclePhysx::startAccelerateReverseMode()
{
	gVehicle4W->mDriveDynData.forceGearChange(PxVehicleGearsData::eREVERSE);

	if (gMimicKeyInputs)
	{
		gVehicleInputData.setDigitalAccel(true);
	}
	else
	{
		gVehicleInputData.setAnalogAccel(1.0f);
	}
}

void VehiclePhysx::startBrakeMode()
{
	if (gMimicKeyInputs)
	{
		gVehicleInputData.setDigitalBrake(true);
	}
	else
	{
		gVehicleInputData.setAnalogBrake(1.0f);
	}
}

void VehiclePhysx::startTurnHardLeftMode()
{
	if (gMimicKeyInputs)
	{
		gVehicleInputData.setDigitalAccel(0.1f);
		gVehicleInputData.setDigitalSteerLeft(true);
	}
	else
	{
		gVehicleInputData.setAnalogAccel(0.1f);
		gVehicleInputData.setAnalogSteer(-1.0f);
	}
}

void VehiclePhysx::startTurnHardRightMode()
{
	if (gMimicKeyInputs)
	{
		gVehicleInputData.setDigitalAccel(0.1f);
		gVehicleInputData.setDigitalSteerRight(true);
	}
	else
	{
		gVehicleInputData.setAnalogAccel(0.1f);
		gVehicleInputData.setAnalogSteer(1.0f);
	}
}

void VehiclePhysx::startHandbrakeTurnLeftMode()
{
	if (gMimicKeyInputs)
	{
		gVehicleInputData.setDigitalSteerLeft(true);
		gVehicleInputData.setDigitalHandbrake(true);
	}
	else
	{
		gVehicleInputData.setAnalogSteer(-1.0f);
		gVehicleInputData.setAnalogHandbrake(1.0f);
	}
}

void VehiclePhysx::startHandbrakeTurnRightMode()
{
	if (gMimicKeyInputs)
	{
		gVehicleInputData.setDigitalSteerRight(true);
		gVehicleInputData.setDigitalHandbrake(true);
	}
	else
	{
		gVehicleInputData.setAnalogSteer(1.0f);
		gVehicleInputData.setAnalogHandbrake(1.0f);
	}
}


void VehiclePhysx::releaseAllControls()
{
	if (gMimicKeyInputs)
	{
		gVehicleInputData.setDigitalAccel(false);
		gVehicleInputData.setDigitalSteerLeft(false);
		gVehicleInputData.setDigitalSteerRight(false);
		gVehicleInputData.setDigitalBrake(false);
		gVehicleInputData.setDigitalHandbrake(false);
	}
	else
	{
		gVehicleInputData.setAnalogAccel(0.0f);
		gVehicleInputData.setAnalogSteer(0.0f);
		gVehicleInputData.setAnalogBrake(0.0f);
		gVehicleInputData.setAnalogHandbrake(0.0f);
	}
}

void VehiclePhysx::removeGround(std::vector<Mesh*> groundMeshes) {
	gScene->removeActor(*meshBody);
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
	sceneDesc.gravity = PxVec3(0.0f, -20.f, 0.0f);

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
	VehicleDesc vehicleDesc = initVehicleDesc(1000);
	gVehicle4W = createVehicle4W(vehicleDesc, gPhysics, gCooking);
	PxTransform startTransform(PxVec3(-10.f, (vehicleDesc.chassisDims.y * 0.5f + vehicleDesc.wheelRadius + 2.0f), -10.f), PxQuat(PxIdentity));
	gVehicle4W->getRigidDynamicActor()->setGlobalPose(startTransform);
	gScene->addActor(*gVehicle4W->getRigidDynamicActor());

	VehicleDesc vehicleDesc2 = initVehicleDesc(1000);
	gVehicle4W2 = createVehicle4W(vehicleDesc2, gPhysics, gCooking);
	PxTransform startTransform2(PxVec3(10.f, (vehicleDesc2.chassisDims.y * 0.5f + vehicleDesc2.wheelRadius + 2.0f), 10.f), PxQuat(PxIdentity));
	gVehicle4W2->getRigidDynamicActor()->setGlobalPose(startTransform2);
	gScene->addActor(*gVehicle4W2->getRigidDynamicActor());


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
	gVehicle4W->setToRestState();
	gVehicle4W->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
	gVehicle4W->mDriveDynData.setUseAutoGears(true);

	gVehicle4W2->setToRestState();
	gVehicle4W2->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
	gVehicle4W2->mDriveDynData.setUseAutoGears(true);

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
			gVehicle4W->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
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
			gVehicle4W->mDriveDynData.forceGearChange(PxVehicleGearsData::eREVERSE);
		}
	}
}

PxVec3 VehiclePhysx::getRotation() {
	return gVehicle4W->getRigidDynamicActor()->getGlobalPose().q.getBasisVector0();
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
		PxVehicleDrive4WSmoothDigitalRawInputsAndSetAnalogInputs(gKeySmoothingData, gSteerVsForwardSpeedTable, gVehicleInputData, timestep, gIsVehicleInAir, *gVehicle4W);
	}
	else
	{
		PxVehicleDrive4WSmoothAnalogRawInputsAndSetAnalogInputs(gPadSmoothingData, gSteerVsForwardSpeedTable, gVehicleInputData, timestep, gIsVehicleInAir, *gVehicle4W);
	}

	PxVehicleDrive4WSmoothAnalogRawInputsAndSetAnalogInputs(gPadSmoothingData, gSteerVsForwardSpeedTable, gVehicleInputData2, timestep, gIsVehicleInAir, *gVehicle4W2);

	//FALL OFF
	PxVehicleWheels* vehicles2[2] = { gVehicle4W,gVehicle4W2 };
	PxBounds3 pxBounds = vehicles2[0]->getRigidDynamicActor()->getWorldBounds();
	PxTransform pos = vehicles2[0]->getRigidDynamicActor()->getGlobalPose();
	glm::vec3 cubePos = glm::vec3(pos.p[0], pos.p[1], pos.p[2]);

	PxBounds3 pxBounds2 = vehicles2[1]->getRigidDynamicActor()->getWorldBounds();
	PxTransform pos2 = vehicles2[1]->getRigidDynamicActor()->getGlobalPose();
	glm::vec3 cubePos2 = glm::vec3(pos2.p[0], pos2.p[1], pos2.p[2]);

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


	//Raycasts.
	PxVehicleWheels* vehicles[1] = { gVehicle4W };
	PxRaycastQueryResult* raycastResults = gVehicleSceneQueryData->getRaycastQueryResultBuffer(0);
	const PxU32 raycastResultsSize = gVehicleSceneQueryData->getQueryResultBufferSize();
	PxVehicleSuspensionRaycasts(gBatchQuery, 1, vehicles, raycastResultsSize, raycastResults);


	//Vehicle update.
	const PxVec3 grav = gScene->getGravity();
	PxWheelQueryResult wheelQueryResults[PX_MAX_NB_WHEELS];
	PxVehicleWheelQueryResult vehicleQueryResults[1] = { {wheelQueryResults, gVehicle4W->mWheelsSimData.getNbWheels()} };

	PxVehicleUpdates(timestep, grav, *gFrictionPairs, 1, vehicles, vehicleQueryResults);
	//Work out if the vehicle is in the air.
	gIsVehicleInAir = gVehicle4W->getRigidDynamicActor()->isSleeping() ? false : PxVehicleIsInAir(vehicleQueryResults[0]);


	//Raycasts car 2.
	PxVehicleWheels* vehicles3[1] = { gVehicle4W2 };
	PxRaycastQueryResult* raycastResults2 = gVehicleSceneQueryData->getRaycastQueryResultBuffer(0);
	const PxU32 raycastResultsSize2 = gVehicleSceneQueryData->getQueryResultBufferSize();
	PxVehicleSuspensionRaycasts(gBatchQuery, 1, vehicles3, raycastResultsSize2, raycastResults2);


	//Vehicle update.
	//const PxVec3 grav = gScene->getGravity();
	PxWheelQueryResult wheelQueryResults2[PX_MAX_NB_WHEELS];
	PxVehicleWheelQueryResult vehicleQueryResults2[1] = { {wheelQueryResults2, gVehicle4W2->mWheelsSimData.getNbWheels()} };

	PxVehicleUpdates(timestep, grav, *gFrictionPairs, 1, vehicles3, vehicleQueryResults2);
	//Work out if the vehicle is in the air.
	gIsVehicleInAir = gVehicle4W2->getRigidDynamicActor()->isSleeping() ? false : PxVehicleIsInAir(vehicleQueryResults2[0]);



	//Scene update.
	gScene->simulate(timestep);
	gScene->fetchResults(true);

	//std::cout << "Speed: " << gVehicle4W->computeForwardSpeed() << " accel: " << gVehicleInputData.getAnalogAccel() << std::endl;
}

void VehiclePhysx::cleanupPhysics()
{
	//gBox->free();

	gVehicle4W->getRigidDynamicActor()->release();
	gVehicle4W->free();
	gVehicle4W2->getRigidDynamicActor()->release();
	gVehicle4W2->free();
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
	if (index == 1) {
		PxVehicleWheels* vehicles[1] = { gVehicle4W };
		PxBounds3 pxBounds = vehicles[0]->getRigidDynamicActor()->getWorldBounds();
		PxTransform pos = vehicles[0]->getRigidDynamicActor()->getGlobalPose();
		glm::vec3 cubePos = glm::vec3(pos.p[0], pos.p[1], pos.p[2]);
		PxMat44 pxTransMatrix = PxMat44(pos);
		return pxTransMatrix;
	}
	if (index == 2) {
		PxVehicleWheels* vehicles2[1] = { gVehicle4W2 };
		PxBounds3 pxBounds2 = vehicles2[0]->getRigidDynamicActor()->getWorldBounds();
		PxTransform pos2 = vehicles2[0]->getRigidDynamicActor()->getGlobalPose();
		glm::vec3 cubePos2 = glm::vec3(pos2.p[0], pos2.p[1], pos2.p[2]);
		PxMat44 pxTransMatrix2 = PxMat44(pos2);
		return pxTransMatrix2;
	}
}

float VehiclePhysx::getAngleAroundY() {
	PxVehicleWheels* vehicles[1] = { gVehicle4W };
	PxQuat vehicleQuaternion = vehicles[0]->getRigidDynamicActor()->getGlobalPose().q;
	PxVec3 v_dir = vehicleQuaternion.getBasisVector2();
	glm::vec3 dir = glm::normalize(glm::vec3(v_dir.x, 0, v_dir.z));
	float angleAroundY = glm::degrees(atan2(dir.z, dir.x));
	return angleAroundY;
}

glm::vec3 VehiclePhysx::getVehiclePos(int index) {
	if (index == 1) {
		PxVehicleWheels* vehicles[1] = { gVehicle4W };
		PxBounds3 pxBounds = vehicles[0]->getRigidDynamicActor()->getWorldBounds();
		PxTransform pos = vehicles[0]->getRigidDynamicActor()->getGlobalPose();
		glm::vec3 cubePos = glm::vec3(pos.p[0], pos.p[1], pos.p[2]);
		return cubePos;
	}
	if (index == 2) {
		PxVehicleWheels* vehicles[1] = { gVehicle4W2 };
		PxBounds3 pxBounds = vehicles[0]->getRigidDynamicActor()->getWorldBounds();
		PxTransform pos = vehicles[0]->getRigidDynamicActor()->getGlobalPose();
		glm::vec3 cubePos = glm::vec3(pos.p[0], pos.p[1], pos.p[2]);
		return cubePos;
	}
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
	gVehicle4W->mDriveDynData.forceGearChange(input);
}







//MARK: AI
PxVehicleDrive4WRawInputData* VehiclePhysx::getVehDat() {
	return &gVehicleInputData2;
}

glm::vec3 VehiclePhysx::getOpponentPos() {
	PxVehicleWheels* vehicles[1] = { gVehicle4W2 };
	PxBounds3 pxBounds = vehicles[0]->getRigidDynamicActor()->getWorldBounds();
	PxTransform pos = vehicles[0]->getRigidDynamicActor()->getGlobalPose();
	glm::vec3 cubePos = glm::vec3(pos.p[0], pos.p[1], pos.p[2]);
	return cubePos;
}

glm::vec3 VehiclePhysx::getOpponentForVec() {
	PxVehicleWheels* vehicles[1] = { gVehicle4W2 };
	PxQuat vehicleQuaternion = vehicles[0]->getRigidDynamicActor()->getGlobalPose().q;
	PxVec3 vDir = vehicleQuaternion.getBasisVector2();
	return glm::vec3(vDir.x, vDir.y, vDir.z);
}

glm::vec3 VehiclePhysx::getPlayerForVec() {
	PxVehicleWheels* vehicles[1] = { gVehicle4W };
	PxQuat vehicleQuaternion = vehicles[0]->getRigidDynamicActor()->getGlobalPose().q;
	PxVec3 vDir = vehicleQuaternion.getBasisVector2();
	return glm::vec3(vDir.x, vDir.y, vDir.z);
}

PxVehicleDrive4W* VehiclePhysx::getOpponent4W() {
	return gVehicle4W2;
}



//MARK: Cooooking
void VehiclePhysx::cookGroundMeshes(std::vector<Mesh*> groundMeshes) {
	for (int i = 0; i < groundMeshes.size(); i++) {
		cookGroundMesh(groundMeshes[i]);
	}
}

void VehiclePhysx::cookGroundMesh(Mesh* meshToCook) {
	PxMaterial* gMaterial = gPhysics->createMaterial(0.8f, 0.8f, 0.1f); //create some material
	PxTriangleMeshDesc meshDesc; //mesh cooking from a triangle mesh

	std::vector<PxVec3> vertices = meshToCook->getActualVertices();
	std::vector<PxU32> indices = meshToCook->getVertexIndices();

	//std::cout << "size " << vertices.size() << "\n";

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
	meshBody = gPhysics->createRigidStatic(PxTransform(PxVec3(0, 0, 0))); //create a rigid body for the cooked mesh
	PxShape* meshShape = gPhysics->createShape(PxTriangleMeshGeometry(triMesh), *gMaterial); //create a shape from the cooked mesh

	PxFilterData qryFilterData;
	setupDrivableSurface(qryFilterData);
	meshShape->setQueryFilterData(qryFilterData);
	meshShape->setSimulationFilterData(groundPlaneSimFilterData);

	meshShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
	meshBody->attachShape(*meshShape); //attach the shape to the body
	gScene->addActor(*meshBody); //and add it to the scene
	triMesh->release(); //clean up
}

int VehiclePhysx::getGameStatus() {
	return GameStatus;
}

void VehiclePhysx::checkGameOver() {
	PxVehicleWheels* vehicles2[2] = { gVehicle4W,gVehicle4W2 };
	PxBounds3 pxBounds = vehicles2[0]->getRigidDynamicActor()->getWorldBounds();
	PxTransform pos = vehicles2[0]->getRigidDynamicActor()->getGlobalPose();
	glm::vec3 cubePos = glm::vec3(pos.p[0], pos.p[1], pos.p[2]);

	PxBounds3 pxBounds2 = vehicles2[1]->getRigidDynamicActor()->getWorldBounds();
	PxTransform pos2 = vehicles2[1]->getRigidDynamicActor()->getGlobalPose();
	glm::vec3 cubePos2 = glm::vec3(pos2.p[0], pos2.p[1], pos2.p[2]);

	if (cubePos.y < -10) {
		GameStatus = 1;
	}

	if (cubePos2.y < -10 && cubePos.y > 0) {
		GameStatus = 2;
	}
}

void VehiclePhysx::setGameStatus(int status) {
	GameStatus = status;
}

void VehiclePhysx::reset() {
	gScene->removeActor(*gVehicle4W->getRigidDynamicActor());
	VehicleDesc vehicleDesc = initVehicleDesc(1000);
	PxTransform startTransform(PxVec3(0, (vehicleDesc.chassisDims.y * 0.5f + vehicleDesc.wheelRadius + 1.0f), 0), PxQuat(PxIdentity));
	gVehicle4W->getRigidDynamicActor()->setGlobalPose(startTransform);
	gVehicle4W->getRigidDynamicActor()->setLinearVelocity(PxVec3(0, 0, 0));
	gScene->addActor(*gVehicle4W->getRigidDynamicActor());
	
	gScene->removeActor(*gVehicle4W2->getRigidDynamicActor());
	VehicleDesc vehicleDesc2 = initVehicleDesc(1000);
	PxTransform startTransform2(PxVec3(10.f, (vehicleDesc2.chassisDims.y * 0.5f + vehicleDesc2.wheelRadius + 1.0f), 10), PxQuat(PxIdentity));
	gVehicle4W2->getRigidDynamicActor()->setGlobalPose(startTransform2);
	gVehicle4W2->getRigidDynamicActor()->setLinearVelocity(PxVec3(0, 0, 0)); 
	gScene->addActor(*gVehicle4W2->getRigidDynamicActor());
}

void VehiclePhysx::applyForce(PxVec3 force, int index) {
	if (index == 1) {
		gVehicle4W->getRigidDynamicActor()->addForce(force);
	}
	if (index == 2) {
		gVehicle4W2->getRigidDynamicActor()->addForce(force);
	}
}

void VehiclePhysx::stopVehicle(int index) {
	if (index == 1) {
		gVehicle4W->getRigidDynamicActor()->setLinearVelocity(PxVec3(0.f, 0.f, 0.f));
	}
	if (index == 2) {
		gVehicle4W2->getRigidDynamicActor()->setLinearVelocity(PxVec3(0.f, 0.f, 0.f));
		gVehicleInputData2.setAnalogHandbrake(1.f);
	}
}