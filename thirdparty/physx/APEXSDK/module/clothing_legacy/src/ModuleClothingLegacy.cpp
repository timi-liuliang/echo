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
#include "ClothingGraphicalLodParameters_0p0.h"
#include "ClothingGraphicalLodParameters_0p1.h"
#include "ConversionClothingGraphicalLodParameters_0p0_0p1.h"
#include "ClothingPhysicalMeshParameters_0p0.h"
#include "ClothingPhysicalMeshParameters_0p1.h"
#include "ConversionClothingPhysicalMeshParameters_0p0_0p1.h"
#include "ClothingPhysicalMeshParameters_0p2.h"
#include "ConversionClothingPhysicalMeshParameters_0p1_0p2.h"
#include "ClothingMaterialLibraryParameters_0p0.h"
#include "ClothingMaterialLibraryParameters_0p1.h"
#include "ConversionClothingMaterialLibraryParameters_0p0_0p1.h"
#include "ClothingGraphicalLodParameters_0p2.h"
#include "ConversionClothingGraphicalLodParameters_0p1_0p2.h"
#include "ClothingAssetParameters_0p0.h"
#include "ClothingAssetParameters_0p1.h"
#include "ConversionClothingAssetParameters_0p0_0p1.h"
#include "ClothingAssetParameters_0p2.h"
#include "ConversionClothingAssetParameters_0p1_0p2.h"
#include "ClothingCookedParam_0p0.h"
#include "ClothingCookedParam_0p1.h"
#include "ConversionClothingCookedParam_0p0_0p1.h"
#include "ClothingPhysicalMeshParameters_0p3.h"
#include "ConversionClothingPhysicalMeshParameters_0p2_0p3.h"
#include "ClothingActorParam_0p0.h"
#include "ClothingActorParam_0p1.h"
#include "ConversionClothingActorParam_0p0_0p1.h"
#include "ClothingActorParam_0p2.h"
#include "ConversionClothingActorParam_0p1_0p2.h"
#include "ClothingPhysicalMeshParameters_0p4.h"
#include "ConversionClothingPhysicalMeshParameters_0p3_0p4.h"
#include "ClothingActorParam_0p3.h"
#include "ConversionClothingActorParam_0p2_0p3.h"
#include "ClothingAssetParameters_0p3.h"
#include "ConversionClothingAssetParameters_0p2_0p3.h"
#include "ClothingActorParam_0p4.h"
#include "ConversionClothingActorParam_0p3_0p4.h"
#include "ClothingPhysicalMeshParameters_0p5.h"
#include "ConversionClothingPhysicalMeshParameters_0p4_0p5.h"
#include "ClothingActorParam_0p5.h"
#include "ConversionClothingActorParam_0p4_0p5.h"
#include "ClothingPhysicalMeshParameters_0p6.h"
#include "ConversionClothingPhysicalMeshParameters_0p5_0p6.h"
#include "ClothingActorParam_0p6.h"
#include "ConversionClothingActorParam_0p5_0p6.h"
#include "ClothingActorParam_0p7.h"
#include "ConversionClothingActorParam_0p6_0p7.h"
#include "ClothingPhysicalMeshParameters_0p7.h"
#include "ConversionClothingPhysicalMeshParameters_0p6_0p7.h"
#include "ClothingActorParam_0p8.h"
#include "ConversionClothingActorParam_0p7_0p8.h"
#include "ClothingActorParam_0p9.h"
#include "ConversionClothingActorParam_0p8_0p9.h"
#include "ClothingAssetParameters_0p4.h"
#include "ConversionClothingAssetParameters_0p3_0p4.h"
#include "ClothingActorParam_0p10.h"
#include "ConversionClothingActorParam_0p9_0p10.h"
#include "ClothingActorParam_0p11.h"
#include "ConversionClothingActorParam_0p10_0p11.h"
#include "ClothingAssetParameters_0p5.h"
#include "ConversionClothingAssetParameters_0p4_0p5.h"
#include "ClothingActorParam_0p12.h"
#include "ConversionClothingActorParam_0p11_0p12.h"
#include "ClothingCookedParam_0p2.h"
#include "ConversionClothingCookedParam_0p1_0p2.h"
#include "ClothingPhysicalMeshParameters_0p8.h"
#include "ConversionClothingPhysicalMeshParameters_0p7_0p8.h"
#include "ClothingMaterialLibraryParameters_0p2.h"
#include "ConversionClothingMaterialLibraryParameters_0p1_0p2.h"
#include "ClothingAssetParameters_0p6.h"
#include "ConversionClothingAssetParameters_0p5_0p6.h"
#include "ClothingActorParam_0p13.h"
#include "ConversionClothingActorParam_0p12_0p13.h"
#include "ClothingMaterialLibraryParameters_0p3.h"
#include "ConversionClothingMaterialLibraryParameters_0p2_0p3.h"
#include "ClothingAssetParameters_0p7.h"
#include "ConversionClothingAssetParameters_0p6_0p7.h"
#include "ClothingMaterialLibraryParameters_0p4.h"
#include "ConversionClothingMaterialLibraryParameters_0p3_0p4.h"
#include "ClothingMaterialLibraryParameters_0p5.h"
#include "ConversionClothingMaterialLibraryParameters_0p4_0p5.h"
#include "ClothingAssetParameters_0p8.h"
#include "ConversionClothingAssetParameters_0p7_0p8.h"
#include "ClothingMaterialLibraryParameters_0p6.h"
#include "ConversionClothingMaterialLibraryParameters_0p5_0p6.h"
#include "ClothingMaterialLibraryParameters_0p7.h"
#include "ConversionClothingMaterialLibraryParameters_0p6_0p7.h"
#include "ClothingMaterialLibraryParameters_0p8.h"
#include "ConversionClothingMaterialLibraryParameters_0p7_0p8.h"
#include "ClothingGraphicalLodParameters_0p3.h"
#include "ConversionClothingGraphicalLodParameters_0p2_0p3.h"
#include "ClothingAssetParameters_0p9.h"
#include "ConversionClothingAssetParameters_0p8_0p9.h"
#include "ClothingPhysicalMeshParameters_0p9.h"
#include "ConversionClothingPhysicalMeshParameters_0p8_0p9.h"
#include "ClothingMaterialLibraryParameters_0p9.h"
#include "ConversionClothingMaterialLibraryParameters_0p8_0p9.h"
#include "ClothingMaterialLibraryParameters_0p10.h"
#include "ConversionClothingMaterialLibraryParameters_0p9_0p10.h"
#include "ClothingAssetParameters_0p10.h"
#include "ConversionClothingAssetParameters_0p9_0p10.h"
#include "ClothingMaterialLibraryParameters_0p11.h"
#include "ConversionClothingMaterialLibraryParameters_0p10_0p11.h"
#include "ClothingActorParam_0p14.h"
#include "ConversionClothingActorParam_0p13_0p14.h"
#include "ClothingAssetParameters_0p11.h"
#include "ConversionClothingAssetParameters_0p10_0p11.h"
#include "ClothingActorParam_0p15.h"
#include "ConversionClothingActorParam_0p14_0p15.h"
#include "ClothingAssetParameters_0p12.h"
#include "ConversionClothingAssetParameters_0p11_0p12.h"
#include "ClothingGraphicalLodParameters_0p4.h"
#include "ConversionClothingGraphicalLodParameters_0p3_0p4.h"
#include "ClothingPhysicalMeshParameters_0p10.h"
#include "ConversionClothingPhysicalMeshParameters_0p9_0p10.h"
#include "ClothingCookedPhysX3Param_0p0.h"
#include "ClothingCookedPhysX3Param_0p1.h"
#include "ConversionClothingCookedPhysX3Param_0p0_0p1.h"
#include "ClothingActorParam_0p16.h"
#include "ConversionClothingActorParam_0p15_0p16.h"
#include "ClothingMaterialLibraryParameters_0p12.h"
#include "ConversionClothingMaterialLibraryParameters_0p11_0p12.h"
#include "ClothingAssetParameters_0p13.h"
#include "ConversionClothingAssetParameters_0p12_0p13.h"
#include "ClothingCookedPhysX3Param_0p2.h"
#include "ConversionClothingCookedPhysX3Param_0p1_0p2.h"
#include "ClothingActorParam_0p17.h"
#include "ConversionClothingActorParam_0p16_0p17.h"
#include "ClothingCookedPhysX3Param_0p3.h"
#include "ConversionClothingCookedPhysX3Param_0p2_0p3.h"
#include "ClothingMaterialLibraryParameters_0p13.h"
#include "ConversionClothingMaterialLibraryParameters_0p12_0p13.h"
#include "ClothingMaterialLibraryParameters_0p14.h"
#include "ConversionClothingMaterialLibraryParameters_0p13_0p14.h"
#include "ClothingCookedPhysX3Param_0p4.h"
#include "ConversionClothingCookedPhysX3Param_0p3_0p4.h"
// AUTO_GENERATED_INCLUDES_END

namespace physx
{
namespace apex
{
namespace legacy
{

// AUTO_GENERATED_OBJECTS_BEGIN
static ClothingGraphicalLodParameters_0p0Factory factory_ClothingGraphicalLodParameters_0p0;
static ClothingGraphicalLodParameters_0p1Factory factory_ClothingGraphicalLodParameters_0p1;
static ClothingPhysicalMeshParameters_0p0Factory factory_ClothingPhysicalMeshParameters_0p0;
static ClothingPhysicalMeshParameters_0p1Factory factory_ClothingPhysicalMeshParameters_0p1;
static ClothingPhysicalMeshParameters_0p2Factory factory_ClothingPhysicalMeshParameters_0p2;
static ClothingMaterialLibraryParameters_0p0Factory factory_ClothingMaterialLibraryParameters_0p0;
static ClothingMaterialLibraryParameters_0p1Factory factory_ClothingMaterialLibraryParameters_0p1;
static ClothingGraphicalLodParameters_0p2Factory factory_ClothingGraphicalLodParameters_0p2;
static ClothingAssetParameters_0p0Factory factory_ClothingAssetParameters_0p0;
static ClothingAssetParameters_0p1Factory factory_ClothingAssetParameters_0p1;
static ClothingAssetParameters_0p2Factory factory_ClothingAssetParameters_0p2;
static ClothingCookedParam_0p0Factory factory_ClothingCookedParam_0p0;
static ClothingCookedParam_0p1Factory factory_ClothingCookedParam_0p1;
static ClothingPhysicalMeshParameters_0p3Factory factory_ClothingPhysicalMeshParameters_0p3;
static ClothingActorParam_0p0Factory factory_ClothingActorParam_0p0;
static ClothingActorParam_0p1Factory factory_ClothingActorParam_0p1;
static ClothingActorParam_0p2Factory factory_ClothingActorParam_0p2;
static ClothingPhysicalMeshParameters_0p4Factory factory_ClothingPhysicalMeshParameters_0p4;
static ClothingActorParam_0p3Factory factory_ClothingActorParam_0p3;
static ClothingAssetParameters_0p3Factory factory_ClothingAssetParameters_0p3;
static ClothingActorParam_0p4Factory factory_ClothingActorParam_0p4;
static ClothingPhysicalMeshParameters_0p5Factory factory_ClothingPhysicalMeshParameters_0p5;
static ClothingActorParam_0p5Factory factory_ClothingActorParam_0p5;
static ClothingPhysicalMeshParameters_0p6Factory factory_ClothingPhysicalMeshParameters_0p6;
static ClothingActorParam_0p6Factory factory_ClothingActorParam_0p6;
static ClothingActorParam_0p7Factory factory_ClothingActorParam_0p7;
static ClothingPhysicalMeshParameters_0p7Factory factory_ClothingPhysicalMeshParameters_0p7;
static ClothingActorParam_0p8Factory factory_ClothingActorParam_0p8;
static ClothingActorParam_0p9Factory factory_ClothingActorParam_0p9;
static ClothingAssetParameters_0p4Factory factory_ClothingAssetParameters_0p4;
static ClothingActorParam_0p10Factory factory_ClothingActorParam_0p10;
static ClothingActorParam_0p11Factory factory_ClothingActorParam_0p11;
static ClothingAssetParameters_0p5Factory factory_ClothingAssetParameters_0p5;
static ClothingActorParam_0p12Factory factory_ClothingActorParam_0p12;
static ClothingCookedParam_0p2Factory factory_ClothingCookedParam_0p2;
static ClothingPhysicalMeshParameters_0p8Factory factory_ClothingPhysicalMeshParameters_0p8;
static ClothingMaterialLibraryParameters_0p2Factory factory_ClothingMaterialLibraryParameters_0p2;
static ClothingAssetParameters_0p6Factory factory_ClothingAssetParameters_0p6;
static ClothingActorParam_0p13Factory factory_ClothingActorParam_0p13;
static ClothingMaterialLibraryParameters_0p3Factory factory_ClothingMaterialLibraryParameters_0p3;
static ClothingAssetParameters_0p7Factory factory_ClothingAssetParameters_0p7;
static ClothingMaterialLibraryParameters_0p4Factory factory_ClothingMaterialLibraryParameters_0p4;
static ClothingMaterialLibraryParameters_0p5Factory factory_ClothingMaterialLibraryParameters_0p5;
static ClothingAssetParameters_0p8Factory factory_ClothingAssetParameters_0p8;
static ClothingMaterialLibraryParameters_0p6Factory factory_ClothingMaterialLibraryParameters_0p6;
static ClothingMaterialLibraryParameters_0p7Factory factory_ClothingMaterialLibraryParameters_0p7;
static ClothingMaterialLibraryParameters_0p8Factory factory_ClothingMaterialLibraryParameters_0p8;
static ClothingGraphicalLodParameters_0p3Factory factory_ClothingGraphicalLodParameters_0p3;
static ClothingAssetParameters_0p9Factory factory_ClothingAssetParameters_0p9;
static ClothingPhysicalMeshParameters_0p9Factory factory_ClothingPhysicalMeshParameters_0p9;
static ClothingMaterialLibraryParameters_0p9Factory factory_ClothingMaterialLibraryParameters_0p9;
static ClothingMaterialLibraryParameters_0p10Factory factory_ClothingMaterialLibraryParameters_0p10;
static ClothingAssetParameters_0p10Factory factory_ClothingAssetParameters_0p10;
static ClothingMaterialLibraryParameters_0p11Factory factory_ClothingMaterialLibraryParameters_0p11;
static ClothingActorParam_0p14Factory factory_ClothingActorParam_0p14;
static ClothingAssetParameters_0p11Factory factory_ClothingAssetParameters_0p11;
static ClothingActorParam_0p15Factory factory_ClothingActorParam_0p15;
static ClothingAssetParameters_0p12Factory factory_ClothingAssetParameters_0p12;
static ClothingGraphicalLodParameters_0p4Factory factory_ClothingGraphicalLodParameters_0p4;
static ClothingPhysicalMeshParameters_0p10Factory factory_ClothingPhysicalMeshParameters_0p10;
static ClothingCookedPhysX3Param_0p0Factory factory_ClothingCookedPhysX3Param_0p0;
static ClothingCookedPhysX3Param_0p1Factory factory_ClothingCookedPhysX3Param_0p1;
static ClothingActorParam_0p16Factory factory_ClothingActorParam_0p16;
static ClothingMaterialLibraryParameters_0p12Factory factory_ClothingMaterialLibraryParameters_0p12;
static ClothingAssetParameters_0p13Factory factory_ClothingAssetParameters_0p13;
static ClothingCookedPhysX3Param_0p2Factory factory_ClothingCookedPhysX3Param_0p2;
static ClothingActorParam_0p17Factory factory_ClothingActorParam_0p17;
static ClothingCookedPhysX3Param_0p3Factory factory_ClothingCookedPhysX3Param_0p3;
static ClothingMaterialLibraryParameters_0p13Factory factory_ClothingMaterialLibraryParameters_0p13;
static ClothingMaterialLibraryParameters_0p14Factory factory_ClothingMaterialLibraryParameters_0p14;
static ClothingCookedPhysX3Param_0p4Factory factory_ClothingCookedPhysX3Param_0p4;
// AUTO_GENERATED_OBJECTS_END

static LegacyClassEntry ModuleClothingLegacyObjects[] =
{
	// AUTO_GENERATED_TABLE_BEGIN
	{
		0,
		1,
		&factory_ClothingGraphicalLodParameters_0p0,
		ClothingGraphicalLodParameters_0p0::freeParameterDefinitionTable,
		ConversionClothingGraphicalLodParameters_0p0_0p1::Create,
		0
	},
	{
		0,
		1,
		&factory_ClothingPhysicalMeshParameters_0p0,
		ClothingPhysicalMeshParameters_0p0::freeParameterDefinitionTable,
		ConversionClothingPhysicalMeshParameters_0p0_0p1::Create,
		0
	},
	{
		1,
		2,
		&factory_ClothingPhysicalMeshParameters_0p1,
		ClothingPhysicalMeshParameters_0p1::freeParameterDefinitionTable,
		ConversionClothingPhysicalMeshParameters_0p1_0p2::Create,
		0
	},
	{
		0,
		1,
		&factory_ClothingMaterialLibraryParameters_0p0,
		ClothingMaterialLibraryParameters_0p0::freeParameterDefinitionTable,
		ConversionClothingMaterialLibraryParameters_0p0_0p1::Create,
		0
	},
	{
		1,
		2,
		&factory_ClothingGraphicalLodParameters_0p1,
		ClothingGraphicalLodParameters_0p1::freeParameterDefinitionTable,
		ConversionClothingGraphicalLodParameters_0p1_0p2::Create,
		0
	},
	{
		0,
		1,
		&factory_ClothingAssetParameters_0p0,
		ClothingAssetParameters_0p0::freeParameterDefinitionTable,
		ConversionClothingAssetParameters_0p0_0p1::Create,
		0
	},
	{
		1,
		2,
		&factory_ClothingAssetParameters_0p1,
		ClothingAssetParameters_0p1::freeParameterDefinitionTable,
		ConversionClothingAssetParameters_0p1_0p2::Create,
		0
	},
	{
		0,
		1,
		&factory_ClothingCookedParam_0p0,
		ClothingCookedParam_0p0::freeParameterDefinitionTable,
		ConversionClothingCookedParam_0p0_0p1::Create,
		0
	},
	{
		2,
		3,
		&factory_ClothingPhysicalMeshParameters_0p2,
		ClothingPhysicalMeshParameters_0p2::freeParameterDefinitionTable,
		ConversionClothingPhysicalMeshParameters_0p2_0p3::Create,
		0
	},
	{
		0,
		1,
		&factory_ClothingActorParam_0p0,
		ClothingActorParam_0p0::freeParameterDefinitionTable,
		ConversionClothingActorParam_0p0_0p1::Create,
		0
	},
	{
		1,
		2,
		&factory_ClothingActorParam_0p1,
		ClothingActorParam_0p1::freeParameterDefinitionTable,
		ConversionClothingActorParam_0p1_0p2::Create,
		0
	},
	{
		3,
		4,
		&factory_ClothingPhysicalMeshParameters_0p3,
		ClothingPhysicalMeshParameters_0p3::freeParameterDefinitionTable,
		ConversionClothingPhysicalMeshParameters_0p3_0p4::Create,
		0
	},
	{
		2,
		3,
		&factory_ClothingActorParam_0p2,
		ClothingActorParam_0p2::freeParameterDefinitionTable,
		ConversionClothingActorParam_0p2_0p3::Create,
		0
	},
	{
		2,
		3,
		&factory_ClothingAssetParameters_0p2,
		ClothingAssetParameters_0p2::freeParameterDefinitionTable,
		ConversionClothingAssetParameters_0p2_0p3::Create,
		0
	},
	{
		3,
		4,
		&factory_ClothingActorParam_0p3,
		ClothingActorParam_0p3::freeParameterDefinitionTable,
		ConversionClothingActorParam_0p3_0p4::Create,
		0
	},
	{
		4,
		5,
		&factory_ClothingPhysicalMeshParameters_0p4,
		ClothingPhysicalMeshParameters_0p4::freeParameterDefinitionTable,
		ConversionClothingPhysicalMeshParameters_0p4_0p5::Create,
		0
	},
	{
		4,
		5,
		&factory_ClothingActorParam_0p4,
		ClothingActorParam_0p4::freeParameterDefinitionTable,
		ConversionClothingActorParam_0p4_0p5::Create,
		0
	},
	{
		5,
		6,
		&factory_ClothingPhysicalMeshParameters_0p5,
		ClothingPhysicalMeshParameters_0p5::freeParameterDefinitionTable,
		ConversionClothingPhysicalMeshParameters_0p5_0p6::Create,
		0
	},
	{
		5,
		6,
		&factory_ClothingActorParam_0p5,
		ClothingActorParam_0p5::freeParameterDefinitionTable,
		ConversionClothingActorParam_0p5_0p6::Create,
		0
	},
	{
		6,
		7,
		&factory_ClothingActorParam_0p6,
		ClothingActorParam_0p6::freeParameterDefinitionTable,
		ConversionClothingActorParam_0p6_0p7::Create,
		0
	},
	{
		6,
		7,
		&factory_ClothingPhysicalMeshParameters_0p6,
		ClothingPhysicalMeshParameters_0p6::freeParameterDefinitionTable,
		ConversionClothingPhysicalMeshParameters_0p6_0p7::Create,
		0
	},
	{
		7,
		8,
		&factory_ClothingActorParam_0p7,
		ClothingActorParam_0p7::freeParameterDefinitionTable,
		ConversionClothingActorParam_0p7_0p8::Create,
		0
	},
	{
		8,
		9,
		&factory_ClothingActorParam_0p8,
		ClothingActorParam_0p8::freeParameterDefinitionTable,
		ConversionClothingActorParam_0p8_0p9::Create,
		0
	},
	{
		3,
		4,
		&factory_ClothingAssetParameters_0p3,
		ClothingAssetParameters_0p3::freeParameterDefinitionTable,
		ConversionClothingAssetParameters_0p3_0p4::Create,
		0
	},
	{
		9,
		10,
		&factory_ClothingActorParam_0p9,
		ClothingActorParam_0p9::freeParameterDefinitionTable,
		ConversionClothingActorParam_0p9_0p10::Create,
		0
	},
	{
		10,
		11,
		&factory_ClothingActorParam_0p10,
		ClothingActorParam_0p10::freeParameterDefinitionTable,
		ConversionClothingActorParam_0p10_0p11::Create,
		0
	},
	{
		4,
		5,
		&factory_ClothingAssetParameters_0p4,
		ClothingAssetParameters_0p4::freeParameterDefinitionTable,
		ConversionClothingAssetParameters_0p4_0p5::Create,
		0
	},
	{
		11,
		12,
		&factory_ClothingActorParam_0p11,
		ClothingActorParam_0p11::freeParameterDefinitionTable,
		ConversionClothingActorParam_0p11_0p12::Create,
		0
	},
	{
		1,
		2,
		&factory_ClothingCookedParam_0p1,
		ClothingCookedParam_0p1::freeParameterDefinitionTable,
		ConversionClothingCookedParam_0p1_0p2::Create,
		0
	},
	{
		7,
		8,
		&factory_ClothingPhysicalMeshParameters_0p7,
		ClothingPhysicalMeshParameters_0p7::freeParameterDefinitionTable,
		ConversionClothingPhysicalMeshParameters_0p7_0p8::Create,
		0
	},
	{
		1,
		2,
		&factory_ClothingMaterialLibraryParameters_0p1,
		ClothingMaterialLibraryParameters_0p1::freeParameterDefinitionTable,
		ConversionClothingMaterialLibraryParameters_0p1_0p2::Create,
		0
	},
	{
		5,
		6,
		&factory_ClothingAssetParameters_0p5,
		ClothingAssetParameters_0p5::freeParameterDefinitionTable,
		ConversionClothingAssetParameters_0p5_0p6::Create,
		0
	},
	{
		12,
		13,
		&factory_ClothingActorParam_0p12,
		ClothingActorParam_0p12::freeParameterDefinitionTable,
		ConversionClothingActorParam_0p12_0p13::Create,
		0
	},
	{
		2,
		3,
		&factory_ClothingMaterialLibraryParameters_0p2,
		ClothingMaterialLibraryParameters_0p2::freeParameterDefinitionTable,
		ConversionClothingMaterialLibraryParameters_0p2_0p3::Create,
		0
	},
	{
		6,
		7,
		&factory_ClothingAssetParameters_0p6,
		ClothingAssetParameters_0p6::freeParameterDefinitionTable,
		ConversionClothingAssetParameters_0p6_0p7::Create,
		0
	},
	{
		3,
		4,
		&factory_ClothingMaterialLibraryParameters_0p3,
		ClothingMaterialLibraryParameters_0p3::freeParameterDefinitionTable,
		ConversionClothingMaterialLibraryParameters_0p3_0p4::Create,
		0
	},
	{
		4,
		5,
		&factory_ClothingMaterialLibraryParameters_0p4,
		ClothingMaterialLibraryParameters_0p4::freeParameterDefinitionTable,
		ConversionClothingMaterialLibraryParameters_0p4_0p5::Create,
		0
	},
	{
		7,
		8,
		&factory_ClothingAssetParameters_0p7,
		ClothingAssetParameters_0p7::freeParameterDefinitionTable,
		ConversionClothingAssetParameters_0p7_0p8::Create,
		0
	},
	{
		5,
		6,
		&factory_ClothingMaterialLibraryParameters_0p5,
		ClothingMaterialLibraryParameters_0p5::freeParameterDefinitionTable,
		ConversionClothingMaterialLibraryParameters_0p5_0p6::Create,
		0
	},
	{
		6,
		7,
		&factory_ClothingMaterialLibraryParameters_0p6,
		ClothingMaterialLibraryParameters_0p6::freeParameterDefinitionTable,
		ConversionClothingMaterialLibraryParameters_0p6_0p7::Create,
		0
	},
	{
		7,
		8,
		&factory_ClothingMaterialLibraryParameters_0p7,
		ClothingMaterialLibraryParameters_0p7::freeParameterDefinitionTable,
		ConversionClothingMaterialLibraryParameters_0p7_0p8::Create,
		0
	},
	{
		2,
		3,
		&factory_ClothingGraphicalLodParameters_0p2,
		ClothingGraphicalLodParameters_0p2::freeParameterDefinitionTable,
		ConversionClothingGraphicalLodParameters_0p2_0p3::Create,
		0
	},
	{
		8,
		9,
		&factory_ClothingAssetParameters_0p8,
		ClothingAssetParameters_0p8::freeParameterDefinitionTable,
		ConversionClothingAssetParameters_0p8_0p9::Create,
		0
	},
	{
		8,
		9,
		&factory_ClothingPhysicalMeshParameters_0p8,
		ClothingPhysicalMeshParameters_0p8::freeParameterDefinitionTable,
		ConversionClothingPhysicalMeshParameters_0p8_0p9::Create,
		0
	},
	{
		8,
		9,
		&factory_ClothingMaterialLibraryParameters_0p8,
		ClothingMaterialLibraryParameters_0p8::freeParameterDefinitionTable,
		ConversionClothingMaterialLibraryParameters_0p8_0p9::Create,
		0
	},
	{
		9,
		10,
		&factory_ClothingMaterialLibraryParameters_0p9,
		ClothingMaterialLibraryParameters_0p9::freeParameterDefinitionTable,
		ConversionClothingMaterialLibraryParameters_0p9_0p10::Create,
		0
	},
	{
		9,
		10,
		&factory_ClothingAssetParameters_0p9,
		ClothingAssetParameters_0p9::freeParameterDefinitionTable,
		ConversionClothingAssetParameters_0p9_0p10::Create,
		0
	},
	{
		10,
		11,
		&factory_ClothingMaterialLibraryParameters_0p10,
		ClothingMaterialLibraryParameters_0p10::freeParameterDefinitionTable,
		ConversionClothingMaterialLibraryParameters_0p10_0p11::Create,
		0
	},
	{
		13,
		14,
		&factory_ClothingActorParam_0p13,
		ClothingActorParam_0p13::freeParameterDefinitionTable,
		ConversionClothingActorParam_0p13_0p14::Create,
		0
	},
	{
		10,
		11,
		&factory_ClothingAssetParameters_0p10,
		ClothingAssetParameters_0p10::freeParameterDefinitionTable,
		ConversionClothingAssetParameters_0p10_0p11::Create,
		0
	},
	{
		14,
		15,
		&factory_ClothingActorParam_0p14,
		ClothingActorParam_0p14::freeParameterDefinitionTable,
		ConversionClothingActorParam_0p14_0p15::Create,
		0
	},
	{
		11,
		12,
		&factory_ClothingAssetParameters_0p11,
		ClothingAssetParameters_0p11::freeParameterDefinitionTable,
		ConversionClothingAssetParameters_0p11_0p12::Create,
		0
	},
	{
		3,
		4,
		&factory_ClothingGraphicalLodParameters_0p3,
		ClothingGraphicalLodParameters_0p3::freeParameterDefinitionTable,
		ConversionClothingGraphicalLodParameters_0p3_0p4::Create,
		0
	},
	{
		9,
		10,
		&factory_ClothingPhysicalMeshParameters_0p9,
		ClothingPhysicalMeshParameters_0p9::freeParameterDefinitionTable,
		ConversionClothingPhysicalMeshParameters_0p9_0p10::Create,
		0
	},
	{
		0,
		1,
		&factory_ClothingCookedPhysX3Param_0p0,
		ClothingCookedPhysX3Param_0p0::freeParameterDefinitionTable,
		ConversionClothingCookedPhysX3Param_0p0_0p1::Create,
		0
	},
	{
		15,
		16,
		&factory_ClothingActorParam_0p15,
		ClothingActorParam_0p15::freeParameterDefinitionTable,
		ConversionClothingActorParam_0p15_0p16::Create,
		0
	},
	{
		11,
		12,
		&factory_ClothingMaterialLibraryParameters_0p11,
		ClothingMaterialLibraryParameters_0p11::freeParameterDefinitionTable,
		ConversionClothingMaterialLibraryParameters_0p11_0p12::Create,
		0
	},
	{
		12,
		13,
		&factory_ClothingAssetParameters_0p12,
		ClothingAssetParameters_0p12::freeParameterDefinitionTable,
		ConversionClothingAssetParameters_0p12_0p13::Create,
		0
	},
	{
		1,
		2,
		&factory_ClothingCookedPhysX3Param_0p1,
		ClothingCookedPhysX3Param_0p1::freeParameterDefinitionTable,
		ConversionClothingCookedPhysX3Param_0p1_0p2::Create,
		0
	},
	{
		16,
		17,
		&factory_ClothingActorParam_0p16,
		ClothingActorParam_0p16::freeParameterDefinitionTable,
		ConversionClothingActorParam_0p16_0p17::Create,
		0
	},
	{
		2,
		3,
		&factory_ClothingCookedPhysX3Param_0p2,
		ClothingCookedPhysX3Param_0p2::freeParameterDefinitionTable,
		ConversionClothingCookedPhysX3Param_0p2_0p3::Create,
		0
	},
	{
		12,
		13,
		&factory_ClothingMaterialLibraryParameters_0p12,
		ClothingMaterialLibraryParameters_0p12::freeParameterDefinitionTable,
		ConversionClothingMaterialLibraryParameters_0p12_0p13::Create,
		0
	},
	{
		13,
		14,
		&factory_ClothingMaterialLibraryParameters_0p13,
		ClothingMaterialLibraryParameters_0p13::freeParameterDefinitionTable,
		ConversionClothingMaterialLibraryParameters_0p13_0p14::Create,
		0
	},
	{
		3,
		4,
		&factory_ClothingCookedPhysX3Param_0p3,
		ClothingCookedPhysX3Param_0p3::freeParameterDefinitionTable,
		ConversionClothingCookedPhysX3Param_0p3_0p4::Create,
		0
	},
	// AUTO_GENERATED_TABLE_END

	{ 0, 0, 0, 0, 0, 0} // Terminator
};

class ModuleClothingLegacy : public ApexLegacyModule, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ModuleClothingLegacy(NiApexSDK* sdk);

protected:
	void releaseLegacyObjects();

private:

	// Add custom conversions here

};

DEFINE_INSTANTIATE_MODULE(ModuleClothingLegacy)

ModuleClothingLegacy::ModuleClothingLegacy(NiApexSDK* inSdk)
{
	name = "Clothing_Legacy";
	mSdk = inSdk;
	mApiProxy = this;

	// Register legacy stuff

	NxParameterized::Traits* t = mSdk->getParameterizedTraits();
	if (!t)
	{
		return;
	}

	// Register auto-generated objects
	registerLegacyObjects(ModuleClothingLegacyObjects);

	// Register custom conversions here
}

void ModuleClothingLegacy::releaseLegacyObjects()
{
	//Release legacy stuff

	NxParameterized::Traits* t = mSdk->getParameterizedTraits();
	if (!t)
	{
		return;
	}

	// Unregister auto-generated objects
	unregisterLegacyObjects(ModuleClothingLegacyObjects);

	// Unregister custom conversions here
}

}
}
} // end namespace physx::apex
