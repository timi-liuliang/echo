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
#include "ApexEmitterAssetParameters_0p0.h"
#include "ApexEmitterAssetParameters_0p1.h"
#include "ConversionApexEmitterAssetParameters_0p0_0p1.h"
#include "ApexEmitterAssetParameters_0p2.h"
#include "ConversionApexEmitterAssetParameters_0p1_0p2.h"
#include "EmitterGeomExplicitParams_0p0.h"
#include "EmitterGeomExplicitParams_0p1.h"
#include "ConversionEmitterGeomExplicitParams_0p0_0p1.h"
#include "EmitterGeomSphereParams_0p0.h"
#include "EmitterGeomSphereParams_0p1.h"
#include "ConversionEmitterGeomSphereParams_0p0_0p1.h"
#include "EmitterGeomSphereShellParams_0p0.h"
#include "EmitterGeomSphereShellParams_0p1.h"
#include "ConversionEmitterGeomSphereShellParams_0p0_0p1.h"
#include "ApexEmitterAssetParameters_0p3.h"
#include "ConversionApexEmitterAssetParameters_0p2_0p3.h"
#include "ApexEmitterAssetParameters_0p4.h"
#include "ConversionApexEmitterAssetParameters_0p3_0p4.h"
#include "ApexEmitterAssetParameters_0p5.h"
#include "ConversionApexEmitterAssetParameters_0p4_0p5.h"
#include "ApexEmitterAssetParameters_0p6.h"
#include "ConversionApexEmitterAssetParameters_0p5_0p6.h"
// AUTO_GENERATED_INCLUDES_END

namespace physx
{
namespace apex
{
namespace legacy
{

// AUTO_GENERATED_OBJECTS_BEGIN
static ApexEmitterAssetParameters_0p0Factory factory_ApexEmitterAssetParameters_0p0;
static ApexEmitterAssetParameters_0p1Factory factory_ApexEmitterAssetParameters_0p1;
static ApexEmitterAssetParameters_0p2Factory factory_ApexEmitterAssetParameters_0p2;
static EmitterGeomExplicitParams_0p0Factory factory_EmitterGeomExplicitParams_0p0;
static EmitterGeomExplicitParams_0p1Factory factory_EmitterGeomExplicitParams_0p1;
static EmitterGeomSphereParams_0p0Factory factory_EmitterGeomSphereParams_0p0;
static EmitterGeomSphereParams_0p1Factory factory_EmitterGeomSphereParams_0p1;
static EmitterGeomSphereShellParams_0p0Factory factory_EmitterGeomSphereShellParams_0p0;
static EmitterGeomSphereShellParams_0p1Factory factory_EmitterGeomSphereShellParams_0p1;
static ApexEmitterAssetParameters_0p3Factory factory_ApexEmitterAssetParameters_0p3;
static ApexEmitterAssetParameters_0p4Factory factory_ApexEmitterAssetParameters_0p4;
static ApexEmitterAssetParameters_0p5Factory factory_ApexEmitterAssetParameters_0p5;
static ApexEmitterAssetParameters_0p6Factory factory_ApexEmitterAssetParameters_0p6;
// AUTO_GENERATED_OBJECTS_END

static LegacyClassEntry ModuleEmitterLegacyObjects[] =
{
	// AUTO_GENERATED_TABLE_BEGIN
	{
		0,
		1,
		&factory_ApexEmitterAssetParameters_0p0,
		ApexEmitterAssetParameters_0p0::freeParameterDefinitionTable,
		ConversionApexEmitterAssetParameters_0p0_0p1::Create,
		0
	},
	{
		1,
		2,
		&factory_ApexEmitterAssetParameters_0p1,
		ApexEmitterAssetParameters_0p1::freeParameterDefinitionTable,
		ConversionApexEmitterAssetParameters_0p1_0p2::Create,
		0
	},
	{
		0,
		1,
		&factory_EmitterGeomExplicitParams_0p0,
		EmitterGeomExplicitParams_0p0::freeParameterDefinitionTable,
		ConversionEmitterGeomExplicitParams_0p0_0p1::Create,
		0
	},
	{
		0,
		1,
		&factory_EmitterGeomSphereParams_0p0,
		EmitterGeomSphereParams_0p0::freeParameterDefinitionTable,
		ConversionEmitterGeomSphereParams_0p0_0p1::Create,
		0
	},
	{
		0,
		1,
		&factory_EmitterGeomSphereShellParams_0p0,
		EmitterGeomSphereShellParams_0p0::freeParameterDefinitionTable,
		ConversionEmitterGeomSphereShellParams_0p0_0p1::Create,
		0
	},
	{
		2,
		3,
		&factory_ApexEmitterAssetParameters_0p2,
		ApexEmitterAssetParameters_0p2::freeParameterDefinitionTable,
		ConversionApexEmitterAssetParameters_0p2_0p3::Create,
		0
	},
	{
		3,
		4,
		&factory_ApexEmitterAssetParameters_0p3,
		ApexEmitterAssetParameters_0p3::freeParameterDefinitionTable,
		ConversionApexEmitterAssetParameters_0p3_0p4::Create,
		0
	},
	{
		4,
		5,
		&factory_ApexEmitterAssetParameters_0p4,
		ApexEmitterAssetParameters_0p4::freeParameterDefinitionTable,
		ConversionApexEmitterAssetParameters_0p4_0p5::Create,
		0
	},
	{
		5,
		6,
		&factory_ApexEmitterAssetParameters_0p5,
		ApexEmitterAssetParameters_0p5::freeParameterDefinitionTable,
		ConversionApexEmitterAssetParameters_0p5_0p6::Create,
		0
	},
	// AUTO_GENERATED_TABLE_END

	{ 0, 0, 0, 0, 0, 0} // Terminator
};

class ModuleEmitterLegacy : public ApexLegacyModule, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ModuleEmitterLegacy(NiApexSDK* sdk);

protected:
	void releaseLegacyObjects();

private:

	// Add custom conversions here

};

DEFINE_INSTANTIATE_MODULE(ModuleEmitterLegacy)

ModuleEmitterLegacy::ModuleEmitterLegacy(NiApexSDK* inSdk)
{
	name = "Emitter_Legacy";
	mSdk = inSdk;
	mApiProxy = this;

	// Register legacy stuff

	NxParameterized::Traits* t = mSdk->getParameterizedTraits();
	if (!t)
	{
		return;
	}

	// Register auto-generated objects
	registerLegacyObjects(ModuleEmitterLegacyObjects);

	// Register custom conversions here
}

void ModuleEmitterLegacy::releaseLegacyObjects()
{
	//Release legacy stuff

	NxParameterized::Traits* t = mSdk->getParameterizedTraits();
	if (!t)
	{
		return;
	}

	// Unregister auto-generated objects
	unregisterLegacyObjects(ModuleEmitterLegacyObjects);

	// Unregister custom conversions here
}

}
}
} // end namespace physx::apex
