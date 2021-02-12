#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "..\KnockOut\Dependencies\imgui\imgui.h"
#include "..\KnockOut\Dependencies\GLFW\include\GLFW\glfw3.h"
#include "..\KnockOut\Dependencies\imgui\imgui_impl_glfw.h"
#include "..\KnockOut\Dependencies\imgui\imgui_impl_opengl3.h"
#include <iostream>
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
#include "OpenALEngine.h"
#include "SoundManager.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Shader.cpp"

using namespace physx;
using namespace snippetvehicle;

//MARK: Function Prototypes
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

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
PxRigidDynamic* gBox = NULL;
PxRigidDynamic* gBox2 = NULL;
PxRigidDynamic* gBox3 = NULL;

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
bool mouseVisible = false;
unsigned int CUBE_VBO, GROUND_VBO, CUBE_VAO, GROUND_VAO;
unsigned int vehicle_texture, cube_texture2, ground_texture;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 15.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::vec3();
glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 direction;

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
	gVehicle4W2 = createVehicle4W(vehicleDesc2, gPhysics, gCooking);
	PxTransform startTransform2(PxVec3(15.f, (vehicleDesc2.chassisDims.y * 0.5f + vehicleDesc2.wheelRadius + 1.0f), 0), PxQuat(PxIdentity));
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
	gScene->addActor(*gBox);

	PxTransform localTm2(PxVec3(0.f, 3.0f, 10.0f));
	gBox2 = gPhysics->createRigidDynamic(localTm2);
	PxShape* shape2 = gPhysics->createShape(PxBoxGeometry(0.5f, 0.5f, 0.5f), *gMaterial);
	PxFilterData myData2 = PxFilterData();
	myData2.word0 = 14;
	myData2.word1 = 2;
	shape2->setSimulationFilterData(myData2);
	gBox2->attachShape(*shape2);
	gScene->addActor(*gBox2);

	PxTransform localTm3(PxVec3(0, 6.0f, 10.0f));
	gBox3 = gPhysics->createRigidDynamic(localTm3);
	PxShape* shape3 = gPhysics->createShape(PxBoxGeometry(0.5f, 0.5f, 0.5f), *gMaterial);
	PxFilterData myData3 = PxFilterData();
	myData3.word0 = 14;
	myData3.word1 = 2;
	shape3->setSimulationFilterData(myData3);
	gBox3->attachShape(*shape3);
	gScene->addActor(*gBox3);

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
	PxVehicleWheels* vehicles2[2] = { gVehicle4W,gVehicle4W2 };
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
		PxQuat vehicleQuaternion = gVehicle4W2->getRigidDynamicActor()->getGlobalPose().q;
		gScene->removeActor(*gVehicle4W2->getRigidDynamicActor());
		VehicleDesc vehicleDesc = initVehicleDesc();
		PxTransform startTransform(PxVec3(15.f, (vehicleDesc.chassisDims.y * 0.5f + vehicleDesc.wheelRadius + 1.0f), 0), PxQuat(vehicleQuaternion));
		gVehicle4W2->getRigidDynamicActor()->setGlobalPose(startTransform);
		gScene->addActor(*gVehicle4W2->getRigidDynamicActor());
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

	//gVehicle4W->setBaseFlag(PxRigi)
}

void cleanupPhysics()
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

void keyPress(unsigned char key, const PxTransform& camera)
{
	PX_UNUSED(camera);
	PX_UNUSED(key);
}


void prepCubeRendering(Shader* ourShader) {
	glEnable(GL_DEPTH_TEST); //to make sure the fragment shader takes into account that some geometry has to be drawn in front of another
	float vertices[] = {
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

	glGenVertexArrays(1, &CUBE_VAO);
	glGenBuffers(1, &CUBE_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, CUBE_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(CUBE_VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//TEXTURES
	glGenTextures(1, &vehicle_texture); //TEXTURE 1
	glBindTexture(GL_TEXTURE_2D, vehicle_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //set the texture wrapping parameters (= how to behave when the texture not big enough to cover the whole area)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //set texture filtering parameters (= how to decide which texel (texture pixel) to show on the current
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //screen pixel, i.e. do we just take what's directly underneath?, or add up the neighboring colors?, etc.)
	int width, height, nrChannels; //load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); //tell stb_image.h to flip loaded texture's on the y-axis
	unsigned char* data = stbi_load("container_texture.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else std::cout << "Failed to load texture" << std::endl;
	stbi_image_free(data); //free image mem
}


void prepGroundRendering(Shader* ourShader) {
	glEnable(GL_DEPTH_TEST); //to make sure the fragment shader takes into account that some geometry has to be drawn in front of another
	float vertices[] = { //vertices of our plane
		 //positions              //normals           //texture coords
		-300.0f,  0.0f,  300.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
		 300.0f,  0.0f,  300.0f,  0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
		 300.0f,  0.0f, -300.0f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f,

		-300.0f,  0.0f, -300.0f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
		-300.0f,  0.0f,  300.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
		 300.0f,  0.0f, -300.0f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
	};
	glGenVertexArrays(1, &GROUND_VAO);
	glGenBuffers(1, &GROUND_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, GROUND_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(GROUND_VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//TEXTURE
	glGenTextures(1, &ground_texture);
	glBindTexture(GL_TEXTURE_2D, ground_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //set the texture wrapping parameters (= how to behave when the texture not big enough to cover the whole area)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //set texture filtering parameters (= how to decide which texel (texture pixel) to show on the current
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //screen pixel, i.e. do we just take what's directly underneath?, or add up the neighboring colors?, etc.)
	int width, height, nrChannels; //load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); //tell stb_image.h to flip loaded texture's on the y-axis
	unsigned char* data = stbi_load("grass.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else std::cout << "Failed to load texture" << std::endl;
	stbi_image_free(data); //free image mem
}

glm::mat4 getGlmMatrix(PxMat44 tempMat) { //convert a 4x4 px matrix to a 4x4 glm matrix
	glm::mat4 model = glm::mat4(1.0f);
	model[0].x = tempMat.column0[0];
	model[0].y = tempMat.column0[1];
	model[0].z = tempMat.column0[2];
	model[0].w = tempMat.column0[3];

	model[1].x = tempMat.column1[0];
	model[1].y = tempMat.column1[1];
	model[1].z = tempMat.column1[2];
	model[1].w = tempMat.column1[3];

	model[2].x = tempMat.column2[0];
	model[2].y = tempMat.column2[1];
	model[2].z = tempMat.column2[2];
	model[2].w = tempMat.column2[3];

	model[3].x = tempMat.column3[0];
	model[3].y = tempMat.column3[1];
	model[3].z = tempMat.column3[2];
	model[3].w = tempMat.column3[3];
	return model;
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

	//MARK: SCENE RENDER PREP
	prepCubeRendering(&ourShader);
	prepGroundRendering(&ourShader);
	ourShader.use(); //tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	ourShader.setInt("material.diffuse", 0);
	ourShader.setInt("material.specular", 1);

	initPhysics();

	//MARK: CAMERA SETUP
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)800 / (float)800, 0.1f, 500.0f); //how to show perspective (fov, aspect ratio)
	ourShader.setMat4("projection", projection); //pass the projection matrix to the fragment shader

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


		PxVehicleWheels* vehicles2[1] = { gVehicle4W2 };
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

		PxTransform pxGroundPos = gGroundPlane->getGlobalPose();
		glm::vec3 groundPos = glm::vec3(pxGroundPos.p[0], pxGroundPos.p[1], pxGroundPos.p[2]);
		PxTransform pxBoxPos = gBox->getGlobalPose();
		glm::vec3 boxPos = glm::vec3(pxBoxPos.p[0], pxBoxPos.p[1], pxBoxPos.p[2]);

		PxTransform pxBoxPos2 = gBox2->getGlobalPose();
		glm::vec3 boxPos2 = glm::vec3(pxBoxPos2.p[0], pxBoxPos2.p[1], pxBoxPos2.p[2]);

		PxTransform pxBoxPos3 = gBox3->getGlobalPose();
		glm::vec3 boxPos3 = glm::vec3(pxBoxPos3.p[0], pxBoxPos3.p[1], pxBoxPos3.p[2]);

		glm::mat4 view = glm::mat4(1.0f); //transformations - first initialize the identity matrix
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp); //apply a special built in matrix specifically made for camera views call the "Look At" matrix
		//ourShader.setMat4("view", view); //set the camera view matrix in our fragment shader

		//MARK: Render Scene
		glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//DIRECTIONAL LIGHTING (sunglight)
		ourShader.use();
		ourShader.setVec3("light.direction", -0.2f, -1.0f, -0.3f);
		ourShader.setVec3("viewPos", cameraPos);
		ourShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
		ourShader.setVec3("light.diffuse", 0.6f, 0.6f, 0.6f);
		ourShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		ourShader.setFloat("material.shininess", 256.0f);

		//VEHICLE
		glActiveTexture(GL_TEXTURE0); //bind textures on corresponding texture units
		glBindTexture(GL_TEXTURE_2D, vehicle_texture);
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, vehicle_texture);
		glBindVertexArray(CUBE_VAO); //tell OpenGL to render whatever we have in our Vertex Array Object
		glm::mat4 model = glm::mat4(1.0f); //identity matrix
		model = getGlmMatrix(pxTransMatrix);
		//model = glm::translate(model, cubePos); //model matrix converts the local coordinates (cubePosition) to the global world coordinates
		//model = glm::rotate(model, glm::radians(0), glm::vec3(0.0f, 1.0f, 0.0f)); //rotate
		//model = glm::scale(model, glm::vec3(pxBounds.getDimensions().x, pxBounds.getDimensions().y, pxBounds.getDimensions().z));
		ourShader.setMat4("model", model); //set the model matrix (which when applied converts the local position to global world coordinates...)
		model[3][1] = model[3][1] - 3.0f;
		ourShader.setMat4("view", view); //set the camera view matrix in our fragment shader		
		glDrawArrays(GL_TRIANGLES, 0, 36); //draw the triangle data, starting at 0 with 36 vertex data points

		//VEHICLE2
		glActiveTexture(GL_TEXTURE0); //bind textures on corresponding texture units
		glBindTexture(GL_TEXTURE_2D, vehicle_texture);
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, vehicle_texture);
		glBindVertexArray(CUBE_VAO); //tell OpenGL to render whatever we have in our Vertex Array Object
		glm::mat4 model2 = glm::mat4(1.0f); //identity matrix
		model2 = getGlmMatrix(pxTransMatrix2);
		//model = glm::translate(model, cubePos); //model matrix converts the local coordinates (cubePosition) to the global world coordinates
		//model = glm::rotate(model, glm::radians(0), glm::vec3(0.0f, 1.0f, 0.0f)); //rotate
		//model = glm::scale(model, glm::vec3(pxBounds.getDimensions().x, pxBounds.getDimensions().y, pxBounds.getDimensions().z));
		ourShader.setMat4("model", model2); //set the model matrix (which when applied converts the local position to global world coordinates...)
		model2[3][1] = model2[3][1] - 3.0f;
		ourShader.setMat4("view", view); //set the camera view matrix in our fragment shader		
		glDrawArrays(GL_TRIANGLES, 0, 36); //draw the triangle data, starting at 0 with 36 vertex data points


		//GROUND
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ground_texture);
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, ground_texture);
		glBindVertexArray(GROUND_VAO);
		model = glm::mat4(1.0f);
		model = glm::translate(model, groundPos);
		//model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.3f, 0.5f));
		ourShader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//BOX
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, vehicle_texture);
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, vehicle_texture);
		glBindVertexArray(CUBE_VAO); //tell OpenGL to render whatever we have in our Vertex Array Object
		model = glm::mat4(1.0f); //identity matrix
		model = getGlmMatrix(PxMat44(pxBoxPos));
		ourShader.setMat4("model", model); //set the model matrix (which when applied converts the local position to global world coordinates...)
		glDrawArrays(GL_TRIANGLES, 0, 36); //draw the triangle data, starting at 0 with 36 vertex data points

		glActiveTexture(GL_TEXTURE0); //bind textures on corresponding texture units
		glBindTexture(GL_TEXTURE_2D, vehicle_texture);
		glBindVertexArray(CUBE_VAO); //tell OpenGL to render whatever we have in our Vertex Array Object
		model = glm::mat4(1.0f); //identity matrix
		model = getGlmMatrix(PxMat44(pxBoxPos2));
		ourShader.setMat4("model", model); //set the model matrix (which when applied converts the local position to global world coordinates...)
		glDrawArrays(GL_TRIANGLES, 0, 36); //draw the triangle data, starting at 0 with 36 vertex data points

		glActiveTexture(GL_TEXTURE0); //bind textures on corresponding texture units
		glBindTexture(GL_TEXTURE_2D, vehicle_texture);
		glBindVertexArray(CUBE_VAO); //tell OpenGL to render whatever we have in our Vertex Array Object
		model = glm::mat4(1.0f); //identity matrix
		model = getGlmMatrix(PxMat44(pxBoxPos3));
		ourShader.setMat4("model", model); //set the model matrix (which when applied converts the local position to global world coordinates...)
		glDrawArrays(GL_TRIANGLES, 0, 36); //draw the triangle data, starting at 0 with 36 vertex data points

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


//MARK: Input Functions
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
		//std::cout << "UP1\n";
		gMimicKeyInputs = true;
		startAccelerateForwardsMode();
	}
	if ((glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_REPEAT)) {
		//std::cout << "DOWN1\n";
		gMimicKeyInputs = true;
		startBrakeMode();
	}
	if ((glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_REPEAT)) {
		//std::cout << "LEFT1\n";
		gMimicKeyInputs = true;
		startTurnHardRightMode();
	}
	if ((glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_REPEAT)) {
		//std::cout << "RIGHT1\n";
		gMimicKeyInputs = true;
		startTurnHardLeftMode();
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
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