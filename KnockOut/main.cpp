#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <..\KnockOut\Dependencies\imgui\imgui.h>
#include <..\KnockOut\Dependencies\GLFW\include\GLFW\glfw3.h>
#include <..\KnockOut\Dependencies\imgui\imgui_impl_glfw.h>
#include <..\KnockOut\Dependencies\imgui\imgui_impl_opengl3.h>
#include <iostream>
#include <ctype.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <../include/physx/PxPhysicsAPI.h>
#include <../include/physx/snippetcommon/SnippetPrint.h>
#include <../include/physx/snippetcommon/SnippetPVD.h>
#include <../include/physx/snippetutils/SnippetUtils.h>
#include <../include/physx/vehicle/PxVehicleUtil.h>
#include <../include/physx/snippetvehiclecommon/SnippetVehicleSceneQuery.h>
#include <../include/physx/snippetvehiclecommon/SnippetVehicleFilterShader.h>
#include <../include/physx/snippetvehiclecommon/SnippetVehicleTireFriction.h>
#include <../include/physx/snippetvehiclecommon/SnippetVehicleCreate.h>
#include <../include/physx/snippetcommon/SnippetPrint.h>
#include <../include/physx/snippetcommon/SnippetPVD.h>
#include <../include/physx/snippetutils/SnippetUtils.h>

#include "OpenALEngine.h"
#include "SoundManager.h"
#include "Shader.cpp"
#include "Mesh.h"
//#include "Utils.h"
#include "Renderer.h"

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
std::vector<PxRigidDynamic*> pxObjects;
std::vector<PxVehicleDrive4W*> pxOpponents;

bool					gIsVehicleInAir = true;

PxReal stackZ = 10.0f;






float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
float yaw = 90.0f;
float pitch = 0.0f;
float lastX = 400, lastY = 300;
float cameraDistance = 5.0f;
float angleAroundTarget = 0.0f;
bool firstMouse = true;
bool vehicleReversing = false;
bool vehicleAccelerating = false;
unsigned int CUBE_VBO, GROUND_VBO, CUBE_VAO, GROUND_VAO;
unsigned int vehicle_texture, cube_texture2, ground_texture;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 15.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::vec3();
glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 direction;


Renderer mainRenderer;
bool gMimicKeyInputs = false;
PxVehicleDrive4WRawInputData gVehicleInputData;
bool mouseVisible = false;





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

VehicleDesc initVehicleDesc()
{
	//Set up the chassis mass, dimensions, moment of inertia, and center of mass offset.
	//The moment of inertia is just the moment of inertia of a cuboid but modified for easier steering.
	//Center of mass offset is 0.65m above the base of the chassis and 0.25m towards the front.
	const PxF32 chassisMass = 1500.0f;
	const PxVec3 chassisDims(2.5f, 2.0f, 5.0f);
	const PxVec3 chassisMOI
	((chassisDims.y * chassisDims.y + chassisDims.z * chassisDims.z) * chassisMass / 12.0f,
		(chassisDims.x * chassisDims.x + chassisDims.z * chassisDims.z) * 0.8f * chassisMass / 12.0f,
		(chassisDims.x * chassisDims.x + chassisDims.y * chassisDims.y) * chassisMass / 12.0f);
	const PxVec3 chassisCMOffset(0.0f, -chassisDims.y * 0.5f + 0.65f, 0.25f);

	//Set up the wheel mass, radius, width, moment of inertia, and number of wheels.
	//Moment of inertia is just the moment of inertia of a cylinder.
	const PxF32 wheelMass = 20.0f;
	const PxF32 wheelRadius = 0.5f;
	const PxF32 wheelWidth = 0.4f;
	const PxF32 wheelMOI = 0.5f * wheelMass * wheelRadius * wheelRadius;
	const PxU32 nbWheels = 6;

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
	vehicleDesc.chassisSimFilterData = PxFilterData(COLLISION_FLAG_WHEEL, COLLISION_FLAG_WHEEL_AGAINST, 0, 0);

	return vehicleDesc;
}

void startAccelerateForwardsMode()
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

void startAccelerateReverseMode()
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

void startBrakeMode()
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

void startTurnHardLeftMode()
{
	if (gMimicKeyInputs)
	{
		gVehicleInputData.setDigitalAccel(true);
		gVehicleInputData.setDigitalSteerLeft(true);
	}
	else
	{
		gVehicleInputData.setAnalogAccel(true);
		gVehicleInputData.setAnalogSteer(-1.0f);
	}
}

void startTurnHardRightMode()
{
	if (gMimicKeyInputs)
	{
		gVehicleInputData.setDigitalAccel(true);
		gVehicleInputData.setDigitalSteerRight(true);
	}
	else
	{
		gVehicleInputData.setAnalogAccel(1.0f);
		gVehicleInputData.setAnalogSteer(1.0f);
	}
}

void startHandbrakeTurnLeftMode()
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

void startHandbrakeTurnRightMode()
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


void releaseAllControls()
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




PxRigidDynamic* box1;

void initPhysics()
{
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

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
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

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

	//Create a plane to drive on.
	PxFilterData groundPlaneSimFilterData(COLLISION_FLAG_GROUND, COLLISION_FLAG_GROUND_AGAINST, 0, 0);
	gGroundPlane = createDrivablePlane(groundPlaneSimFilterData, gMaterial, gPhysics);
	gScene->addActor(*gGroundPlane);

	//Create a vehicle that will drive on the plane.
	VehicleDesc vehicleDesc = initVehicleDesc();
	gVehicle4W = createVehicle4W(vehicleDesc, gPhysics, gCooking);
	PxTransform startTransform(PxVec3(0, (vehicleDesc.chassisDims.y * 0.5f + vehicleDesc.wheelRadius + 1.0f), 0), PxQuat(PxIdentity));
	gVehicle4W->getRigidDynamicActor()->setGlobalPose(startTransform);
	gScene->addActor(*gVehicle4W->getRigidDynamicActor());

	VehicleDesc vehicleDesc2 = initVehicleDesc();
	PxVehicleDrive4W* tempVeh = createVehicle4W(vehicleDesc2, gPhysics, gCooking);
	PxTransform startTransform2(PxVec3(5.f, (vehicleDesc2.chassisDims.y * 0.5f + vehicleDesc2.wheelRadius + 1.0f), 25.f), PxQuat(PxIdentity));
	tempVeh->getRigidDynamicActor()->setGlobalPose(startTransform2);
	gScene->addActor(*tempVeh->getRigidDynamicActor());
	pxOpponents.push_back(tempVeh);

	//Create a box for the vehicle to collide with
	PxTransform localTm(PxVec3(0, 20.0f, 0.0f));
	box1 = gPhysics->createRigidDynamic(localTm);
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(0.5f, 0.5f, 0.5f), *gMaterial);
	PxFilterData myData = PxFilterData();
	myData.word0 = 14;
	myData.word1 = 2;
	shape->setSimulationFilterData(myData);
	box1->attachShape(*shape);
	gScene->addActor(*box1);

	pxObjects.push_back(box1);

	PxTransform localTm2(PxVec3(0.f, 3.0f, 10.0f));
	PxRigidDynamic* box2 = gPhysics->createRigidDynamic(localTm2);
	PxShape* shape2 = gPhysics->createShape(PxBoxGeometry(0.5f, 0.5f, 0.5f), *gMaterial);
	PxFilterData myData2 = PxFilterData();
	myData2.word0 = 14;
	myData2.word1 = 2;
	shape2->setSimulationFilterData(myData2);
	box2->attachShape(*shape2);
	gScene->addActor(*box2);

	pxObjects.push_back(box2);

	PxTransform localTm3(PxVec3(0, 6.0f, 10.0f));
	PxRigidDynamic* box3 = gPhysics->createRigidDynamic(localTm3);
	PxShape* shape3 = gPhysics->createShape(PxBoxGeometry(0.5f, 0.5f, 0.5f), *gMaterial);
	PxFilterData myData3 = PxFilterData();
	myData3.word0 = 14;
	myData3.word1 = 2;
	shape3->setSimulationFilterData(myData3);
	box3->attachShape(*shape3);
	gScene->addActor(*box3);

	pxObjects.push_back(box3);

	//Set the vehicle to rest in first gear.
	//Set the vehicle to use auto-gears.
	gVehicle4W->setToRestState();
	gVehicle4W->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
	gVehicle4W->mDriveDynData.setUseAutoGears(true);

	gVehicleModeTimer = 0.0f;
	gVehicleOrderProgress = 0;
	startBrakeMode();
}

void incrementDrivingMode(const PxF32 timestep)
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

void stepPhysics()
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

	//FALL OFF
	PxVehicleWheels* vehicles2[2] = { gVehicle4W, pxOpponents[0] };
	PxBounds3 pxBounds = vehicles2[0]->getRigidDynamicActor()->getWorldBounds();
	PxTransform pos = vehicles2[0]->getRigidDynamicActor()->getGlobalPose();
	glm::vec3 cubePos = glm::vec3(pos.p[0], pos.p[1], pos.p[2]);

	PxBounds3 pxBounds2 = vehicles2[1]->getRigidDynamicActor()->getWorldBounds();
	PxTransform pos2 = vehicles2[1]->getRigidDynamicActor()->getGlobalPose();
	glm::vec3 cubePos2 = glm::vec3(pos2.p[0], pos2.p[1], pos2.p[2]);

	if (pos.p[2] >= 300 || pos.p[2] <= -300 || pos.p[0] >= 300 || pos.p[0] <= -300) {
		PxQuat vehicleQuaternion = gVehicle4W->getRigidDynamicActor()->getGlobalPose().q;
		gScene->removeActor(*gVehicle4W->getRigidDynamicActor());
		VehicleDesc vehicleDesc = initVehicleDesc();
		PxTransform startTransform(PxVec3(0, (vehicleDesc.chassisDims.y * 0.5f + vehicleDesc.wheelRadius + 1.0f), 0), PxQuat(vehicleQuaternion));
		gVehicle4W->getRigidDynamicActor()->setGlobalPose(startTransform);
		gScene->addActor(*gVehicle4W->getRigidDynamicActor());
	}
	if (pos2.p[2] >= 300 || pos2.p[2] <= -300 || pos2.p[0] >= 300 || pos2.p[0] <= -300) {
		PxQuat vehicleQuaternion = pxOpponents[0]->getRigidDynamicActor()->getGlobalPose().q;
		gScene->removeActor(*pxOpponents[0]->getRigidDynamicActor());
		VehicleDesc vehicleDesc = initVehicleDesc();
		PxTransform startTransform(PxVec3(15.f, (vehicleDesc.chassisDims.y * 0.5f + vehicleDesc.wheelRadius + 1.0f), 0), PxQuat(vehicleQuaternion));
		pxOpponents[0]->getRigidDynamicActor()->setGlobalPose(startTransform);
		gScene->addActor(*pxOpponents[0]->getRigidDynamicActor());
	}



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
	PxVehicleWheels* vehicles3[1] = { pxOpponents[0] };
	PxRaycastQueryResult* raycastResults2 = gVehicleSceneQueryData->getRaycastQueryResultBuffer(0);
	const PxU32 raycastResultsSize2 = gVehicleSceneQueryData->getQueryResultBufferSize();
	PxVehicleSuspensionRaycasts(gBatchQuery, 1, vehicles3, raycastResultsSize2, raycastResults2);


	//Vehicle update.
	//const PxVec3 grav = gScene->getGravity();
	PxWheelQueryResult wheelQueryResults2[PX_MAX_NB_WHEELS];
	PxVehicleWheelQueryResult vehicleQueryResults2[1] = { {wheelQueryResults2, pxOpponents[0]->mWheelsSimData.getNbWheels()} };

	PxVehicleUpdates(timestep, grav, *gFrictionPairs, 1, vehicles3, vehicleQueryResults2);
	//Work out if the vehicle is in the air.
	gIsVehicleInAir = pxOpponents[0]->getRigidDynamicActor()->isSleeping() ? false : PxVehicleIsInAir(vehicleQueryResults2[0]);



	//Scene update.
	gScene->simulate(timestep);
	gScene->fetchResults(true);

	//gVehicle4W->setBaseFlag(PxRigi)
}

void cleanupPhysics()
{
	//gBox->free();

	gVehicle4W->getRigidDynamicActor()->release();
	gVehicle4W->free();
	pxOpponents[0]->getRigidDynamicActor()->release();
	pxOpponents[0]->free();
	for(int i = 0; i<pxObjects.size(); i++) PX_RELEASE(pxObjects[i]);
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

void keyPress(unsigned char key, const PxTransform& camera)
{
	PX_UNUSED(camera);
	PX_UNUSED(key);
}
















void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
		if (mouseVisible) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			mouseVisible = false;
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			mouseVisible = true;
		}
	}

	releaseAllControls();

	float cameraSpeed = 10 * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

	if ((glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_UP) == GLFW_REPEAT)) {
		gMimicKeyInputs = true;
		gVehicle4W->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
		startAccelerateForwardsMode();
	}
	if ((glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_REPEAT)) {
		gMimicKeyInputs = true;
		gVehicle4W->mDriveDynData.forceGearChange(PxVehicleGearsData::eREVERSE);
		startAccelerateReverseMode();
	}
	if ((glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_REPEAT)) {
		gMimicKeyInputs = true;
		startBrakeMode();
	}
	if ((glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_REPEAT)) {
		gMimicKeyInputs = true;
		startTurnHardRightMode();
	}
	if ((glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_REPEAT)) {
		gMimicKeyInputs = true;
		startTurnHardLeftMode();
	}
}




void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	mouseCallback(window, xpos, ypos);
}











//MARK: Main
int main(int argc, char** argv) {

	//MARK: INIT Sounds
	OpenALEngine wavPlayer = OpenALEngine();
	SoundManager bgm = wavPlayer.createSoundPlayer(0);
	bgm.setVolume(0.3f);
	bgm.loopSound(true);

	//MARK: INIT GLFW
	const char* glsl_version = "#version 130";
	GLFWwindow* window;
	if (!glfwInit()) return -1;
	window = glfwCreateWindow(800, 800, "Knock Out", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK) std::cout << "Error!" << std::endl;
	std::cout << glGetString(GL_VERSION) << std::endl;
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	Shader ourShader("vertex_shader.vs", "fragment_shader.fs");
	mainRenderer.setUpRendering(cameraPos, ourShader);


	//MARK: INIT IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);



	initPhysics();



	//MARK: RENDER LOOP ---------------------------------------------------------------------------------------------------------------
	while (!glfwWindowShouldClose(window)) {

		if (!bgm.soundPlaying()) {
			bgm.playSound();
		}

		//MARK: Frame Start
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);
		stepPhysics();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		PxVehicleWheels* vehicles[1] = { gVehicle4W };
		PxBounds3 pxBounds = vehicles[0]->getRigidDynamicActor()->getWorldBounds();
		PxTransform pos = vehicles[0]->getRigidDynamicActor()->getGlobalPose();
		glm::vec3 cubePos = glm::vec3(pos.p[0], pos.p[1], pos.p[2]);
		PxMat44 pxTransMatrix = PxMat44(pos);


		PxVehicleWheels* vehicles2[1] = { pxOpponents[0] };
		PxBounds3 pxBounds2 = vehicles2[0]->getRigidDynamicActor()->getWorldBounds();
		PxTransform pos2 = vehicles2[0]->getRigidDynamicActor()->getGlobalPose();
		glm::vec3 cubePos2 = glm::vec3(pos2.p[0], pos2.p[1], pos2.p[2]);
		PxMat44 pxTransMatrix2 = PxMat44(pos2);

		//camera, TODO: move somewhere else
		PxQuat vehicleQuaternion = vehicles[0]->getRigidDynamicActor()->getGlobalPose().q;
		PxVec3 v_dir = vehicleQuaternion.getBasisVector2();
		glm::vec3 dir = glm::normalize(glm::vec3(v_dir.x, 0, v_dir.z));
		float angleAroundY = glm::degrees(atan2(dir.z, dir.x));

		//angleAroundTarget += (xChange * turnSpeed); //calculateAngleAroundTarget
		//if (angleAroundTarget < -40.f) angleAroundTarget = -40.f;
		//if (angleAroundTarget > 40.f) angleAroundTarget = 40.f;

		cameraPos.y = cubePos.y + 2.f; //calculatePos
		float xOffset = cameraDistance * cos(glm::radians(angleAroundY + angleAroundTarget));
		float zOffset = cameraDistance * sin(glm::radians(angleAroundY + angleAroundTarget));
		cameraPos.x = cubePos.x - xOffset;
		cameraPos.z = cubePos.z - zOffset;

		yaw = angleAroundY + angleAroundTarget;

		cameraFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch)); //update
		cameraFront.y = sin(glm::radians(pitch));
		cameraFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(cameraFront);
		cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
		cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));

		glm::mat4 view = glm::mat4(1.0f); //transformations - first initialize the identity matrix
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp); //apply a special built in matrix specifically made for camera views called the "Look At" matrix
		//ourShader.setMat4("view", view); //set the camera view matrix in our fragment shader



		//MARK: Render Scene
		glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//---------------------------------------------------------------
		mainRenderer.renderGameFrame(gVehicle4W, pxOpponents, box1, gGroundPlane, ourShader, view);
		//---------------------------------------------------------------


		//MARK: Render ImgUI
		{
			ImGui::Begin("Debug Menu");
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//MARK: Frame End
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	//---------------------------------------------------------------------------------------------------------------------------------

	//MARK: Clean up & Terminate
	cleanupPhysics();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 0;
}
