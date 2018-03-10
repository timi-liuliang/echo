/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __DESTRUCTIBLEACTORUTILS_H__
#define __DESTRUCTIBLEACTORUTILS_H__

#include "NxApexDefs.h"
#include "MinPhysxSdkVersion.h"
#if NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED

#include "DestructibleActor.h"

#if NX_SDK_VERSION_MAJOR == 3
#include "PxPhysics.h"
#endif

namespace physx
{
namespace apex
{
namespace destructible
{

#ifdef _DEBUG
#define VERIFY_PARAM(_A) PX_ASSERT((_A) == NxParameterized::ERROR_NONE)
#else
#define VERIFY_PARAM(_A) (_A)
#endif


enum AssignType {
	AtoB,
	BtoA,
	Default = BtoA,
	Inverse = AtoB
};

static const bool Serialize   = true;
static const bool Deserialize = false;

///////////////////////////////////////////////////////////////////////////

static const char* typeName(NxDestructibleParameterizedType::Enum e)
{
	const char* typeName = "";
	switch (e)
	{
	case NxDestructibleParameterizedType::State:
		typeName = DestructibleActorState::staticClassName();
		break;
	case NxDestructibleParameterizedType::Params:
		typeName = DestructibleActorParam::staticClassName();
		break;
	default:
		break;
	}
	return typeName;
}

PX_INLINE static bool isType(NxParameterized::Interface* i, NxDestructibleParameterizedType::Enum e)
{
	return physx::PxStricmp(i->className(), typeName(e)) == 0;
}

///////////////////////////////////////////////////////////////////////////

template <typename T, typename FlagType>
PX_INLINE void assignFlag(T& val, FlagType flag, bool setFlag)
{
	val = (T)(setFlag ? (val | (T)flag) : (val & ~(T)flag));
}

template <typename T, typename FlagType>
PX_INLINE void assignFlag(bool& flagSet, const T& val, FlagType flag)
{
	flagSet = (val & flag) ? true : false;
}

template<typename T, typename FlagType>
PX_INLINE void assignFlags(T& a, FlagType flag, bool& b, AssignType assignType)
{
	if (BtoA == assignType)
		assignFlag(a, flag, b);
	else
		assignFlag(b, a, flag);
}

template<typename T, typename U>
PX_INLINE void assign(T& a, U& b, AssignType assignType)
{
	if(AtoB == assignType)
		b = static_cast<U>(a);
	else
		a = static_cast<T>(b);
}

template<typename T, typename U, typename T1, typename U1>
PX_INLINE void assignPtr(T1& a, U1& b, AssignType assignType)
{
	if(AtoB == assignType)
		b = (U)a;
	else
		a = (T)b;
}

///////////////////////////////////////////////////////////////////////////

#if NX_SDK_VERSION_MAJOR == 2

void assign(NxActorDesc& descA,
            DestructibleActorParamNS::ActorDescTemplate_Type& descB,
            AssignType assignType = Default)
{
	assignFlags(descA.flags,              NX_AF_DISABLE_COLLISION,                  descB.flags.NX_AF_DISABLE_COLLISION,                               assignType);
	assignFlags(descA.flags,              NX_AF_DISABLE_RESPONSE,                   descB.flags.NX_AF_DISABLE_RESPONSE,                                assignType);
	assignFlags(descA.flags,              NX_AF_LOCK_COM,                           descB.flags.NX_AF_LOCK_COM,                                        assignType);
	assignFlags(descA.flags,              NX_AF_FLUID_DISABLE_COLLISION,            descB.flags.NX_AF_FLUID_DISABLE_COLLISION,                         assignType);
	assignFlags(descA.flags,              NX_AF_CONTACT_MODIFICATION,               descB.flags.NX_AF_CONTACT_MODIFICATION,                            assignType);
	assignFlags(descA.flags,              NX_AF_FORCE_CONE_FRICTION,                descB.flags.NX_AF_FORCE_CONE_FRICTION,                             assignType);
	assignFlags(descA.flags,              NX_AF_USER_ACTOR_PAIR_FILTERING,          descB.flags.NX_AF_USER_ACTOR_PAIR_FILTERING,                       assignType);
	assignFlags(descA.contactReportFlags, NX_IGNORE_PAIR,                           descB.contactReportFlags.NX_IGNORE_PAIR,                           assignType);
	assignFlags(descA.contactReportFlags, NX_NOTIFY_ON_START_TOUCH,                 descB.contactReportFlags.NX_NOTIFY_ON_START_TOUCH,                 assignType);
	assignFlags(descA.contactReportFlags, NX_NOTIFY_ON_END_TOUCH,                   descB.contactReportFlags.NX_NOTIFY_ON_END_TOUCH,                   assignType);
	assignFlags(descA.contactReportFlags, NX_NOTIFY_ON_TOUCH,                       descB.contactReportFlags.NX_NOTIFY_ON_TOUCH,                       assignType);
	assignFlags(descA.contactReportFlags, NX_NOTIFY_ON_IMPACT,                      descB.contactReportFlags.NX_NOTIFY_ON_IMPACT,                      assignType);
	assignFlags(descA.contactReportFlags, NX_NOTIFY_ON_ROLL,                        descB.contactReportFlags.NX_NOTIFY_ON_ROLL,                        assignType);
	assignFlags(descA.contactReportFlags, NX_NOTIFY_ON_SLIDE,                       descB.contactReportFlags.NX_NOTIFY_ON_SLIDE,                       assignType);
	assignFlags(descA.contactReportFlags, NX_NOTIFY_FORCES,                         descB.contactReportFlags.NX_NOTIFY_FORCES,                         assignType);
	assignFlags(descA.contactReportFlags, NX_NOTIFY_ON_START_TOUCH_FORCE_THRESHOLD, descB.contactReportFlags.NX_NOTIFY_ON_START_TOUCH_FORCE_THRESHOLD, assignType);
	assignFlags(descA.contactReportFlags, NX_NOTIFY_ON_END_TOUCH_FORCE_THRESHOLD,   descB.contactReportFlags.NX_NOTIFY_ON_END_TOUCH_FORCE_THRESHOLD,   assignType);
	assignFlags(descA.contactReportFlags, NX_NOTIFY_ON_TOUCH_FORCE_THRESHOLD,       descB.contactReportFlags.NX_NOTIFY_ON_TOUCH_FORCE_THRESHOLD,       assignType);
	assignFlags(descA.contactReportFlags, NX_NOTIFY_CONTACT_MODIFICATION,           descB.contactReportFlags.NX_NOTIFY_CONTACT_MODIFICATION,           assignType);

	assign(descA.density,            descB.density,                        assignType);
	assign(descA.group,              descB.actorCollisionGroup,            assignType);
	assign(descA.dominanceGroup,     descB.dominanceGroup,                 assignType);
	assign(descA.forceFieldMaterial, descB.forceFieldMaterial,             assignType);
	assignPtr<void*,size_t>(descA.userData,             descB.userData,    assignType);
	assignPtr<const char*,size_t>(descA.name,           descB.name,        assignType);
	assignPtr<NxCompartment*,size_t>(descA.compartment, descB.compartment, assignType);
}

///////////////////////////////////////////////////////////////////////////

void assign(NxBodyDesc& descA,
            DestructibleActorParamNS::BodyDescTemplate_Type& descB,
            AssignType assignType = Default)
{
	assignFlags(descA.flags, NX_BF_DISABLE_GRAVITY,   descB.flags.NX_BF_DISABLE_GRAVITY,   assignType);
	assignFlags(descA.flags, NX_BF_FILTER_SLEEP_VEL,  descB.flags.NX_BF_FILTER_SLEEP_VEL,  assignType);
	assignFlags(descA.flags, NX_BF_ENERGY_SLEEP_TEST, descB.flags.NX_BF_ENERGY_SLEEP_TEST, assignType);
	assignFlags(descA.flags, NX_BF_VISUALIZATION,     descB.flags.NX_BF_VISUALIZATION,     assignType);

	assign(descA.wakeUpCounter          , descB.wakeUpCounter,          assignType);
	assign(descA.linearDamping          , descB.linearDamping,          assignType);
	assign(descA.angularDamping         , descB.angularDamping,         assignType);
	assign(descA.maxAngularVelocity     , descB.maxAngularVelocity,     assignType);
	assign(descA.CCDMotionThreshold     , descB.CCDMotionThreshold,     assignType);
	assign(descA.sleepLinearVelocity    , descB.sleepLinearVelocity,    assignType);
	assign(descA.sleepAngularVelocity   , descB.sleepAngularVelocity,   assignType);
	assign(descA.solverIterationCount   , descB.solverIterationCount,   assignType);
	assign(descA.sleepEnergyThreshold   , descB.sleepEnergyThreshold,   assignType);
	assign(descA.sleepDamping           , descB.sleepDamping,           assignType);
	assign(descA.contactReportThreshold , descB.contactReportThreshold, assignType);
}

///////////////////////////////////////////////////////////////////////////

void assign(NxBoxShapeDesc& descA,
            DestructibleActorParamNS::ShapeDescTemplate_Type& descB,
            AssignType assignType = Default)
{
	assignFlags(descA.shapeFlags, NX_TRIGGER_ON_ENTER,              descB.flags.NX_TRIGGER_ON_ENTER,              assignType);
	assignFlags(descA.shapeFlags, NX_TRIGGER_ON_LEAVE,              descB.flags.NX_TRIGGER_ON_LEAVE,              assignType);
	assignFlags(descA.shapeFlags, NX_TRIGGER_ON_STAY,               descB.flags.NX_TRIGGER_ON_STAY,               assignType);
	assignFlags(descA.shapeFlags, NX_SF_VISUALIZATION,              descB.flags.NX_SF_VISUALIZATION,              assignType);
	assignFlags(descA.shapeFlags, NX_SF_DISABLE_COLLISION,          descB.flags.NX_SF_DISABLE_COLLISION,          assignType);
	assignFlags(descA.shapeFlags, NX_SF_FEATURE_INDICES,            descB.flags.NX_SF_FEATURE_INDICES,            assignType);
	assignFlags(descA.shapeFlags, NX_SF_DISABLE_RAYCASTING,         descB.flags.NX_SF_DISABLE_RAYCASTING,         assignType);
	assignFlags(descA.shapeFlags, NX_SF_POINT_CONTACT_FORCE,        descB.flags.NX_SF_POINT_CONTACT_FORCE,        assignType);
	assignFlags(descA.shapeFlags, NX_SF_FLUID_DRAIN,                descB.flags.NX_SF_FLUID_DRAIN,                assignType);
	assignFlags(descA.shapeFlags, NX_SF_FLUID_DISABLE_COLLISION,    descB.flags.NX_SF_FLUID_DISABLE_COLLISION,    assignType);
	assignFlags(descA.shapeFlags, NX_SF_FLUID_TWOWAY,               descB.flags.NX_SF_FLUID_TWOWAY,               assignType);
	assignFlags(descA.shapeFlags, NX_SF_DISABLE_RESPONSE,           descB.flags.NX_SF_DISABLE_RESPONSE,           assignType);
	assignFlags(descA.shapeFlags, NX_SF_DYNAMIC_DYNAMIC_CCD,        descB.flags.NX_SF_DYNAMIC_DYNAMIC_CCD,        assignType);
	assignFlags(descA.shapeFlags, NX_SF_DISABLE_SCENE_QUERIES,      descB.flags.NX_SF_DISABLE_SCENE_QUERIES,      assignType);
	assignFlags(descA.shapeFlags, NX_SF_CLOTH_DRAIN,                descB.flags.NX_SF_CLOTH_DRAIN,                assignType);
	assignFlags(descA.shapeFlags, NX_SF_CLOTH_DISABLE_COLLISION,    descB.flags.NX_SF_CLOTH_DISABLE_COLLISION,    assignType);
	assignFlags(descA.shapeFlags, NX_SF_CLOTH_TWOWAY,               descB.flags.NX_SF_CLOTH_TWOWAY,               assignType);
	assignFlags(descA.shapeFlags, NX_SF_SOFTBODY_DRAIN,             descB.flags.NX_SF_SOFTBODY_DRAIN,             assignType);
	assignFlags(descA.shapeFlags, NX_SF_SOFTBODY_DISABLE_COLLISION, descB.flags.NX_SF_SOFTBODY_DISABLE_COLLISION, assignType);
	assignFlags(descA.shapeFlags, NX_SF_SOFTBODY_TWOWAY,            descB.flags.NX_SF_SOFTBODY_TWOWAY,            assignType);

	assign(descA.group            , descB.collisionGroup,     assignType);
	assign(descA.materialIndex    , descB.materialIndex,      assignType);
	assign(descA.density          , descB.density,            assignType);
	assign(descA.skinWidth        , descB.skinWidth,          assignType);
	assign(descA.groupsMask.bits0 , descB.groupsMask.bits0,   assignType);
	assign(descA.groupsMask.bits1 , descB.groupsMask.bits1,   assignType);
	assign(descA.groupsMask.bits2 , descB.groupsMask.bits2,   assignType);
	assign(descA.groupsMask.bits3 , descB.groupsMask.bits3,   assignType);
	assignPtr<void*,size_t>(descA.userData,   descB.userData, assignType);
	assignPtr<const char*,size_t>(descA.name, descB.name,     assignType);
}

#elif NX_SDK_VERSION_MAJOR == 3

void assign(PhysX3DescTemplate& descA,
			DestructibleActorParamNS::P3ActorDescTemplate_Type& descB0,
			DestructibleActorParamNS::P3BodyDescTemplate_Type&  descB1,
			DestructibleActorParamNS::P3ShapeDescTemplate_Type& descB2,
			AssignType assignType = Default)
{
	// actor
	assign(descA.dominanceGroup    ,      descB0.dominanceGroup,     assignType);
	assign(descA.ownerClient       ,      descB0.ownerClient,        assignType);
	assign(descA.clientBehaviorBits,      descB0.clientBehaviorBits, assignType);
	
	assignFlags(descA.actorFlags,         PxActorFlag::eVISUALIZATION,                     descB0.flags.eVISUALIZATION,                                assignType);
	assignFlags(descA.actorFlags,         PxActorFlag::eDISABLE_GRAVITY,                   descB0.flags.eDISABLE_GRAVITY,                              assignType);
	assignFlags(descA.actorFlags,         PxActorFlag::eSEND_SLEEP_NOTIFIES,               descB0.flags.eSEND_SLEEP_NOTIFIES,                          assignType);

	assignFlags(descA.contactReportFlags, PxPairFlag::eSOLVE_CONTACT,                      descB0.contactReportFlags.eSOLVE_CONTACT,                   assignType);
	assignFlags(descA.contactReportFlags, PxPairFlag::eMODIFY_CONTACTS,                    descB0.contactReportFlags.eMODIFY_CONTACTS,                 assignType);
	assignFlags(descA.contactReportFlags, PxPairFlag::eNOTIFY_TOUCH_FOUND,                 descB0.contactReportFlags.eNOTIFY_TOUCH_FOUND,              assignType);
	assignFlags(descA.contactReportFlags, PxPairFlag::eNOTIFY_TOUCH_PERSISTS,              descB0.contactReportFlags.eNOTIFY_TOUCH_PERSISTS,           assignType);
	assignFlags(descA.contactReportFlags, PxPairFlag::eNOTIFY_TOUCH_LOST,                  descB0.contactReportFlags.eNOTIFY_TOUCH_LOST,               assignType);
	assignFlags(descA.contactReportFlags, PxPairFlag::eNOTIFY_THRESHOLD_FORCE_FOUND,       descB0.contactReportFlags.eNOTIFY_THRESHOLD_FORCE_FOUND,    assignType);
	assignFlags(descA.contactReportFlags, PxPairFlag::eNOTIFY_THRESHOLD_FORCE_PERSISTS,    descB0.contactReportFlags.eNOTIFY_THRESHOLD_FORCE_PERSISTS, assignType);
	assignFlags(descA.contactReportFlags, PxPairFlag::eNOTIFY_THRESHOLD_FORCE_LOST,        descB0.contactReportFlags.eNOTIFY_THRESHOLD_FORCE_LOST,     assignType);
	assignFlags(descA.contactReportFlags, PxPairFlag::eNOTIFY_CONTACT_POINTS,              descB0.contactReportFlags.eNOTIFY_CONTACT_POINTS,           assignType);
	assignFlags(descA.contactReportFlags, PxPairFlag::eDETECT_CCD_CONTACT,                 descB0.contactReportFlags.eDETECT_CCD_CONTACT,              assignType);
	assignFlags(descA.contactReportFlags, PxPairFlag::eCONTACT_DEFAULT,                    descB0.contactReportFlags.eCONTACT_DEFAULT,                 assignType);
	assignFlags(descA.contactReportFlags, PxPairFlag::eTRIGGER_DEFAULT,                    descB0.contactReportFlags.eTRIGGER_DEFAULT,                 assignType);
	assignPtr<void*,size_t>(descA.userData,   descB0.userData, assignType);
	assignPtr<const char*,size_t>(descA.name, descB0.name, assignType);

	// body
	assignFlags(descA.bodyFlags, PxRigidDynamicFlag::eKINEMATIC, descB1.flags.eKINEMATIC, assignType);
	assignFlags(descA.bodyFlags, PxRigidDynamicFlag::eENABLE_CCD, descB1.flags.eENABLE_CCD, assignType);
	assign(descA.density                , descB1.density,                assignType);
	assign(descA.sleepThreshold         , descB1.sleepThreshold,         assignType);
	assign(descA.wakeUpCounter          , descB1.wakeUpCounter,          assignType);
	assign(descA.linearDamping          , descB1.linearDamping,          assignType);
	assign(descA.angularDamping         , descB1.angularDamping,         assignType);
	assign(descA.maxAngularVelocity     , descB1.maxAngularVelocity,     assignType);
	assign(descA.solverIterationCount   , descB1.solverIterationCount,   assignType);
	assign(descA.velocityIterationCount , descB1.velocityIterationCount, assignType);
	assign(descA.contactReportThreshold , descB1.contactReportThreshold, assignType);
	assign(descA.sleepLinearVelocity    , descB1.sleepLinearVelocity,    assignType);

	// shape
	assignFlags(descA.shapeFlags, PxShapeFlag::eSIMULATION_SHAPE,  descB2.flags.eSIMULATION_SHAPE,  assignType);
	assignFlags(descA.shapeFlags, PxShapeFlag::eSCENE_QUERY_SHAPE, descB2.flags.eSCENE_QUERY_SHAPE, assignType);
	assignFlags(descA.shapeFlags, PxShapeFlag::eTRIGGER_SHAPE,     descB2.flags.eTRIGGER_SHAPE,     assignType);
	assignFlags(descA.shapeFlags, PxShapeFlag::eVISUALIZATION,     descB2.flags.eVISUALIZATION,     assignType);
	assignFlags(descA.shapeFlags, PxShapeFlag::ePARTICLE_DRAIN,    descB2.flags.ePARTICLE_DRAIN,    assignType);
	
	// TODO: Remove this from the NxParameterized destructible actor, shift it to the BODY descriptor?
	//assignFlags(descA.shapeFlags, PxShapeFlag::eUSE_SWEPT_BOUNDS,  descB2.flags.eUSE_SWEPT_BOUNDS, assignType);
	
	assign(descA.simulationFilterData.word0 , descB2.simulationFilterData.word0, assignType);
	assign(descA.simulationFilterData.word1 , descB2.simulationFilterData.word1, assignType);
	assign(descA.simulationFilterData.word2 , descB2.simulationFilterData.word2, assignType);
	assign(descA.simulationFilterData.word3 , descB2.simulationFilterData.word3, assignType);
	assign(descA.queryFilterData.word0      , descB2.queryFilterData.word0,      assignType);
	assign(descA.queryFilterData.word1      , descB2.queryFilterData.word1,      assignType);
	assign(descA.queryFilterData.word2      , descB2.queryFilterData.word2,      assignType);
	assign(descA.queryFilterData.word3      , descB2.queryFilterData.word3,      assignType);
	assign(descA.contactOffset              , descB2.contactOffset,              assignType);
	assign(descA.restOffset                 , descB2.restOffset,                 assignType);
	assignPtr<void*,size_t>(descA.shapeUserData   , descB2.userData,             assignType);
	assignPtr<const char*, size_t>(descA.shapeName, descB2.name,                 assignType);

	if (BtoA == assignType)
	{
		PxMaterial* material = NULL;
		if (descB2.material)
			material = (PxMaterial*)descB2.material;
		else
		{
			// Get the first material as a fallback
			if (NxGetApexSDK()->getPhysXSDK()->getNbMaterials())
				NxGetApexSDK()->getPhysXSDK()->getMaterials(&material, 1, 0);
		}
		if (NULL != material)
		{
			descA.materials.pushBack(material);
		}
	}
	else
		descB2.material = descA.materials.size() > 0 ? (size_t)descA.materials[0] : (physx::PxU64)0;
}

PX_INLINE void deserialize(PhysX3DescTemplate& descA,
                           const DestructibleActorParamNS::P3ActorDescTemplate_Type& descB0,
                           const DestructibleActorParamNS::P3BodyDescTemplate_Type&  descB1,
                           const DestructibleActorParamNS::P3ShapeDescTemplate_Type& descB2)
{
	assign(descA,
	       const_cast<DestructibleActorParamNS::P3ActorDescTemplate_Type&>(descB0),
	       const_cast<DestructibleActorParamNS::P3BodyDescTemplate_Type&>(descB1),
	       const_cast<DestructibleActorParamNS::P3ShapeDescTemplate_Type&>(descB2), BtoA);
}

PX_INLINE void serialize(const PhysX3DescTemplate& descA,
                         DestructibleActorParamNS::P3ActorDescTemplate_Type& descB0,
                         DestructibleActorParamNS::P3BodyDescTemplate_Type&  descB1,
                         DestructibleActorParamNS::P3ShapeDescTemplate_Type& descB2)
{
	assign(const_cast<PhysX3DescTemplate&>(descA), descB0, descB1, descB2, AtoB);
}

#endif

///////////////////////////////////////////////////////////////////////////

PX_INLINE void assign(NxDestructibleDepthParameters& paramA, 
                      DestructibleActorParamNS::DestructibleDepthParameters_Type& paramB, 
                      AssignType assignType = Default)
{
	assignFlags(paramA.flags, NxDestructibleDepthParametersFlag::OVERRIDE_IMPACT_DAMAGE,       paramB.OVERRIDE_IMPACT_DAMAGE,       assignType);
	assignFlags(paramA.flags, NxDestructibleDepthParametersFlag::OVERRIDE_IMPACT_DAMAGE_VALUE, paramB.OVERRIDE_IMPACT_DAMAGE_VALUE, assignType);
	assignFlags(paramA.flags, NxDestructibleDepthParametersFlag::IGNORE_POSE_UPDATES,          paramB.IGNORE_POSE_UPDATES,          assignType);
	assignFlags(paramA.flags, NxDestructibleDepthParametersFlag::IGNORE_RAYCAST_CALLBACKS,     paramB.IGNORE_RAYCAST_CALLBACKS,     assignType);
	assignFlags(paramA.flags, NxDestructibleDepthParametersFlag::IGNORE_CONTACT_CALLBACKS,     paramB.IGNORE_CONTACT_CALLBACKS,     assignType);
	assignFlags(paramA.flags, NxDestructibleDepthParametersFlag::USER_FLAG_0,                  paramB.USER_FLAG_0,                  assignType);
	assignFlags(paramA.flags, NxDestructibleDepthParametersFlag::USER_FLAG_1,                  paramB.USER_FLAG_1,                  assignType);
	assignFlags(paramA.flags, NxDestructibleDepthParametersFlag::USER_FLAG_2,                  paramB.USER_FLAG_2,                  assignType);
	assignFlags(paramA.flags, NxDestructibleDepthParametersFlag::USER_FLAG_3,                  paramB.USER_FLAG_3,                  assignType);
}

///////////////////////////////////////////////////////////////////////////

PX_INLINE void assign(NxDestructibleParameters& paramA,
                      DestructibleActorParamNS::DestructibleParameters_Type& paramB,
                      AssignType assignType = Default)
{
	assign(paramA.damageCap                             , paramB.damageCap,                     assignType);
	assign(paramA.debrisDepth                           , paramB.debrisDepth,                   assignType);
	assign(paramA.debrisLifetimeMax                     , paramB.debrisLifetimeMax,             assignType);
	assign(paramA.debrisLifetimeMin                     , paramB.debrisLifetimeMin,             assignType);
	assign(paramA.debrisMaxSeparationMax                , paramB.debrisMaxSeparationMax,        assignType);
	assign(paramA.debrisMaxSeparationMin                , paramB.debrisMaxSeparationMin,        assignType);
	assign(paramA.essentialDepth                        , paramB.essentialDepth,                assignType);
	assign(paramA.forceToDamage                         , paramB.forceToDamage,                 assignType);
	assign(paramA.fractureImpulseScale                  , paramB.fractureImpulseScale,          assignType);
	assign(paramA.damageDepthLimit                      , paramB.damageDepthLimit,              assignType);
	assign(paramA.impactVelocityThreshold               , paramB.impactVelocityThreshold,       assignType);
	assign(paramA.maxChunkSpeed                         , paramB.maxChunkSpeed,                 assignType);
	assign(paramA.minimumFractureDepth                  , paramB.minimumFractureDepth,          assignType);
	assign(paramA.impactDamageDefaultDepth              , paramB.impactDamageDefaultDepth,      assignType);
	assign(paramA.debrisDestructionProbability          , paramB.debrisDestructionProbability,  assignType);
	assign(paramA.validBounds                           , paramB.validBounds,                   assignType);
	assign(paramA.dynamicChunksDominanceGroup           , paramB.dynamicChunkDominanceGroup,               assignType);
	assign(paramA.useDynamicChunksGroupsMask            , paramB.dynamicChunksGroupsMask.useGroupsMask,    assignType);
#if NX_SDK_VERSION_MAJOR == 2
	assign(paramA.dynamicChunksGroupsMask.bits0         , paramB.dynamicChunksGroupsMask.bits0,            assignType);
	assign(paramA.dynamicChunksGroupsMask.bits1         , paramB.dynamicChunksGroupsMask.bits1,            assignType);
	assign(paramA.dynamicChunksGroupsMask.bits2         , paramB.dynamicChunksGroupsMask.bits2,            assignType);
	assign(paramA.dynamicChunksGroupsMask.bits3         , paramB.dynamicChunksGroupsMask.bits3,            assignType);
#elif NX_SDK_VERSION_MAJOR == 3
	assign(paramA.dynamicChunksFilterData.word0         , paramB.dynamicChunksGroupsMask.bits0,            assignType);
	assign(paramA.dynamicChunksFilterData.word1         , paramB.dynamicChunksGroupsMask.bits1,            assignType);
	assign(paramA.dynamicChunksFilterData.word2         , paramB.dynamicChunksGroupsMask.bits2,            assignType);
	assign(paramA.dynamicChunksFilterData.word3         , paramB.dynamicChunksGroupsMask.bits3,            assignType);
	assign(paramA.supportStrength						, paramB.supportStrength,						   assignType);
#endif
	assign(paramA.legacyChunkBoundsTestSetting			, paramB.legacyChunkBoundsTestSetting,				assignType);
	assign(paramA.legacyDamageRadiusSpreadSetting		, paramB.legacyDamageRadiusSpreadSetting,			assignType);

	assignFlags(paramA.flags, NxDestructibleParametersFlag::ACCUMULATE_DAMAGE,            paramB.flags.ACCUMULATE_DAMAGE,       assignType);
	assignFlags(paramA.flags, NxDestructibleParametersFlag::DEBRIS_TIMEOUT,               paramB.flags.DEBRIS_TIMEOUT,          assignType);
	assignFlags(paramA.flags, NxDestructibleParametersFlag::DEBRIS_MAX_SEPARATION,        paramB.flags.DEBRIS_MAX_SEPARATION,   assignType);
	assignFlags(paramA.flags, NxDestructibleParametersFlag::CRUMBLE_SMALLEST_CHUNKS,      paramB.flags.CRUMBLE_SMALLEST_CHUNKS, assignType);
	assignFlags(paramA.flags, NxDestructibleParametersFlag::ACCURATE_RAYCASTS,            paramB.flags.ACCURATE_RAYCASTS,       assignType);
	assignFlags(paramA.flags, NxDestructibleParametersFlag::USE_VALID_BOUNDS,             paramB.flags.USE_VALID_BOUNDS,        assignType);
	assignFlags(paramA.flags, NxDestructibleParametersFlag::CRUMBLE_VIA_RUNTIME_FRACTURE, paramB.flags.CRUMBLE_VIA_RUNTIME_FRACTURE, assignType);

	// RT Fracture assign
	assign(paramA.rtFractureParameters.sheetFracture,			paramB.runtimeFracture.sheetFracture,			assignType);
	assign(paramA.rtFractureParameters.depthLimit,				paramB.runtimeFracture.depthLimit,				assignType);
	assign(paramA.rtFractureParameters.destroyIfAtDepthLimit,	paramB.runtimeFracture.destroyIfAtDepthLimit,	assignType);
	assign(paramA.rtFractureParameters.minConvexSize,			paramB.runtimeFracture.minConvexSize,			assignType);
	assign(paramA.rtFractureParameters.impulseScale,			paramB.runtimeFracture.impulseScale,			assignType);
	assign(paramA.rtFractureParameters.glass.numSectors,		paramB.runtimeFracture.glass.numSectors,		assignType);
	assign(paramA.rtFractureParameters.glass.sectorRand,		paramB.runtimeFracture.glass.sectorRand,		assignType);
	assign(paramA.rtFractureParameters.glass.firstSegmentSize,	paramB.runtimeFracture.glass.firstSegmentSize,	assignType);
	assign(paramA.rtFractureParameters.glass.segmentScale,		paramB.runtimeFracture.glass.segmentScale,		assignType);
	assign(paramA.rtFractureParameters.glass.segmentRand,		paramB.runtimeFracture.glass.segmentRand,		assignType);
	assign(paramA.rtFractureParameters.attachment.posX,			paramB.runtimeFracture.attachment.posX,			assignType);
	assign(paramA.rtFractureParameters.attachment.negX,			paramB.runtimeFracture.attachment.negX,			assignType);
	assign(paramA.rtFractureParameters.attachment.posY,			paramB.runtimeFracture.attachment.posY,			assignType);
	assign(paramA.rtFractureParameters.attachment.negY,			paramB.runtimeFracture.attachment.negY,			assignType);
	assign(paramA.rtFractureParameters.attachment.posZ,			paramB.runtimeFracture.attachment.posZ,			assignType);
	assign(paramA.rtFractureParameters.attachment.negZ,			paramB.runtimeFracture.attachment.negZ,			assignType);
}

///////////////////////////////////////////////////////////////////////////

template<typename A, typename B>
PX_INLINE void deserialize(A& paramA, const B& paramB)
{
	assign(paramA, const_cast<B&>(paramB), BtoA);
}

template<typename A, typename B>
PX_INLINE void serialize(const A& paramA, B& paramB)
{
	assign(const_cast<A&>(paramA), paramB, AtoB);
}

///////////////////////////////////////////////////////////////////////////

PX_INLINE void serialize(const NxDestructibleParameters& nxIn,
						 NxParameterized::Interface& nxOut)
{
	if (isType(&nxOut, NxDestructibleParameterizedType::Params))
	{
		DestructibleActorParam& nxOutParams = static_cast<DestructibleActorParam&>(nxOut);
		NxParameterized::Handle handle(nxOutParams);
		VERIFY_PARAM(nxOutParams.getParameterHandle("depthParameters", handle));
		nxOutParams.resizeArray(handle, (physx::PxI32)nxIn.depthParametersCount);

		serialize(nxIn, nxOutParams.destructibleParameters);

		PX_ASSERT(nxOutParams.depthParameters.arraySizes[0] == (physx::PxI32)nxIn.depthParametersCount);
		for (physx::PxU32 i = 0; i < nxIn.depthParametersCount; ++i)
		{
			serialize(nxIn.depthParameters[i], nxOutParams.depthParameters.buf[i]);
		}
	}
}

PX_INLINE void deserialize(const DestructibleActorParamNS::DestructibleParameters_Type& nxIn, 
						   const DestructibleActorParamNS::DestructibleDepthParameters_DynamicArray1D_Type& nxIn2,
						   NxDestructibleParameters& nxOut)
{
	deserialize(nxOut, const_cast<DestructibleActorParamNS::DestructibleParameters_Type&>(nxIn));
	nxOut.depthParametersCount = (physx::PxU32)nxIn2.arraySizes[0];
	for (physx::PxU32 depth = 0; depth < nxOut.depthParametersCount; ++depth)
	{
		deserialize(nxOut.depthParameters[depth], nxIn2.buf[depth]);
	}
}

PX_INLINE void serialize(const DestructibleActor& actor, 
						 const DestructibleStructure::Chunk* nxIn,
						 const physx::PxU32 nxInCount,
						 DestructibleActorChunks& nxOut)
{
	if (nxInCount <= 0)
		return;

	NxParameterized::Handle handle(nxOut);
	VERIFY_PARAM(nxOut.getParameterHandle("data", handle));
	nxOut.resizeArray(handle, (physx::PxI32)nxInCount);
	PX_ASSERT((physx::PxI32)nxInCount == nxOut.data.arraySizes[0]);
	
	for (physx::PxU32 i = 0; i < nxInCount; ++i)
	{
		nxOut.data.buf[i].state       = nxIn[i].state;
		nxOut.data.buf[i].flags       = nxIn[i].flags;
		nxOut.data.buf[i].islandID    = nxIn[i].islandID;
		nxOut.data.buf[i].damage      = nxIn[i].damage;
		nxOut.data.buf[i].localOffset = nxIn[i].localOffset;
		nxOut.data.buf[i].localSphere = physx::PxVec4(PXFROMNXVEC3(nxIn[i].localSphere.center), nxIn[i].localSphere.radius);
		nxOut.data.buf[i].shapesCount = (nxIn[i].isDestroyed() || !(nxIn[i].state & ChunkVisible)) ? 0 : nxIn[i].getShapeCount();
		
		if (nxIn[i].visibleAncestorIndex != (physx::PxI32)DestructibleStructure::InvalidChunkIndex)
			nxOut.data.buf[i].visibleAncestorIndex = nxIn[i].visibleAncestorIndex - (physx::PxI32)actor.getFirstChunkIndex();
		else
			nxOut.data.buf[i].visibleAncestorIndex = (physx::PxI32)DestructibleStructure::InvalidChunkIndex;

		if (!nxIn[i].isDestroyed())
		{
			// BRG - shouldn't this state be obtained from nxIn[i].state?  In any case, must decouple this now since we have "dormant" kinematic chunks
			// which are chunks that have been freed but turned kinematic for "hard" sleeping
//			const bool chunkDynamic = !actor.getChunkActor(i)->readBodyFlag(NX_BF_KINEMATIC);
//			assignFlag(nxOut.data.buf[i].state, ChunkDynamic, chunkDynamic);

			nxOut.data.buf[i].linearVelocity   = actor.getChunkLinearVelocity(i);
			nxOut.data.buf[i].angularVelocity  = actor.getChunkAngularVelocity(i);
			nxOut.data.buf[i].globalPose       = PxTransform(actor.getChunkPose(i));
			nxOut.data.buf[i].globalPose.q.normalize();
		}
	}
}

PX_INLINE void deserializeChunkData(const DestructibleActorChunksNS::Chunk_Type& nxIn, DestructibleStructure::Chunk& nxOut)
{
	nxOut.state                = nxIn.state;
	nxOut.flags                = nxIn.flags;
	nxOut.damage               = nxIn.damage;
	nxOut.localOffset          = nxIn.localOffset;
	nxOut.localSphere          = NxSphere(NXFROMPXVEC3(nxIn.localSphere.getXYZ()), nxIn.localSphere.w);
	nxOut.islandID             = nxIn.islandID;
	nxOut.clearShapes();
	// Poses and velocities are restored later as chunks are actually created
}

///////////////////////////////////////////////////////////////////////////

}
}
} // end namespace physx::apex

#endif

#endif
