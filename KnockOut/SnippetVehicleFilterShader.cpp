//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright (c) 2008-2019 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include <new>
#include "./SnippetVehicleFilterShader.h"
#include "../include/physx/PxPhysicsAPI.h"

#include <iostream>

namespace snippetvehicle
{
#define GROUP_SIZE	32
using namespace physx;

static PxGroupsMask convert(const PxFilterData& fd)
{
	PxGroupsMask mask;

	mask.bits0 = PxU16((fd.word2 & 0xffff));
	mask.bits1 = PxU16((fd.word2 >> 16));
	mask.bits2 = PxU16((fd.word3 & 0xffff));
	mask.bits3 = PxU16((fd.word3 >> 16));

	return mask;
}

struct PxCollisionBitMap
{
	PX_INLINE PxCollisionBitMap() : enable(true) {}

	bool operator()() const { return enable; }
	bool& operator= (const bool& v) { enable = v; return enable; }

private:
	bool enable;
};

PxCollisionBitMap gCollisionTable[GROUP_SIZE][GROUP_SIZE];

static void gAND(PxGroupsMask& results, const PxGroupsMask& mask0, const PxGroupsMask& mask1)
{
	results.bits0 = PxU16(mask0.bits0 & mask1.bits0);
	results.bits1 = PxU16(mask0.bits1 & mask1.bits1);
	results.bits2 = PxU16(mask0.bits2 & mask1.bits2);
	results.bits3 = PxU16(mask0.bits3 & mask1.bits3);
}
static void gOR(PxGroupsMask& results, const PxGroupsMask& mask0, const PxGroupsMask& mask1)
{
	results.bits0 = PxU16(mask0.bits0 | mask1.bits0);
	results.bits1 = PxU16(mask0.bits1 | mask1.bits1);
	results.bits2 = PxU16(mask0.bits2 | mask1.bits2);
	results.bits3 = PxU16(mask0.bits3 | mask1.bits3);
}
static void gXOR(PxGroupsMask& results, const PxGroupsMask& mask0, const PxGroupsMask& mask1)
{
	results.bits0 = PxU16(mask0.bits0 ^ mask1.bits0);
	results.bits1 = PxU16(mask0.bits1 ^ mask1.bits1);
	results.bits2 = PxU16(mask0.bits2 ^ mask1.bits2);
	results.bits3 = PxU16(mask0.bits3 ^ mask1.bits3);
}
static void gNAND(PxGroupsMask& results, const PxGroupsMask& mask0, const PxGroupsMask& mask1)
{
	results.bits0 = PxU16(~(mask0.bits0 & mask1.bits0));
	results.bits1 = PxU16(~(mask0.bits1 & mask1.bits1));
	results.bits2 = PxU16(~(mask0.bits2 & mask1.bits2));
	results.bits3 = PxU16(~(mask0.bits3 & mask1.bits3));
}
static void gNOR(PxGroupsMask& results, const PxGroupsMask& mask0, const PxGroupsMask& mask1)
{
	results.bits0 = PxU16(~(mask0.bits0 | mask1.bits0));
	results.bits1 = PxU16(~(mask0.bits1 | mask1.bits1));
	results.bits2 = PxU16(~(mask0.bits2 | mask1.bits2));
	results.bits3 = PxU16(~(mask0.bits3 | mask1.bits3));
}
static void gNXOR(PxGroupsMask& results, const PxGroupsMask& mask0, const PxGroupsMask& mask1)
{
	results.bits0 = PxU16(~(mask0.bits0 ^ mask1.bits0));
	results.bits1 = PxU16(~(mask0.bits1 ^ mask1.bits1));
	results.bits2 = PxU16(~(mask0.bits2 ^ mask1.bits2));
	results.bits3 = PxU16(~(mask0.bits3 ^ mask1.bits3));
}

static void gSWAP_AND(PxGroupsMask& results, const PxGroupsMask& mask0, const PxGroupsMask& mask1)
{
	results.bits0 = PxU16(mask0.bits0 & mask1.bits2);
	results.bits1 = PxU16(mask0.bits1 & mask1.bits3);
	results.bits2 = PxU16(mask0.bits2 & mask1.bits0);
	results.bits3 = PxU16(mask0.bits3 & mask1.bits1);
}

typedef void	(*FilterFunction)	(PxGroupsMask& results, const PxGroupsMask& mask0, const PxGroupsMask& mask1);
FilterFunction const gTable[] = { gAND, gOR, gXOR, gNAND, gNOR, gNXOR, gSWAP_AND };

PxFilterOp::Enum gFilterOps[3] = { PxFilterOp::PX_FILTEROP_AND, PxFilterOp::PX_FILTEROP_AND, PxFilterOp::PX_FILTEROP_AND };

PxGroupsMask gFilterConstants[2];

bool gFilterBool = false;

PxFilterFlags VehicleFilterShader(
	PxFilterObjectAttributes attributes0,
	PxFilterData filterData0,
	PxFilterObjectAttributes attributes1,
	PxFilterData filterData1,
	PxPairFlags& pairFlags,
	const void* constantBlock,
	PxU32 constantBlockSize)
{
	
	//std::cout << filterData0.word0 << "\n";
	//std::cout << filterData0.word1 << "\n";
	//std::cout << filterData1.word0 << "\n";
	//std::cout << filterData1.word1 << "\n\n";

	PX_UNUSED(constantBlock);
	PX_UNUSED(constantBlockSize);
	PX_UNUSED(attributes0);
	PX_UNUSED(attributes1);

	
	// let triggers through
	if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
	{
		//std::cout << "trigger\n";
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
		return PxFilterFlags();
	}

	// Collision Group
	if (!gCollisionTable[filterData0.word0][filterData1.word0]())
	{
		//std::cout << "Collision\n";
		return PxFilterFlag::eSUPPRESS;
	}

	// Filter function
	PxGroupsMask g0 = convert(filterData0);
	PxGroupsMask g1 = convert(filterData1);

	PxGroupsMask g0k0;	gTable[gFilterOps[0]](g0k0, g0, gFilterConstants[0]);
	PxGroupsMask g1k1;	gTable[gFilterOps[1]](g1k1, g1, gFilterConstants[1]);
	PxGroupsMask final;	gTable[gFilterOps[2]](final, g0k0, g1k1);

	//box not car
	bool r = final.bits0 || final.bits1 || final.bits2 || final.bits3;
	if (r != gFilterBool)
	{
		//std::cout << "box\n";
		return PxFilterFlag::eSUPPRESS;
	}

	if ((0 == (filterData0.word0 & filterData1.word1)) && (0 == (filterData1.word0 & filterData0.word1))) {
		//std::cout << "test\n\n";
		if ((filterData1.word0 != 14) && (filterData1.word1 != 2)) {
			return PxFilterFlag::eSUPPRESS;
		}
	}

	pairFlags = PxPairFlag::eCONTACT_DEFAULT;
	pairFlags |= PxPairFlags(PxU16(filterData0.word2 | filterData1.word2));

	return PxFilterFlags();
	
}
	// all initial and persisting reports for 
  //everything, with per-point...

/*
PxFilterFlags VehicleFilterShader
(PxFilterObjectAttributes attributes0, PxFilterData filterData0, 
 PxFilterObjectAttributes attributes1, PxFilterData filterData1,
 PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	PX_UNUSED(attributes0);
	PX_UNUSED(attributes1);
	PX_UNUSED(constantBlock);
	PX_UNUSED(constantBlockSize);


	if( (0 == (filterData0.word0 & filterData1.word1)) && (0 == (filterData1.word0 & filterData0.word1)) )
		return PxFilterFlag::eSUPPRESS;

	pairFlags = PxPairFlag::eCONTACT_DEFAULT;
	pairFlags |= PxPairFlags(PxU16(filterData0.word2 | filterData1.word2));

	return PxFilterFlags();
}
*/

} // namespace snippetvehicle
