
#ifndef MYCONTACT_H
#define MYCONTACT_H

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
#include "../include/physx/PxSimulationEventCallback.h"

using namespace physx;

class MyContact : public PxSimulationEventCallback
{
public:
	void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs);

private:

};

#endif 

