/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "PsShare.h"
#include "NxApex.h"
#include "ApexLegacyModule.h"
#include "ApexRWLockable.h"

// AUTO_GENERATED_INCLUDES_BEGIN
#include "ConvexHullParameters_0p0.h"
#include "ConvexHullParameters_0p1.h"
#include "ConversionConvexHullParameters_0p0_0p1.h"
// AUTO_GENERATED_INCLUDES_END

namespace physx
{
namespace apex
{
namespace legacy
{

// AUTO_GENERATED_OBJECTS_BEGIN
static ConvexHullParameters_0p0Factory factory_ConvexHullParameters_0p0;
static ConvexHullParameters_0p1Factory factory_ConvexHullParameters_0p1;
// AUTO_GENERATED_OBJECTS_END

static LegacyClassEntry ModuleCommonLegacyObjects[] = {
	// AUTO_GENERATED_TABLE_BEGIN
	{
		0,
		1,
		&factory_ConvexHullParameters_0p0,
		ConvexHullParameters_0p0::freeParameterDefinitionTable,
		ConversionConvexHullParameters_0p0_0p1::Create,
		0
	},
	// AUTO_GENERATED_TABLE_END

	{ 0, 0, 0, 0, 0, 0} // Terminator
};

class ModuleCommonLegacy : public ApexLegacyModule, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ModuleCommonLegacy( NiApexSDK* sdk );

protected:
	void releaseLegacyObjects();

private:

	// Add custom conversions here

};

DEFINE_INSTANTIATE_MODULE(ModuleCommonLegacy)

ModuleCommonLegacy::ModuleCommonLegacy( NiApexSDK* inSdk )
{
	name = "Common_Legacy";
	mSdk = inSdk;
	mApiProxy = this;

	// Register legacy stuff

	NxParameterized::Traits *t = mSdk->getParameterizedTraits();
	if( !t )
		return;

	// Register auto-generated objects
	registerLegacyObjects(ModuleCommonLegacyObjects);

	// Register custom conversions here
}

void ModuleCommonLegacy::releaseLegacyObjects()
{
	//Release legacy stuff

	NxParameterized::Traits *t = mSdk->getParameterizedTraits();
	if( !t )
		return;

	// Unregister auto-generated objects
	unregisterLegacyObjects(ModuleCommonLegacyObjects);

	// Unregister custom conversions here
}

}
}
} // end namespace physx::apex
