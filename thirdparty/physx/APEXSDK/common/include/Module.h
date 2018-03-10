/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_H
#define MODULE_H

#include "ApexInterface.h"
#include "ApexString.h"
#include "NiApexSDK.h"
#include "PsArray.h"

struct NxApexParameter;

namespace physx
{
namespace apex
{

class Module : public physx::UserAllocated
{
public:
	Module();
	void release();

	physx::PxU32 getNbParameters() const;
	NxApexParameter** getParameters();
	void setIntValue(physx::PxU32 parameterIndex, physx::PxU32 value);
	physx::PxF32 getCurrentValue(NxRange<physx::PxU32> range, physx::PxU32 staticIndex, NxInterpolator* = NULL) const;
	physx::PxF32 getCurrentValue(NxRange<physx::PxF32> range, physx::PxU32 staticIndex, NxInterpolator* = NULL) const;

	physx::PxF32 getLODUnitCost() const;
	void setLODUnitCost(physx::PxF32 cost);

	physx::PxF32 getLODBenefitValue() const;
	void setLODBenefitValue(physx::PxF32 value);

	bool getLODEnabled() const;
	void setLODEnabled(bool enabled);

	const char* getName() const;

	/* Framework internal NiModule class methods */
	void destroy();

	NiApexSDK* mSdk;


protected:
	ApexSimpleString name;
	NxModule* mApiProxy;

	physx::Array<NxApexParameter*> mParameters;
	physx::Array<char*> allNames;

	bool mLodEnabled;
	physx::PxF32 mLodUnitCost;
	physx::PxF32 mLodBenefitValue;

	void registerLODParameter(const char* name, NxRange<physx::PxU32> range);
};

}
} // end namespace physx::apex

#endif // MODULE_H
