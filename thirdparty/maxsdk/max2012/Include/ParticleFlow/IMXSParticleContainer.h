/*! \file IMXSParticleContainer.h
    \brief An interface class to maxscript version of ParticleContainer.
				 The interface is given to script actions.
*/
/**********************************************************************
*<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 06-04-02

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "IParticleContainer.h"
#include "..\point3.h"
#include "..\quat.h"

// interface ID
#define MXSPARTICLECONTAINER_INTERFACE Interface_ID(0x74f93c10, 0x1eb34600) 

#define GetMXSParticleContainerInterface(obj) ((IMXSParticleContainer*)obj->GetInterface(MXSPARTICLECONTAINER_INTERFACE)) 

class IMXSParticleContainer : public FPMixinInterface
{
public: 

// function IDs
	enum {	kGetParticleContainer,
			kGetParticleSystem,
			kGetParticleSystemNode,
			kGetInitActions,
			kGetInitActionNodes,
			kGetTimeStart,
			kGetTimeEnd,
			kSetTimeEnd,
			kSetTimeEndPrecise,
			kGetActionNode,
			kGetIntegrator,

			kRandSign,
			kRand01,
			kRand11,
			kRand55,
			kRand0X,
			kRandSpherePoint,
			kRandDivergeVector,

			kGetUseTime,
			kSetUseTime,
			kGetUseAge,
			kSetUseAge,
			kGetUseLifespan,
			kSetUseLifespan,
			kGetUseEventTime,
			kSetUseEventTime,
			kGetUsePosition,
			kSetUsePosition,
			kGetUseSpeed,
			kSetUseSpeed,
			kGetUseAcceleration,
			kSetUseAcceleration,
			kGetUseOrientation,
			kSetUseOrientation,
			kGetUseSpin,
			kSetUseSpin,
			kGetUseScale,
			kSetUseScale,
			kGetUseTM,
			kSetUseTM,
			kGetUseSelected,
			kSetUseSelected,
			kGetUseShape,
			kSetUseShape,
			kGetUseMtlIndex,
			kSetUseMtlIndex,
			kGetUseMapping,
			kSetUseMapping,
			kGetUseInt,
			kSetUseInt,
			kGetUseFloat,
			kSetUseFloat,
			kGetUseVector,
			kSetUseVector,
			kGetUseMatrix,
			kSetUseMatrix,

			kNumParticles,

			kAddParticle,
			kAddParticles,
			kDeleteParticle,
			kDeleteParticles,

			kGetParticleBornIndex,
			kHasParticleBornIndex,

			kGetCurrentParticleIndex,
			kGetCurrentParticleBornIndex,
			kSetCurrentParticleIndex,
			kSetCurrentParticleBornIndex,

			kIsParticleNewByIndex,
			kIsParticleNewByBornIndex,
			kIsParticleNew,

			kGetParticleTimeByIndex,
			kGetParticleTimeByBornIndex,
			kSetParticleTimeByIndex,
			kSetParticleTimePreciseByIndex,
			kSetParticleTimeByBornIndex,
			kSetParticleTimePreciseByBornIndex,
			kGetParticleTime,
			kSetParticleTime,

			kGetParticleAgeByIndex,
			kGetParticleAgeByBornIndex,
			kSetParticleAgeByIndex,
			kSetParticleAgeByBornIndex,
			kGetParticleAge,
			kSetParticleAge,

			kGetParticleLifespanByIndex,
			kGetParticleLifespanByBornIndex,
			kSetParticleLifespanByIndex,
			kSetParticleLifespanByBornIndex,
			kGetParticleLifespan,
			kSetParticleLifespan,

			kGetParticleEventTimeByIndex,
			kGetParticleEventTimeByBornIndex,
			kSetParticleEventTimeByIndex,
			kSetParticleEventTimeByBornIndex,
			kGetParticleEventTime,
			kSetParticleEventTime,

			kGetParticlePositionByIndex,
			kGetParticlePositionByBornIndex,
			kSetParticlePositionByIndex,
			kSetParticlePositionByBornIndex,
			kGetParticlePosition,
			kSetParticlePosition,

			kGetParticleSpeedByIndex,
			kGetParticleSpeedByBornIndex,
			kSetParticleSpeedByIndex,
			kSetParticleSpeedByBornIndex,
			kGetParticleSpeed,
			kSetParticleSpeed,

			kGetParticleAccelerationByIndex,
			kGetParticleAccelerationByBornIndex,
			kSetParticleAccelerationByIndex,
			kSetParticleAccelerationByBornIndex,
			kGetParticleAcceleration,
			kSetParticleAcceleration,

			kGetParticleOrientationByIndex,
			kGetParticleOrientationByBornIndex,
			kSetParticleOrientationByIndex,
			kSetParticleOrientationByBornIndex,
			kGetParticleOrientation,
			kSetParticleOrientation,

			kGetParticleSpinByIndex,
			kGetParticleSpinByBornIndex,
			kSetParticleSpinByIndex,
			kSetParticleSpinByBornIndex,
			kGetParticleSpin,
			kSetParticleSpin,

			kGetParticleScaleByIndex,
			kGetParticleScaleByBornIndex,
			kSetParticleScaleByIndex,
			kSetParticleScaleByBornIndex,
			kGetParticleScale,
			kSetParticleScale,
			kGetParticleScaleXYZByIndex,
			kGetParticleScaleXYZByBornIndex,
			kSetParticleScaleXYZByIndex,
			kSetParticleScaleXYZByBornIndex,
			kGetParticleScaleXYZ,
			kSetParticleScaleXYZ,

			kGetParticleTMByIndex,
			kGetParticleTMByBornIndex,
			kSetParticleTMByIndex,
			kSetParticleTMByBornIndex,
			kGetParticleTM,
			kSetParticleTM,

			kGetParticleSelectedByIndex,
			kGetParticleSelectedByBornIndex,
			kSetParticleSelectedByIndex,
			kSetParticleSelectedByBornIndex,
			kGetParticleSelected,
			kSetParticleSelected,

			kGetParticleShapeByIndex,
			kGetParticleShapeByBornIndex,
			kSetParticleShapeByIndex,
			kSetParticleShapeByBornIndex,
			kGetParticleShape,
			kSetParticleShape,
			kSetGlobalParticleShape,

			kSetParticleMtlIndexByIndex,
			kSetParticleMtlIndexByBornIndex,
			kSetGlobalParticleMtlIndex,

			kSetParticleMappingByIndex,
			kSetParticleMappingByBornIndex,
			kSetGlobalParticleMapping,

			kGetParticleIntByIndex,
			kGetParticleIntByBornIndex,
			kSetParticleIntByIndex,
			kSetParticleIntByBornIndex,
			kGetParticleInt,
			kSetParticleInt,
			
			kGetParticleFloatByIndex,
			kGetParticleFloatByBornIndex,
			kSetParticleFloatByIndex,
			kSetParticleFloatByBornIndex,
			kGetParticleFloat,
			kSetParticleFloat,

			kGetParticleVectorByIndex,
			kGetParticleVectorByBornIndex,
			kSetParticleVectorByIndex,
			kSetParticleVectorByBornIndex,
			kGetParticleVector,
			kSetParticleVector,

			kGetParticleMatrixByIndex,
			kGetParticleMatrixByBornIndex,
			kSetParticleMatrixByIndex,
			kSetParticleMatrixByBornIndex,
			kGetParticleMatrix,
			kSetParticleMatrix,

			kSetParticleTestStatusByIndex,
			kSetParticleTestStatusByBornIndex,
			kGetParticleTestStatus,
			kSetParticleTestStatus,
			kSetGlobalTestStatus,

			kSetParticleTestTimeByIndex,
			kSetParticleTestTimePreciseByIndex,
			kSetParticleTestTimeByBornIndex,
			kSetParticleTestTimePreciseByBornIndex,
			kGetParticleTestTime,
			kSetParticleTestTime,
			kSetParticleTestTimePrecise,
			kSetGlobalTestTime,
			kSetGlobalTestTimePrecise,
	}; 

// Function Map for Function Publish System 
//***********************************
	BEGIN_FUNCTION_MAP

	FN_0(kGetParticleContainer,	TYPE_IOBJECT,	GetParticleContainer);
	FN_0(kGetParticleSystem,	TYPE_OBJECT,	GetParticleSystem);
	FN_0(kGetParticleSystemNode,TYPE_INODE,		GetParticleSystemNode);
	FN_0(kGetInitActions,		TYPE_OBJECT_TAB_BR,GetInitActions);
	FN_0(kGetInitActionNodes,	TYPE_INODE_TAB_BR, GetInitActionNodes);
	FN_0(kGetTimeStart,			TYPE_TIMEVALUE, GetTimeStart);
	FN_0(kGetTimeEnd,			TYPE_TIMEVALUE, GetTimeEnd);
	VFN_1(kSetTimeEnd,							SetTimeEnd, TYPE_TIMEVALUE);
	VFN_2(kSetTimeEndPrecise,					SetTimeEnd, TYPE_TIMEVALUE, TYPE_FLOAT);
	FN_0(kGetActionNode,		TYPE_INODE,		GetActionNode);
	FN_0(kGetIntegrator,		TYPE_INTERFACE,	GetIntegrator);

	FN_0(kRandSign,				TYPE_INT,		RandSign);
	FN_0(kRand01,				TYPE_FLOAT,		Rand01);
	FN_0(kRand11,				TYPE_FLOAT,		Rand11);
	FN_0(kRand55,				TYPE_FLOAT,		Rand55);
	FN_1(kRand0X,				TYPE_INT,		Rand0X, TYPE_INT);
	FN_0(kRandSpherePoint,		TYPE_POINT3,	RandSpherePoint);
	FN_2(kRandDivergeVector,	TYPE_POINT3,	RandDivergeVector, TYPE_POINT3, TYPE_ANGLE);

	PROP_FNS( kGetUseTime,			GetUseTime,			kSetUseTime,		SetUseTime,			TYPE_bool);
	PROP_FNS( kGetUseAge,			GetUseAge,			kSetUseAge,			SetUseAge,			TYPE_bool);
	PROP_FNS( kGetUseLifespan,		GetUseLifespan,		kSetUseLifespan,	SetUseLifespan,		TYPE_bool);
	PROP_FNS( kGetUseEventTime,		GetUseEventTime,	kSetUseEventTime,	SetUseEventTime,	TYPE_bool);
	PROP_FNS( kGetUsePosition,		GetUsePosition,		kSetUsePosition,	SetUsePosition,		TYPE_bool);
	PROP_FNS( kGetUseSpeed,			GetUseSpeed,		kSetUseSpeed,		SetUseSpeed,		TYPE_bool);
	PROP_FNS( kGetUseAcceleration,	GetUseAcceleration,	kSetUseAcceleration,SetUseAcceleration,	TYPE_bool);
	PROP_FNS( kGetUseOrientation,	GetUseOrientation,	kSetUseOrientation,	SetUseOrientation,	TYPE_bool);
	PROP_FNS( kGetUseSpin,			GetUseSpin,			kSetUseSpin,		SetUseSpin,			TYPE_bool);
	PROP_FNS( kGetUseScale,			GetUseScale,		kSetUseScale,		SetUseScale,		TYPE_bool);
	PROP_FNS( kGetUseTM,			GetUseTM,			kSetUseTM,			SetUseTM,			TYPE_bool);
	PROP_FNS( kGetUseSelected,		GetUseSelected,		kSetUseSelected,	SetUseSelected,		TYPE_bool);
	PROP_FNS( kGetUseShape,			GetUseShape,		kSetUseShape,		SetUseShape,		TYPE_bool);
	PROP_FNS( kGetUseMtlIndex,		GetUseMtlIndex,		kSetUseMtlIndex,	SetUseMtlIndex,		TYPE_bool);
	PROP_FNS( kGetUseMapping,		GetUseMapping,		kSetUseMapping,		SetUseMapping,		TYPE_bool);
	PROP_FNS( kGetUseInt,			GetUseInt,			kSetUseInt,			SetUseInt,			TYPE_bool);
	PROP_FNS( kGetUseFloat,			GetUseFloat,		kSetUseFloat,		SetUseFloat,		TYPE_bool);
	PROP_FNS( kGetUseVector,		GetUseVector,		kSetUseVector,		SetUseVector,		TYPE_bool);
	PROP_FNS( kGetUseMatrix,		GetUseMatrix,		kSetUseMatrix,		SetUseMatrix,		TYPE_bool);

	FN_0(kNumParticles,			TYPE_INT,		NumParticles);

	FN_0(kAddParticle,			TYPE_bool,		AddParticle);
	FN_1(kAddParticles,			TYPE_bool,		AddParticles,		TYPE_INT);
	FN_1(kDeleteParticle,		TYPE_bool,		DeleteParticle,		TYPE_INDEX);
	FN_2(kDeleteParticles,		TYPE_bool,		DeleteParticles,	TYPE_INDEX, TYPE_INT);

	FN_1(kGetParticleBornIndex,	TYPE_INDEX,	GetParticleBornIndex,	TYPE_INDEX);
	FN_2(kHasParticleBornIndex,	TYPE_bool,	HasParticleBornIndex,	TYPE_INDEX, TYPE_INDEX_BR);

	PROP_FNS(kGetCurrentParticleIndex,		GetCurrentParticleIndex,		kSetCurrentParticleIndex,		SetCurrentParticleIndex,	TYPE_INDEX);
	PROP_FNS(kGetCurrentParticleBornIndex,	GetCurrentParticleBornIndex,	kSetCurrentParticleBornIndex,	SetCurrentParticleBornIndex,TYPE_INDEX);

	FN_1(kIsParticleNewByIndex,		TYPE_bool,	IsParticleNewByIndex,		TYPE_INDEX);
	FN_1(kIsParticleNewByBornIndex,	TYPE_bool,	IsParticleNewByBornIndex,	TYPE_INDEX);
	RO_PROP_FN(kIsParticleNew,		IsParticleNew, TYPE_bool);

	FN_1(kGetParticleTimeByIndex,		TYPE_TIMEVALUE,	GetParticleTimeByIndex,	TYPE_INDEX);
	FN_1(kGetParticleTimeByBornIndex,	TYPE_TIMEVALUE,	GetParticleTimeByBornIndex,TYPE_INDEX);
	VFN_2(kSetParticleTimeByIndex,						SetParticleTimeByIndex,	TYPE_INDEX, TYPE_TIMEVALUE);
	VFN_3(kSetParticleTimePreciseByIndex,				SetParticleTimeByIndex,	TYPE_INDEX, TYPE_TIMEVALUE, TYPE_FLOAT);
	VFN_2(kSetParticleTimeByBornIndex,					SetParticleTimeByBornIndex,TYPE_INDEX, TYPE_TIMEVALUE);
	VFN_3(kSetParticleTimePreciseByBornIndex,			SetParticleTimeByBornIndex,TYPE_INDEX, TYPE_TIMEVALUE, TYPE_FLOAT);
	PROP_FNS(kGetParticleTime, GetParticleTime, kSetParticleTime, SetParticleTime,	TYPE_TIMEVALUE);
	
	FN_1(kGetParticleAgeByIndex,		TYPE_TIMEVALUE,	GetParticleAgeByIndex,			TYPE_INDEX);
	FN_1(kGetParticleAgeByBornIndex,	TYPE_TIMEVALUE,	GetParticleAgeByBornIndex,		TYPE_INDEX);
	VFN_2(kSetParticleAgeByIndex,						SetParticleAgeByIndex,			TYPE_INDEX, TYPE_TIMEVALUE);
	VFN_2(kSetParticleAgeByBornIndex,					SetParticleAgeByBornIndex,		TYPE_INDEX, TYPE_TIMEVALUE);
	PROP_FNS(kGetParticleAge, GetParticleAge, kSetParticleAge, SetParticleAge,			TYPE_TIMEVALUE);

	FN_1(kGetParticleLifespanByIndex,		TYPE_TIMEVALUE,	GetParticleLifespanByIndex,			TYPE_INDEX);
	FN_1(kGetParticleLifespanByBornIndex,	TYPE_TIMEVALUE,	GetParticleLifespanByBornIndex,		TYPE_INDEX);
	VFN_2(kSetParticleLifespanByIndex,						SetParticleLifespanByIndex,			TYPE_INDEX, TYPE_TIMEVALUE);
	VFN_2(kSetParticleLifespanByBornIndex,					SetParticleLifespanByBornIndex,		TYPE_INDEX, TYPE_TIMEVALUE);
	PROP_FNS(kGetParticleLifespan, GetParticleLifespan, kSetParticleLifespan, SetParticleLifespan,			TYPE_TIMEVALUE);

	FN_1(kGetParticleEventTimeByIndex,		TYPE_TIMEVALUE,	GetParticleEventTimeByIndex,	TYPE_INDEX);
	FN_1(kGetParticleEventTimeByBornIndex,	TYPE_TIMEVALUE,	GetParticleEventTimeByBornIndex,TYPE_INDEX);
	VFN_2(kSetParticleEventTimeByIndex,						SetParticleEventTimeByIndex,	TYPE_INDEX, TYPE_TIMEVALUE);
	VFN_2(kSetParticleEventTimeByBornIndex,					SetParticleEventTimeByBornIndex,TYPE_INDEX, TYPE_TIMEVALUE);
	PROP_FNS(kGetParticleEventTime, GetParticleEventTime, kSetParticleEventTime, SetParticleEventTime,	TYPE_TIMEVALUE);

	FN_1(kGetParticlePositionByIndex,		TYPE_POINT3,	GetParticlePositionByIndex,			TYPE_INDEX);
	FN_1(kGetParticlePositionByBornIndex,	TYPE_POINT3,	GetParticlePositionByBornIndex,		TYPE_INDEX);
	VFN_2(kSetParticlePositionByIndex,						SetParticlePositionByIndex,			TYPE_INDEX, TYPE_POINT3);
	VFN_2(kSetParticlePositionByBornIndex,					SetParticlePositionByBornIndex,		TYPE_INDEX, TYPE_POINT3);
	PROP_FNS(kGetParticlePosition, GetParticlePosition, kSetParticlePosition, SetParticlePosition,	TYPE_POINT3);

	FN_1(kGetParticleSpeedByIndex,		TYPE_POINT3,	GetParticleSpeedByIndex,		TYPE_INDEX);
	FN_1(kGetParticleSpeedByBornIndex,	TYPE_POINT3,	GetParticleSpeedByBornIndex,	TYPE_INDEX);
	VFN_2(kSetParticleSpeedByIndex,						SetParticleSpeedByIndex,		TYPE_INDEX, TYPE_POINT3);
	VFN_2(kSetParticleSpeedByBornIndex,					SetParticleSpeedByBornIndex,	TYPE_INDEX, TYPE_POINT3);
	PROP_FNS(kGetParticleSpeed, GetParticleSpeed, kSetParticleSpeed, SetParticleSpeed,	TYPE_POINT3);

	FN_1(kGetParticleAccelerationByIndex,		TYPE_POINT3,	GetParticleAccelerationByIndex,		TYPE_INDEX);
	FN_1(kGetParticleAccelerationByBornIndex,	TYPE_POINT3,	GetParticleAccelerationByBornIndex,	TYPE_INDEX);
	VFN_2(kSetParticleAccelerationByIndex,						SetParticleAccelerationByIndex,		TYPE_INDEX, TYPE_POINT3);
	VFN_2(kSetParticleAccelerationByBornIndex,					SetParticleAccelerationByBornIndex,	TYPE_INDEX, TYPE_POINT3);
	PROP_FNS(kGetParticleAcceleration, GetParticleAcceleration, kSetParticleAcceleration, SetParticleAcceleration,	TYPE_POINT3);

	FN_1(kGetParticleOrientationByIndex,		TYPE_POINT3,	GetParticleOrientationByIndex,		TYPE_INDEX);
	FN_1(kGetParticleOrientationByBornIndex,	TYPE_POINT3,	GetParticleOrientationByBornIndex,	TYPE_INDEX);
	VFN_2(kSetParticleOrientationByIndex,						SetParticleOrientationByIndex,		TYPE_INDEX, TYPE_POINT3);
	VFN_2(kSetParticleOrientationByBornIndex,					SetParticleOrientationByBornIndex,	TYPE_INDEX, TYPE_POINT3);
	PROP_FNS(kGetParticleOrientation, GetParticleOrientation, kSetParticleOrientation, SetParticleOrientation,	TYPE_POINT3);

	FN_1(kGetParticleSpinByIndex,		TYPE_ANGAXIS,	GetParticleSpinByIndex,		TYPE_INDEX);
	FN_1(kGetParticleSpinByBornIndex,	TYPE_ANGAXIS,	GetParticleSpinByBornIndex,	TYPE_INDEX);
	VFN_2(kSetParticleSpinByIndex,						SetParticleSpinByIndex,		TYPE_INDEX, TYPE_ANGAXIS);
	VFN_2(kSetParticleSpinByBornIndex,					SetParticleSpinByBornIndex,	TYPE_INDEX, TYPE_ANGAXIS);
	PROP_FNS(kGetParticleSpin, GetParticleSpin, kSetParticleSpin, SetParticleSpin,	TYPE_ANGAXIS);

	FN_1(kGetParticleScaleByIndex,			TYPE_FLOAT,	GetParticleScaleByIndex,		TYPE_INDEX);
	FN_1(kGetParticleScaleByBornIndex,		TYPE_FLOAT,	GetParticleScaleByBornIndex,	TYPE_INDEX);
	VFN_2(kSetParticleScaleByIndex,						SetParticleScaleByIndex,		TYPE_INDEX, TYPE_FLOAT);
	VFN_2(kSetParticleScaleByBornIndex,					SetParticleScaleByBornIndex,	TYPE_INDEX, TYPE_FLOAT);
	PROP_FNS(kGetParticleScale, GetParticleScale, kSetParticleScale, SetParticleScale,	TYPE_FLOAT);
	FN_1(kGetParticleScaleXYZByIndex,		TYPE_POINT3,GetParticleScaleXYZByIndex,		TYPE_INDEX);
	FN_1(kGetParticleScaleXYZByBornIndex,	TYPE_POINT3,GetParticleScaleXYZByBornIndex,	TYPE_INDEX);
	VFN_2(kSetParticleScaleXYZByIndex,					SetParticleScaleXYZByIndex,		TYPE_INDEX, TYPE_POINT3);
	VFN_2(kSetParticleScaleXYZByBornIndex,				SetParticleScaleXYZByBornIndex,	TYPE_INDEX, TYPE_POINT3);
	PROP_FNS(kGetParticleScaleXYZ, GetParticleScaleXYZ, kSetParticleScaleXYZ, SetParticleScaleXYZ,	TYPE_POINT3);

	FN_1(kGetParticleTMByIndex,			TYPE_MATRIX3,	GetParticleTMByIndex,		TYPE_INDEX);
	FN_1(kGetParticleTMByBornIndex,		TYPE_MATRIX3,	GetParticleTMByBornIndex,	TYPE_INDEX);
	VFN_2(kSetParticleTMByIndex,						SetParticleTMByIndex,		TYPE_INDEX, TYPE_MATRIX3);
	VFN_2(kSetParticleTMByBornIndex,					SetParticleTMByBornIndex,	TYPE_INDEX, TYPE_MATRIX3);
	PROP_FNS(kGetParticleTM, GetParticleTM, kSetParticleTM, SetParticleTM,			TYPE_MATRIX3);

	FN_1(kGetParticleSelectedByIndex,		TYPE_bool,	GetParticleSelectedByIndex,		TYPE_INDEX);
	FN_1(kGetParticleSelectedByBornIndex,	TYPE_bool,	GetParticleSelectedByBornIndex,	TYPE_INDEX);
	VFN_2(kSetParticleSelectedByIndex,					SetParticleSelectedByIndex,		TYPE_INDEX, TYPE_bool);
	VFN_2(kSetParticleSelectedByBornIndex,				SetParticleSelectedByBornIndex,	TYPE_INDEX, TYPE_bool);
	PROP_FNS(kGetParticleSelected, GetParticleSelected, kSetParticleSelected, SetParticleSelected,	TYPE_bool);

	FN_1(kGetParticleShapeByIndex,		TYPE_MESH,	GetParticleShapeByIndex,		TYPE_INDEX);
	FN_1(kGetParticleShapeByBornIndex,	TYPE_MESH,	GetParticleShapeByBornIndex,	TYPE_INDEX);
	VFN_2(kSetParticleShapeByIndex,					SetParticleShapeByIndex,		TYPE_INDEX, TYPE_MESH);
	VFN_2(kSetParticleShapeByBornIndex,				SetParticleShapeByBornIndex,	TYPE_INDEX, TYPE_MESH);
	VFN_1(kSetGlobalParticleShape,					SetGlobalParticleShape,			TYPE_MESH);
	PROP_FNS(kGetParticleShape, GetParticleShape, kSetParticleShape, SetParticleShape,			TYPE_MESH);

	VFN_2(kSetParticleMtlIndexByIndex,				SetParticleMtlIndexByIndex,		TYPE_INDEX, TYPE_INDEX);
	VFN_2(kSetParticleMtlIndexByBornIndex,			SetParticleMtlIndexByBornIndex,	TYPE_INDEX, TYPE_INDEX);
	VFN_1(kSetGlobalParticleMtlIndex,				SetParticleMtlIndex,			TYPE_INDEX);

	VFN_3(kSetParticleMappingByIndex,				SetParticleMappingByIndex,		TYPE_INDEX, TYPE_INT, TYPE_POINT3);
	VFN_3(kSetParticleMappingByBornIndex,			SetParticleMappingByBornIndex,	TYPE_INDEX, TYPE_INT, TYPE_POINT3);
	VFN_2(kSetGlobalParticleMapping,				SetParticleMapping,				TYPE_INT, TYPE_POINT3);

	FN_1(kGetParticleIntByIndex,		TYPE_INT,	GetParticleIntByIndex,		TYPE_INDEX);
	FN_1(kGetParticleIntByBornIndex,	TYPE_INT,	GetParticleIntByBornIndex,	TYPE_INDEX);
	VFN_2(kSetParticleIntByIndex,					SetParticleIntByIndex,		TYPE_INDEX, TYPE_INT);
	VFN_2(kSetParticleIntByBornIndex,				SetParticleIntByBornIndex,	TYPE_INDEX, TYPE_INT);
	PROP_FNS(kGetParticleInt, GetParticleInt, kSetParticleInt, SetParticleInt,	TYPE_INT);

	FN_1(kGetParticleFloatByIndex,		TYPE_FLOAT,	GetParticleFloatByIndex,		TYPE_INDEX);
	FN_1(kGetParticleFloatByBornIndex,	TYPE_FLOAT,	GetParticleFloatByBornIndex,	TYPE_INDEX);
	VFN_2(kSetParticleFloatByIndex,					SetParticleFloatByIndex,		TYPE_INDEX, TYPE_FLOAT);
	VFN_2(kSetParticleFloatByBornIndex,				SetParticleFloatByBornIndex,	TYPE_INDEX, TYPE_FLOAT);
	PROP_FNS(kGetParticleFloat, GetParticleFloat, kSetParticleFloat, SetParticleFloat,	TYPE_FLOAT);

	FN_1(kGetParticleVectorByIndex,		TYPE_POINT3,	GetParticleVectorByIndex,		TYPE_INDEX);
	FN_1(kGetParticleVectorByBornIndex,	TYPE_POINT3,	GetParticleVectorByBornIndex,	TYPE_INDEX);
	VFN_2(kSetParticleVectorByIndex,					SetParticleVectorByIndex,		TYPE_INDEX, TYPE_POINT3);
	VFN_2(kSetParticleVectorByBornIndex,				SetParticleVectorByBornIndex,	TYPE_INDEX, TYPE_POINT3);
	PROP_FNS(kGetParticleVector, GetParticleVector, kSetParticleVector, SetParticleVector,	TYPE_POINT3);

	FN_1(kGetParticleMatrixByIndex,			TYPE_MATRIX3,	GetParticleMatrixByIndex,		TYPE_INDEX);
	FN_1(kGetParticleMatrixByBornIndex,		TYPE_MATRIX3,	GetParticleMatrixByBornIndex,	TYPE_INDEX);
	VFN_2(kSetParticleMatrixByIndex,						SetParticleMatrixByIndex,		TYPE_INDEX, TYPE_MATRIX3);
	VFN_2(kSetParticleMatrixByBornIndex,					SetParticleMatrixByBornIndex,	TYPE_INDEX, TYPE_MATRIX3);
	PROP_FNS(kGetParticleMatrix, GetParticleMatrix, kSetParticleMatrix, SetParticleMatrix,	TYPE_MATRIX3);

	VFN_2(kSetParticleTestStatusByIndex,					SetParticleTestStatusByIndex,		TYPE_INDEX, TYPE_bool);
	VFN_2(kSetParticleTestStatusByBornIndex,				SetParticleTestStatusByBornIndex,	TYPE_INDEX, TYPE_bool);
	VFN_1(kSetGlobalTestStatus,								SetGlobalTestStatus,				TYPE_bool);
	PROP_FNS(kGetParticleTestStatus, GetParticleTestStatus, kSetParticleTestStatus, SetParticleTestStatus,	TYPE_bool);
	
	VFN_2(kSetParticleTestTimeByIndex,					SetParticleTestTimeByIndex,		TYPE_INDEX, TYPE_TIMEVALUE);
	VFN_3(kSetParticleTestTimePreciseByIndex,			SetParticleTestTimeByIndex,		TYPE_INDEX, TYPE_TIMEVALUE, TYPE_FLOAT);
	VFN_2(kSetParticleTestTimeByBornIndex,				SetParticleTestTimeByBornIndex, TYPE_INDEX, TYPE_TIMEVALUE);
	VFN_3(kSetParticleTestTimePreciseByBornIndex,		SetParticleTestTimeByBornIndex,	TYPE_INDEX, TYPE_TIMEVALUE, TYPE_FLOAT);
	PROP_FNS(kGetParticleTestTime, GetParticleTestTime, kSetParticleTestTime, SetParticleTestTime,	TYPE_TIMEVALUE);
	VFN_2(kSetParticleTestTimePrecise,					SetParticleTestTime,			TYPE_TIMEVALUE, TYPE_FLOAT);
	VFN_1(kSetGlobalTestTime,							SetGlobalTestTime,				TYPE_TIMEVALUE);
	VFN_2(kSetGlobalTestTimePrecise,					SetGlobalTestTime,				TYPE_TIMEVALUE, TYPE_FLOAT);

	END_FUNCTION_MAP

    /** @defgroup IMXSParticleContainer IMXSParticleContainer.h
    *  @{
    */

	/*! \fn virtual IObject* GetParticleContainer() const = 0;
	*  \brief The non-maxscript-version of particle container in the Init/Proceed/Release calls for operator and test
	*/
	virtual IObject* GetParticleContainer() const = 0;

	/*! \fn virtual Object* GetParticleSystem() const = 0;
	*  \brief The particle system in the Init/Proceed/Release calls for operator and test
	*/
	virtual Object* GetParticleSystem() const = 0;

	/*! \fn virtual INode* GetParticleSystemNode() const = 0;
	*  \brief The particle system node in the Init/Proceed/Release calls for operator and test
	*/
	virtual INode* GetParticleSystemNode() const = 0;

	/*! \fn virtual Tab<Object*>& GetInitActions() = 0;
	*  \brief The list of actions during initialization in the Init/Proceed/Release calls for operator and test
	*/
	virtual Tab<Object*>& GetInitActions() = 0;

	/*! \fn virtual Tab<INode*>& GetInitActionNodes() = 0;
	*  \brief The list of action nodes during initialization in the Init/Proceed/Release calls for operator and test
	*/
	virtual Tab<INode*>& GetInitActionNodes() = 0;

	/*! \fn virtual TimeValue GetTimeStart() const = 0;
	*  \brief Start of the proceed interval in the Init/Proceed/Release calls for operator and test
	*/
	virtual TimeValue GetTimeStart() const = 0;

	/*! \fn virtual TimeValue GetTimeEnd() const = 0;
	*  \brief End of the proceed interval in the Init/Proceed/Release calls for operator and test
	*/
	virtual TimeValue GetTimeEnd() const = 0;

	/*! \fn virtual void SetTimeEnd(TimeValue time) = 0;
	*  \brief Set the end time if the operator can't proceed particles to the given time end in the Init/Proceed/Release calls for operator
	*/
	virtual void SetTimeEnd(TimeValue time) = 0;

	/*! \fn virtual void SetTimeEnd(TimeValue time, float timeDelta) = 0;
	*  \brief Set the end time if the test can't proceed particles to the given time end in the Init/Proceed/Release calls for test
	*/
	virtual void SetTimeEnd(TimeValue time, float timeDelta) = 0;

	/*! \fn virtual INode* GetActionNode() const = 0;
	*  \brief The node of the action in the Init/Proceed/Release calls for operator and test
	*/
	virtual INode* GetActionNode() const = 0;

	/*! \fn virtual FPInterface* GetIntegrator() const = 0;
	*  \brief Update cycle integrator in the Init/Proceed/Release calls for operator and test
	*/
	virtual FPInterface* GetIntegrator() const = 0;

	/*! \fn virtual int		RandSign( void ) = 0;
	*  \brief Get random sign { -1, 1 }
	*/
	virtual int		RandSign( void ) = 0;

	/*! \fn virtual float	Rand01( void ) = 0;
	*  \brief Get random number between 0.0f and 1.0f
	*/
	virtual float	Rand01( void ) = 0;

	/*! \fn virtual float	Rand11( void ) = 0;
	*  \brief Get random number between -1.0f and 1.0f
	*/
	virtual float	Rand11( void ) = 0;

	/*! \fn virtual float	Rand55( void ) = 0;
	*  \brief Get random number between -0.5f and 0.5f
	*/
	virtual float	Rand55( void ) = 0;

	/*! \fn virtual int		Rand0X(int maxnum) = 0;
	*  \brief Get integer random number between 0 and maxnum
	*/
	virtual int		Rand0X(int maxnum) = 0;

	/*! \fn virtual Point3*  RandSpherePoint() = 0;
	*  \brief Get random point from sphere surface of radius 1
	*/
	virtual Point3*  RandSpherePoint() = 0;

	/*! \fn virtual Point3*	RandDivergeVector(Point3 vec, float maxAngle) = 0;
	*  \brief Returns a vector with the same length as a given vector but the direction differs from the given for no more than maxAngle
	*/
	virtual Point3*	RandDivergeVector(Point3 vec, float maxAngle) = 0;

	/*! \fn virtual bool GetUseTime() const = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual bool GetUseTime() const = 0;

	/*! \fn virtual void SetUseTime(bool use) = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual void SetUseTime(bool use) = 0;

	/*! \fn virtual bool GetUseAge() const = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual bool GetUseAge() const = 0;

	/*! \fn virtual void SetUseAge(bool use) = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual void SetUseAge(bool use) = 0;

	/*! \fn virtual bool GetUseLifespan() const = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual bool GetUseLifespan() const = 0;

	/*! \fn virtual void SetUseLifespan(bool use) = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual void SetUseLifespan(bool use) = 0;

	/*! \fn virtual bool GetUseEventTime() const = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual bool GetUseEventTime() const = 0;

	/*! \fn virtual void SetUseEventTime(bool use) = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual void SetUseEventTime(bool use) = 0;

	/*! \fn virtual bool GetUsePosition() const = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual bool GetUsePosition() const = 0;

	/*! \fn virtual void SetUsePosition(bool use) = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual void SetUsePosition(bool use) = 0;

	/*! \fn virtual bool GetUseSpeed() const = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual bool GetUseSpeed() const = 0;

	/*! \fn virtual void SetUseSpeed(bool use) = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual void SetUseSpeed(bool use) = 0;

	/*! \fn virtual bool GetUseAcceleration() const = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual bool GetUseAcceleration() const = 0;

	/*! \fn virtual void SetUseAcceleration(bool use) = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual void SetUseAcceleration(bool use) = 0;

	/*! \fn virtual bool GetUseOrientation() const = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual bool GetUseOrientation() const = 0;

	/*! \fn virtual void SetUseOrientation(bool use) = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual void SetUseOrientation(bool use) = 0;

	/*! \fn virtual bool GetUseSpin() const = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual bool GetUseSpin() const = 0;

	/*! \fn virtual void SetUseSpin(bool use) = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual void SetUseSpin(bool use) = 0;

	/*! \fn virtual bool GetUseScale() const = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual bool GetUseScale() const = 0;

	/*! \fn virtual void SetUseScale(bool use) = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual void SetUseScale(bool use) = 0;

	/*! \fn virtual bool GetUseTM() const = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual bool GetUseTM() const = 0;

	/*! \fn virtual void SetUseTM(bool use) = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual void SetUseTM(bool use) = 0;

	/*! \fn virtual bool GetUseSelected() const = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual bool GetUseSelected() const = 0;

	/*! \fn virtual void SetUseSelected(bool use) = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual void SetUseSelected(bool use) = 0;

	/*! \fn virtual bool GetUseShape() const = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual bool GetUseShape() const = 0;

	/*! \fn virtual void SetUseShape(bool use) = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual void SetUseShape(bool use) = 0;

	/*! \fn virtual bool GetUseMtlIndex() const = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual bool GetUseMtlIndex() const = 0;

	/*! \fn virtual void SetUseMtlIndex(bool use) = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual void SetUseMtlIndex(bool use) = 0;

	/*! \fn virtual bool GetUseMapping() const = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual bool GetUseMapping() const = 0;

	/*! \fn virtual void SetUseMapping(bool use) = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual void SetUseMapping(bool use) = 0;

	/*! \fn virtual bool GetUseInt() const = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual bool GetUseInt() const = 0;

	/*! \fn virtual void SetUseInt(bool use) = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual void SetUseInt(bool use) = 0;

	/*! \fn virtual bool GetUseFloat() const = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual bool GetUseFloat() const = 0;

	/*! \fn virtual void SetUseFloat(bool use) = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual void SetUseFloat(bool use) = 0;

	/*! \fn virtual bool GetUseVector() const = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual bool GetUseVector() const = 0;

	/*! \fn virtual void SetUseVector(bool use) = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual void SetUseVector(bool use) = 0;

	/*! \fn virtual bool GetUseMatrix() const = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual bool GetUseMatrix() const = 0;

	/*! \fn virtual void SetUseMatrix(bool use) = 0;
	*  \brief Used to define whether particle channel will be used by the operator/test
	*/
	virtual void SetUseMatrix(bool use) = 0;

	/*! \fn virtual int NumParticles() = 0;
	*  \brief Method to get/set properties of the particle in the container
	*/
	virtual int NumParticles() = 0;

	/*! \fn virtual bool AddParticle() = 0;
	*  \brief Add a single particle to particle system. Returns true if the operation was completed successfully. 
	*/
	virtual bool AddParticle() = 0;

	/*! \fn virtual bool AddParticles(int num) = 0;
	*  \brief Add "num" particles into the particle system. Returns true if the operation was completed successfully.
 	*/
	virtual bool AddParticles(int num) = 0;

	/*! \fn virtual bool DeleteParticle(int index) = 0;
	*  \brief Delete a single particle with the given index. Returns true if the operation was completed successfully.
 	*/
	virtual bool DeleteParticle(int index) = 0;

	/*! \fn virtual bool DeleteParticles(int start, int num) = 0;
	*  \brief List-type delete of "num" particles starting with "start". Returns true if the operation was completed successfully. 
	*/
	virtual bool DeleteParticles(int start, int num) = 0;

	/*! \fn virtual int GetParticleBornIndex(int i) = 0;
	*  \brief Each particle is given a unique ID (consecutive) upon its birth. The method 
	allows us to distinguish physically different particles even if they are using the same particle index (because of the "index reusing").
	\param int i: index of the particle in the range of [0, NumParticles-1]
	*/
	virtual int GetParticleBornIndex(int i) = 0;

	/*! \fn virtual bool HasParticleBornIndex(int bornIndex, int& index) = 0;
	*  \brief Implemented by the Plug-In. The methods verifies if a particle with a given particle id (born index) is present
	in the particle container. The methods returns index in the container. If there is no such particle, the method returns false.
	\param bornIndex: particle born index
	\param index: particle index in the particle group or particle system
	*/
	virtual bool HasParticleBornIndex(int bornIndex, int& index) = 0;

	/*! \fn virtual int GetCurrentParticleIndex() = 0;
	*  \brief Defines "current" index, used to get the property without specifying the index.
	*/
	virtual int GetCurrentParticleIndex() = 0;

	/*! \fn virtual int GetCurrentParticleBornIndex() = 0;
	*  \brief Defines "current" bornIndex, used to get the property without specifying the index.
	*/
	virtual int GetCurrentParticleBornIndex() = 0;

	/*! \fn virtual void SetCurrentParticleIndex(int index) = 0;
	*  \brief Defines "current" index, used to set the property without specifying the index.
	*/
	virtual void SetCurrentParticleIndex(int index) = 0;

	/*! \fn virtual void SetCurrentParticleBornIndex(int bornIndex) = 0;
	*  \brief Defines "current" bornIndex, used to set the property without specifying the index.
	*/
	virtual void SetCurrentParticleBornIndex(int bornIndex) = 0;

	/*! \fn virtual bool IsParticleNewByIndex(int index) = 0;
	*  \brief Defines if a particle just arrived into the current event. Particle is specified by either its index in the particle container, 
	or by its born index. If no index is specified then the "current" index is used.
	\param id: particle born index
	\param index: particle index in the particle container
	*/
	virtual bool IsParticleNewByIndex(int index) = 0;

	/*! \fn virtual bool IsParticleNewByBornIndex(int id) = 0;
	*  \brief See IsParticleNewByIndex().
	*/
	virtual bool IsParticleNewByBornIndex(int id) = 0;

	/*! \fn virtual bool IsParticleNew() = 0;
	*  \brief See IsParticleNewByIndex().
	*/
	virtual bool IsParticleNew() = 0;

	/*! \fn virtual TimeValue GetParticleTimeByIndex(int index) = 0;
	*  \brief Defines time for the current state for a particle. Particle is specified by either its
	index in the particle container, or by its born index. If no index is specified then the "current" index is used.
	Modification of the time is only accessible for tests, and only for particles that satisfy the test.
	\param id: int, particle born index
	\param index: int, particle index in the particle container
	\param time: TimeValue, time of the current state for the particle
	\param timeDelta: float, if you need more precision to set time value then use the timeDelta parameter
	*/
	virtual TimeValue GetParticleTimeByIndex(int index) = 0;

	/*! \fn virtual TimeValue GetParticleTimeByBornIndex(int id) = 0;
	*  \brief See GetParticleTimeByIndex().
	*/
	virtual TimeValue GetParticleTimeByBornIndex(int id) = 0;

	/*! \fn virtual void SetParticleTimeByIndex(int index, TimeValue time) = 0;
	*  \brief See GetParticleTimeByIndex().
	*/
	virtual void SetParticleTimeByIndex(int index, TimeValue time) = 0;

	/*! \fn virtual void SetParticleTimeByIndex(int index, TimeValue time, float timeDelta) = 0;
	*  \brief See GetParticleTimeByIndex().
	*/
	virtual void SetParticleTimeByIndex(int index, TimeValue time, float timeDelta) = 0;

	/*! \fn virtual void SetParticleTimeByBornIndex(int id, TimeValue time) = 0;
	*  \brief See GetParticleTimeByIndex().
	*/
	virtual void SetParticleTimeByBornIndex(int id, TimeValue time) = 0;

	/*! \fn virtual void SetParticleTimeByBornIndex(int id, TimeValue time, float timeDelta) = 0;
	*  \brief See GetParticleTimeByIndex().
	*/
	virtual void SetParticleTimeByBornIndex(int id, TimeValue time, float timeDelta) = 0;

	/*! \fn virtual TimeValue GetParticleTime() = 0;
	*  \brief See GetParticleTimeByIndex().
	*/
	virtual TimeValue GetParticleTime() = 0;

	/*! \fn virtual void SetParticleTime(TimeValue time) = 0;
	*  \brief See GetParticleTimeByIndex().
	*/
	virtual void SetParticleTime(TimeValue time) = 0;

	/*! \fn virtual TimeValue GetParticleAgeByIndex(int index) = 0;
	*  \brief Defines age of the specified particle. Particle is specified by either its
	index in the particle group or particle system, or by its born index.
	If no index is specified then the "current" index is used.
	\param int id: particle born index
	\param int index: particle index in the particle group
	\param TimeValue age: new age value to set for a particle
	*/
	virtual TimeValue GetParticleAgeByIndex(int index) = 0;

	/*! \fn virtual TimeValue GetParticleAgeByBornIndex(int id) = 0;
	*  \brief See GetParticleAgeByIndex().
	*/
	virtual TimeValue GetParticleAgeByBornIndex(int id) = 0;

	/*! \fn virtual void SetParticleAgeByIndex(int index, TimeValue age) = 0;
	*  \brief See GetParticleAgeByIndex().
	*/
	virtual void SetParticleAgeByIndex(int index, TimeValue age) = 0;

	/*! \fn virtual void SetParticleAgeByBornIndex(int id, TimeValue age) = 0;
	*  \brief See GetParticleAgeByIndex().
	*/
	virtual void SetParticleAgeByBornIndex(int id, TimeValue age) = 0;

	/*! \fn virtual TimeValue GetParticleAge() = 0;
	*  \brief See GetParticleAgeByIndex().
	*/
	virtual TimeValue GetParticleAge() = 0;

	/*! \fn virtual void SetParticleAge(TimeValue age) = 0;
	*  \brief See GetParticleAgeByIndex().
	*/
	virtual void SetParticleAge(TimeValue age) = 0;

	/*! \fn virtual TimeValue GetParticleLifespanByIndex(int index) = 0;
	*  \brief Defines lifespan of the specified particle. Particle is specified by either its
	index in the particle group or particle system, or by its born index.
	If no index is specified then the "current" index is used.
	\param int id: particle born index
	\param int index: particle index in the particle group
	\param TimeValue lifespan: new lifespan value to set for a particle
	*/
	virtual TimeValue GetParticleLifespanByIndex(int index) = 0;

	/*! \fn virtual TimeValue GetParticleLifespanByBornIndex(int id) = 0;
	*  \brief See GetParticleLifespanByIndex().
	*/
	virtual TimeValue GetParticleLifespanByBornIndex(int id) = 0;

	/*! \fn virtual void SetParticleLifespanByIndex(int index, TimeValue lifespan) = 0;
	*  \brief See GetParticleLifespanByIndex().
	*/
	virtual void SetParticleLifespanByIndex(int index, TimeValue lifespan) = 0;

	/*! \fn virtual void SetParticleLifespanByBornIndex(int id, TimeValue lifespan) = 0;
	*  \brief See GetParticleLifespanByIndex().
	*/
	virtual void SetParticleLifespanByBornIndex(int id, TimeValue lifespan) = 0;

	/*! \fn virtual TimeValue GetParticleLifespan() = 0;
	*  \brief See GetParticleLifespanByIndex().
	*/
	virtual TimeValue GetParticleLifespan() = 0;

	/*! \fn virtual void SetParticleLifespan(TimeValue lifespan) = 0;
	*  \brief See GetParticleLifespanByIndex().
	*/
	virtual void SetParticleLifespan(TimeValue lifespan) = 0;

	/*! \fn virtual TimeValue GetParticleEventTimeByIndex(int index) = 0;
	*  \brief Defines for how long the specified particle was staying in the current
	action list (event). Particle is specified by either its
	index in the particle container, or by its born index.
	If no index is specified then the "current" index is used.
	\param int id: particle born index
	\param int index: particle index in the particle container
	\param TimeValue time: how long particle was staying in the current action list (event)
	*/
	virtual TimeValue GetParticleEventTimeByIndex(int index) = 0;

	/*! \fn virtual TimeValue GetParticleEventTimeByBornIndex(int id) = 0;
	*  \brief See GetParticleEventTimeByIndex().
	*/
	virtual TimeValue GetParticleEventTimeByBornIndex(int id) = 0;

	/*! \fn virtual void SetParticleEventTimeByIndex(int index, TimeValue time) = 0;
	*  \brief See GetParticleEventTimeByIndex().
	*/
	virtual void SetParticleEventTimeByIndex(int index, TimeValue time) = 0;

	/*! \fn virtual void SetParticleEventTimeByBornIndex(int id, TimeValue time) = 0;
	*  \brief See GetParticleEventTimeByIndex().
	*/
	virtual void SetParticleEventTimeByBornIndex(int id, TimeValue time) = 0;

	/*! \fn virtual TimeValue GetParticleEventTime() = 0;
	*  \brief See GetParticleEventTimeByIndex().
	*/
	virtual TimeValue GetParticleEventTime() = 0;

	/*! \fn virtual void SetParticleEventTime(TimeValue time) = 0;
	*  \brief See GetParticleEventTimeByIndex().
	*/
	virtual void SetParticleEventTime(TimeValue time) = 0;
	
	/*! \fn virtual Point3* GetParticlePositionByIndex(int index) = 0;
	*  \brief Defines position of the specified particle in the current state.
	Particle is specified by either its index in the particle container, or by its born index.
	If no index is specified then the "current" index is used.
	\param int id: 	particle born index
	\param int index: particle index in the particle group
	\param Point3 pos: position of the particle
	*/
	virtual Point3* GetParticlePositionByIndex(int index) = 0;

	/*! \fn virtual Point3* GetParticlePositionByBornIndex(int id) = 0;
	*  \brief See GetParticlePositionByIndex().
	*/
	virtual Point3* GetParticlePositionByBornIndex(int id) = 0;

	/*! \fn virtual void SetParticlePositionByIndex(int index, Point3 pos) = 0;
	*  \brief See GetParticlePositionByIndex().
	*/
	virtual void SetParticlePositionByIndex(int index, Point3 pos) = 0;

	/*! \fn virtual void SetParticlePositionByBornIndex(int id, Point3 pos) = 0;
	*  \brief See GetParticlePositionByIndex().
	*/
	virtual void SetParticlePositionByBornIndex(int id, Point3 pos) = 0;

	/*! \fn virtual Point3* GetParticlePosition() = 0;
	*  \brief See GetParticlePositionByIndex().
	*/
	virtual Point3* GetParticlePosition() = 0;

	/*! \fn virtual void SetParticlePosition(Point3 pos) = 0;
	*  \brief See GetParticlePositionByIndex().
	*/
	virtual void SetParticlePosition(Point3 pos) = 0;

	/*! \fn virtual Point3* GetParticleSpeedByIndex(int index) = 0;
	*  \brief Defines speed of the specified particle in the current state.
	Particle is specified by either its index in the particle container, or by its born index.
	If no index is specified then the "current" index is used.
	\param int id: particle born index
	\param int index: particle index in the particle group
	\param Point3 speed: speed of the particle in units per frame
	*/
	virtual Point3* GetParticleSpeedByIndex(int index) = 0;

	/*! \fn virtual Point3* GetParticleSpeedByBornIndex(int id) = 0;
	*  \brief See GetParticleSpeedByIndex().
	*/
	virtual Point3* GetParticleSpeedByBornIndex(int id) = 0;

	/*! \fn virtual void SetParticleSpeedByIndex(int index, Point3 speed) = 0;
	*  \brief See GetParticleSpeedByIndex().
	*/
	virtual void SetParticleSpeedByIndex(int index, Point3 speed) = 0;

	/*! \fn virtual void SetParticleSpeedByBornIndex(int id, Point3 speed) = 0;
	*  \brief See GetParticleSpeedByIndex().
	*/
	virtual void SetParticleSpeedByBornIndex(int id, Point3 speed) = 0;

	/*! \fn virtual Point3* GetParticleSpeed() = 0;
	*  \brief See GetParticleSpeedByIndex().
	*/
	virtual Point3* GetParticleSpeed() = 0;

	/*! \fn virtual void SetParticleSpeed(Point3 speed) = 0;
	*  \brief See GetParticleSpeedByIndex().
	*/
	virtual void SetParticleSpeed(Point3 speed) = 0;

	/*! \fn virtual Point3* GetParticleAccelerationByIndex(int index) = 0;
	*  \brief Defines acceleration of the specified particle in the current state.
	Particle is specified by either its index in the particle container, or by its born index.
	If no index is specified then the "current" index is used. The acceleration is reset in the 
	each update cycle so the operator/test should update it for each particle in each proceed call.
	\param int id: particle born index
	\param int index: particle index in the particle group
	\param Point3 accel: acceleration of the particle in units per frame squared
	*/
	virtual Point3* GetParticleAccelerationByIndex(int index) = 0;

	/*! \fn virtual Point3* GetParticleAccelerationByBornIndex(int id) = 0;
	*  \brief See GetParticleAccelerationByIndex().
	*/
	virtual Point3* GetParticleAccelerationByBornIndex(int id) = 0;

	/*! \fn virtual void SetParticleAccelerationByIndex(int index, Point3 accel) = 0;
	*  \brief See GetParticleAccelerationByIndex().
	*/
	virtual void SetParticleAccelerationByIndex(int index, Point3 accel) = 0;

	/*! \fn virtual void SetParticleAccelerationByBornIndex(int id, Point3 accel) = 0;
	*  \brief See GetParticleAccelerationByIndex().
	*/
	virtual void SetParticleAccelerationByBornIndex(int id, Point3 accel) = 0;

	/*! \fn virtual Point3* GetParticleAcceleration() = 0;
	*  \brief See GetParticleAccelerationByIndex().
	*/
	virtual Point3* GetParticleAcceleration() = 0;

	/*! \fn virtual void SetParticleAcceleration(Point3 speed) = 0;
	*  \brief See GetParticleAccelerationByIndex().
	*/
	virtual void SetParticleAcceleration(Point3 speed) = 0;

	/*! \fn virtual Point3* GetParticleOrientationByIndex(int index) = 0;
	*  \brief Defines orientation of the specified particle in the current state.
	Particle is specified by either its index in the particle container, or by its born index.
	If no index is specified then the "current" index is used.
	\param int id: particle born index
	\param int index: particle index in the particle group
	\param Point3 orient: orientation of the particle. The orientation is defined by incremental rotations by world axes X, Y and Z. The rotation values are in degrees.
	*/
	virtual Point3* GetParticleOrientationByIndex(int index) = 0;

	/*! \fn virtual Point3* GetParticleOrientationByBornIndex(int id) = 0;
	*  \brief See GetParticleOrientationByIndex().
	*/
	virtual Point3* GetParticleOrientationByBornIndex(int id) = 0;

	/*! \fn virtual void SetParticleOrientationByIndex(int index, Point3 orient) = 0;
	*  \brief See GetParticleOrientationByIndex().
	*/
	virtual void SetParticleOrientationByIndex(int index, Point3 orient) = 0;

	/*! \fn virtual void SetParticleOrientationByBornIndex(int id, Point3 orient) = 0;
	*  \brief See GetParticleOrientationByIndex().
	*/
	virtual void SetParticleOrientationByBornIndex(int id, Point3 orient) = 0;

	/*! \fn virtual Point3* GetParticleOrientation() = 0;
	*  \brief See GetParticleOrientationByIndex().
	*/
	virtual Point3* GetParticleOrientation() = 0;

	/*! \fn virtual void SetParticleOrientation(Point3 orient) = 0;
	*  \brief See GetParticleOrientationByIndex().
	*/
	virtual void SetParticleOrientation(Point3 orient) = 0;

	/*! \fn virtual AngAxis* GetParticleSpinByIndex(int index) = 0;
	*  \brief Defines angular speed of the specified particle in the current state.
	Particle is specified by either its index in the particle container, or by its born index.
	If no index is specified then the "current" index is used.
	\param int id: particle born index
	\param int index: particle index in the particle group
	\param AngAxis spin: angular speed of the particle in rotation per frame, axis defines rotation axis, angle defines rotation amount per frame
	*/
	virtual AngAxis* GetParticleSpinByIndex(int index) = 0;

	/*! \fn virtual AngAxis* GetParticleSpinByBornIndex(int id) = 0;
	*  \brief See GetParticleSpinByIndex().
	*/
	virtual AngAxis* GetParticleSpinByBornIndex(int id) = 0;

	/*! \fn virtual void SetParticleSpinByIndex(int index, AngAxis spin) = 0;
	*  \brief See GetParticleSpinByIndex().
	*/
	virtual void SetParticleSpinByIndex(int index, AngAxis spin) = 0;

	/*! \fn virtual void SetParticleSpinByBornIndex(int id, AngAxis spin) = 0;
	*  \brief See GetParticleSpinByIndex().
	*/
	virtual void SetParticleSpinByBornIndex(int id, AngAxis spin) = 0;

	/*! \fn virtual AngAxis* GetParticleSpin() = 0;
	*  \brief See GetParticleSpinByIndex().
	*/
	virtual AngAxis* GetParticleSpin() = 0;

	/*! \fn virtual void SetParticleSpin(AngAxis spin) = 0;
	*  \brief See GetParticleSpinByIndex().
	*/
	virtual void SetParticleSpin(AngAxis spin) = 0;

	/*! \fn virtual float GetParticleScaleByIndex(int index) = 0;
	*  \brief Defines scale factor of the specified particle in the current state. The XYZ form is used for non-uniform scaling.
	Particle is specified by either its index in the particle container, or by its born index.
	If no index is specified then the "current" index is used.
	\param int id: particle born index
	\param int index: particle index in the particle group
	\param float scale: uniform scale factor
	\param Point3 scale: scale factor for each local axis of the particle
	*/
	virtual float GetParticleScaleByIndex(int index) = 0;

	/*! \fn virtual float GetParticleScaleByBornIndex(int id) = 0;
	*  \brief See GetParticleScaleByIndex().
	*/
	virtual float GetParticleScaleByBornIndex(int id) = 0;

	/*! \fn virtual void SetParticleScaleByIndex(int index, float scale) = 0;
	*  \brief See GetParticleScaleByIndex().
	*/
	virtual void SetParticleScaleByIndex(int index, float scale) = 0;

	/*! \fn virtual void SetParticleScaleByBornIndex(int id, float scale) = 0;
	*  \brief See GetParticleScaleByIndex().
	*/
	virtual void SetParticleScaleByBornIndex(int id, float scale) = 0;

	/*! \fn virtual float GetParticleScale() = 0;
	*  \brief See GetParticleScaleByIndex().
	*/
	virtual float GetParticleScale() = 0;

	/*! \fn virtual void SetParticleScale(float scale) = 0;
	*  \brief See GetParticleScaleByIndex().
	*/
	virtual void SetParticleScale(float scale) = 0;

	/*! \fn virtual Point3* GetParticleScaleXYZByIndex(int index) = 0;
	*  \brief See GetParticleScaleByIndex().
	*/
	virtual Point3* GetParticleScaleXYZByIndex(int index) = 0;

	/*! \fn virtual Point3* GetParticleScaleXYZByBornIndex(int id) = 0;
	*  \brief See GetParticleScaleByIndex().
	*/
	virtual Point3* GetParticleScaleXYZByBornIndex(int id) = 0;

	/*! \fn virtual void SetParticleScaleXYZByIndex(int index, Point3 scale) = 0;
	*  \brief See GetParticleScaleByIndex().
	*/
	virtual void SetParticleScaleXYZByIndex(int index, Point3 scale) = 0;

	/*! \fn virtual void SetParticleScaleXYZByBornIndex(int id, Point3 scale) = 0;
	*  \brief See GetParticleScaleByIndex().
	*/
	virtual void SetParticleScaleXYZByBornIndex(int id, Point3 scale) = 0;

	/*! \fn virtual Point3* GetParticleScaleXYZ() = 0;
	*  \brief See GetParticleScaleByIndex().
	*/
	virtual Point3* GetParticleScaleXYZ() = 0;

	/*! \fn virtual void SetParticleScaleXYZ(Point3 scale) = 0;
	*  \brief See GetParticleScaleByIndex().
	*/
	virtual void SetParticleScaleXYZ(Point3 scale) = 0;

	/*! \fn virtual Matrix3* GetParticleTMByIndex(int index) = 0;
	*  \brief Defines transformation matrix of the specified particle in the current state. Particle is specified by either its index 
	in the particle container, or by its born index. If no index is specified then the "current" index is used.
	\param int id: particle born index
	\param int index: particle index in the particle group
	\param Matrix3 tm: transformation matrix of the particle
	*/
	virtual Matrix3* GetParticleTMByIndex(int index) = 0;

	/*! \fn virtual Matrix3* GetParticleTMByBornIndex(int id) = 0;
	*  \brief See GetParticleTMByIndex().
	*/
	virtual Matrix3* GetParticleTMByBornIndex(int id) = 0;

	/*! \fn virtual void SetParticleTMByIndex(int index, Matrix3 tm) = 0;
	*  \brief See GetParticleTMByIndex().
	*/
	virtual void SetParticleTMByIndex(int index, Matrix3 tm) = 0;

	/*! \fn virtual void SetParticleTMByBornIndex(int id, Matrix3 tm) = 0;
	*  \brief See GetParticleTMByIndex().
	*/
	virtual void SetParticleTMByBornIndex(int id, Matrix3 tm) = 0;

	/*! \fn virtual Matrix3* GetParticleTM() = 0;
	*  \brief See GetParticleTMByIndex().
	*/
	virtual Matrix3* GetParticleTM() = 0;

	/*! \fn virtual void SetParticleTM(Matrix3 tm) = 0;
	*  \brief See GetParticleTMByIndex().
	*/
	virtual void SetParticleTM(Matrix3 tm) = 0;

	/*! \fn virtual bool GetParticleSelectedByIndex(int index) = 0;
	*  \brief Defines selection status of the specified particle in the current state.
	Particle is specified by either its index in the particle container, or by its born index.
	If no index is specified then the "current" index is used.
	\param int id: particle born index
	\param int index: particle index in the particle group
	\param bool selected: selection status of the particle
	*/
	virtual bool GetParticleSelectedByIndex(int index) = 0;

	/*! \fn virtual bool GetParticleSelectedByBornIndex(int id) = 0;
	*  \brief See GetParticleSelectedByIndex().
	*/
	virtual bool GetParticleSelectedByBornIndex(int id) = 0;

	/*! \fn virtual void SetParticleSelectedByIndex(int index, bool selected) = 0;
	*  \brief See GetParticleSelectedByIndex().
	*/
	virtual void SetParticleSelectedByIndex(int index, bool selected) = 0;

	/*! \fn virtual void SetParticleSelectedByBornIndex(int id, bool selected) = 0;
	*  \brief See GetParticleSelectedByIndex().
	*/
	virtual void SetParticleSelectedByBornIndex(int id, bool selected) = 0;

	/*! \fn virtual bool GetParticleSelected() = 0;
	*  \brief See GetParticleSelectedByIndex().
	*/
	virtual bool GetParticleSelected() = 0;

	/*! \fn virtual void SetParticleSelected(bool selected) = 0;
	*  \brief See GetParticleSelectedByIndex().
	*/
	virtual void SetParticleSelected(bool selected) = 0;


	/*! \fn virtual Mesh* GetParticleShapeByIndex(int index) = 0;
	*  \brief Defines shape of the specified particle in the current state.
	Particle is specified by either its index in the particle container, or by its born index.
	if no index is specified then the "current" index is used.
	\param int id: particle born index
	\param int index: particle index in the particle group
	\param Mesh* shape: shape of the particle
	*/
	virtual Mesh* GetParticleShapeByIndex(int index) = 0;

	/*! \fn virtual Mesh* GetParticleShapeByBornIndex(int id) = 0;
	*  \brief See GetParticleShapeByIndex().
	*/
	virtual Mesh* GetParticleShapeByBornIndex(int id) = 0;

	/*! \fn virtual void SetParticleShapeByIndex(int index, Mesh* shape) = 0;
	*  \brief See GetParticleShapeByIndex().
	*/
	virtual void SetParticleShapeByIndex(int index, Mesh* shape) = 0;

	/*! \fn virtual void SetParticleShapeByBornIndex(int id, Mesh* shape) = 0;
	*  \brief See GetParticleShapeByIndex().
	*/
	virtual void SetParticleShapeByBornIndex(int id, Mesh* shape) = 0;

	/*! \fn virtual Mesh* GetParticleShape() = 0;
	*  \brief See GetParticleShapeByIndex().
	*/
	virtual Mesh* GetParticleShape() = 0;

	/*! \fn virtual void SetParticleShape(Mesh* shape) = 0;
	*  \brief See GetParticleShapeByIndex().
	*/
	virtual void SetParticleShape(Mesh* shape) = 0;

	/*! \fn virtual void SetGlobalParticleShape(Mesh* shape) = 0;
	*  \brief  Set the same shape for all particles. See GetParticleShapeByIndex().
	*/
	virtual void SetGlobalParticleShape(Mesh* shape) = 0;

	/*! \fn virtual void SetParticleMtlIndexByIndex(int index, int mtlIndex) = 0;
	*  \brief Defines material index of the specified particle in the current state.
	Particle is specified by either its index in the particle container, or by its born index.
	If no index is specified then the "current" index is used.
	\param int id: particle born index
	\param int index: particle index in the particle group
	\param int mtlIndex: material index of the particle
	*/
	virtual void SetParticleMtlIndexByIndex(int index, int mtlIndex) = 0;

	/*! \fn virtual void SetParticleMtlIndexByBornIndex(int id, int mtlIndex) = 0;
	*  \brief See SetParticleMtlIndexByIndex().
	*/
	virtual void SetParticleMtlIndexByBornIndex(int id, int mtlIndex) = 0;

	/*! \fn virtual void SetParticleMtlIndex(int mtlIndex) = 0;
	*  \brief See SetParticleMtlIndexByIndex().
	*/
	virtual void SetParticleMtlIndex(int mtlIndex) = 0;

	/*! \fn virtual void SetParticleMappingByIndex(int index, int mapChannel, UVVert mapValue) = 0;
	*  \brief Defines mapping of the specified particle in the current state.
	All vertices of the particle are assigned the same map value.
	Particle is specified by either its index in the particle container, or by its born index.
	If no index is specified then the "current" index is used.
	\param int id: particle born index
	\param int index: particle index in the particle group
	\param int mapChannel: mapping channel; range from 0 to MAX_MESHMAPS-1
	\param UVVert mapValue: mapping value of the particle
	*/
	virtual void SetParticleMappingByIndex(int index, int mapChannel, UVVert mapValue) = 0;

	/*! \fn virtual void SetParticleMappingByBornIndex(int id, int mapChannel, UVVert mapValue) = 0;
	*  \brief See SetParticleMappingByIndex().
	*/
	virtual void SetParticleMappingByBornIndex(int id, int mapChannel, UVVert mapValue) = 0;

	/*! \fn virtual void SetParticleMapping(int mapChannel, UVVert mapValue) = 0;
	*  \brief See SetParticleMappingByIndex().
	*/
	virtual void SetParticleMapping(int mapChannel, UVVert mapValue) = 0;

	/*! \fn virtual int GetParticleIntByIndex(int index) = 0;
	*  \brief Defines integer value of the specified particle.
	The methods can be used to keep integer data with the particle, and to be used later.
	Particle is specified by either its index in the particle container, or by its born index.
	If no index is specified then the "current" index is used.
	\param int id: particle born index
	\param int index: particle index in the particle group
	\param int value: integer value of the particle
	*/
	virtual int GetParticleIntByIndex(int index) = 0;

	/*! \fn virtual int GetParticleIntByBornIndex(int id) = 0;
	*  \brief See GetParticleIntByIndex().
	*/
	virtual int GetParticleIntByBornIndex(int id) = 0;

	/*! \fn virtual void SetParticleIntByIndex(int index, int value) = 0;
	*  \brief See GetParticleIntByIndex().
	*/
	virtual void SetParticleIntByIndex(int index, int value) = 0;

	/*! \fn virtual void SetParticleIntByBornIndex(int id, int value) = 0;
	*  \brief See GetParticleIntByIndex().
	*/
	virtual void SetParticleIntByBornIndex(int id, int value) = 0;

	/*! \fn virtual int GetParticleInt() = 0;
	*  \brief See GetParticleIntByIndex().
	*/
	virtual int GetParticleInt() = 0;

	/*! \fn virtual void SetParticleInt(int value) = 0;
	*  \brief See GetParticleIntByIndex().
	*/
	virtual void SetParticleInt(int value) = 0;

	/*! \fn virtual float GetParticleFloatByIndex(int index) = 0;
	*  \brief Defines float value of the specified particle.
	The methods can be used to keep float data with the particle, and to be used later.
	Particle is specified by either its index in the particle container, or by its born index.
	If no index is specified then the "current" index is used.
	\param int id: particle born index
	\param int index: particle index in the particle group
	\param float value: float value of the particle
	*/
	virtual float GetParticleFloatByIndex(int index) = 0;

	/*! \fn virtual float GetParticleFloatByBornIndex(int id) = 0;
	*  \brief See GetParticleFloatByIndex().
	*/
	virtual float GetParticleFloatByBornIndex(int id) = 0;

	/*! \fn virtual void SetParticleFloatByIndex(int index, float value) = 0;
	*  \brief See GetParticleFloatByIndex().
	*/
	virtual void SetParticleFloatByIndex(int index, float value) = 0;

	/*! \fn virtual void SetParticleFloatByBornIndex(int id, float value) = 0;
	*  \brief See GetParticleFloatByIndex().
	*/
	virtual void SetParticleFloatByBornIndex(int id, float value) = 0;

	/*! \fn virtual float GetParticleFloat() = 0;
	*  \brief See GetParticleFloatByIndex().
	*/
	virtual float GetParticleFloat() = 0;

	/*! \fn virtual void SetParticleFloat(float value) = 0;
	*  \brief See GetParticleFloatByIndex().
	*/
	virtual void SetParticleFloat(float value) = 0;

	/*! \fn virtual Point3* GetParticleVectorByIndex(int index) = 0;
	*  \brief Defines vector value of the specified particle. Can be used to keep vector data with the particle, and to be used later.
	Particle is specified by either its index in the particle container, or by its born index. If no index is specified then the "current" index is used.
	\param id: int, particle born index
	\param index: int, particle index in the particle group
	\param Point3: vector value of the particle
	*/
	virtual Point3* GetParticleVectorByIndex(int index) = 0;

	/*! \fn virtual Point3* GetParticleVectorByBornIndex(int id) = 0;
	*  \brief See GetParticleVectorByIndex().
	*/
	virtual Point3* GetParticleVectorByBornIndex(int id) = 0;

	/*! \fn virtual void SetParticleVectorByIndex(int index, Point3 value) = 0;
	*  \brief See GetParticleVectorByIndex().
	*/
	virtual void SetParticleVectorByIndex(int index, Point3 value) = 0;

	/*! \fn virtual void SetParticleVectorByBornIndex(int id, Point3 value) = 0;
	*  \brief See GetParticleVectorByIndex().
	*/
	virtual void SetParticleVectorByBornIndex(int id, Point3 value) = 0;

	/*! \fn virtual Point3* GetParticleVector() = 0;
	*  \brief See GetParticleVectorByIndex().
	*/
	virtual Point3* GetParticleVector() = 0;

	/*! \fn virtual void SetParticleVector(Point3 value) = 0;
	*  \brief See GetParticleVectorByIndex().
	*/
	virtual void SetParticleVector(Point3 value) = 0;

	/*! \fn virtual Matrix3* GetParticleMatrixByIndex(int index) = 0;
	*  \brief Defines matrix value of the specified particle. Can be used to keep matrix data with the particle, and to be used later.
	Particle is specified by either its index in the particle container, or by its born index. If no index is specified then the "current" index is used.
	\param int id: particle born index
	\param int index: particle index in the particle group
	\param Matrix3 value: matrix value of the particle
	*/
	virtual Matrix3* GetParticleMatrixByIndex(int index) = 0;

	/*! \fn virtual Matrix3* GetParticleMatrixByBornIndex(int id) = 0;
	*  \brief See GetParticleMatrixByIndex().
	*/
	virtual Matrix3* GetParticleMatrixByBornIndex(int id) = 0;

	/*! \fn virtual void SetParticleMatrixByIndex(int index, Matrix3 value) = 0;
	*  \brief See GetParticleMatrixByIndex().
	*/
	virtual void SetParticleMatrixByIndex(int index, Matrix3 value) = 0;

	/*! \fn virtual void SetParticleMatrixByBornIndex(int id, Matrix3 value) = 0;
	*  \brief See GetParticleMatrixByIndex().
	*/
	virtual void SetParticleMatrixByBornIndex(int id, Matrix3 value) = 0;

	/*! \fn virtual Matrix3* GetParticleMatrix() = 0;
	*  \brief See GetParticleMatrixByIndex().
	*/
	virtual Matrix3* GetParticleMatrix() = 0;

	/*! \fn virtual void SetParticleMatrix(Matrix3 value) = 0;
	*  \brief See GetParticleMatrixByIndex().
	*/
	virtual void SetParticleMatrix(Matrix3 value) = 0;

	/*! \fn virtual void SetParticleTestStatusByIndex(int index, bool testStatus) = 0;
	*  \brief Defines test status (true or false) for particles while testing. Particle is specified by either its index in the particle container, 
	or by its born index. If no index is specified then the "current" index is used.
	\param int id: particle born index
	\param int index: particle index in the particle group
	\param bool testStatus: test status
	*/
	virtual void SetParticleTestStatusByIndex(int index, bool testStatus) = 0;

	/*! \fn virtual void SetParticleTestStatusByBornIndex(int id, bool testStatus) = 0;
	*  \brief See SetParticleTestStatusByIndex()
	*/
	virtual void SetParticleTestStatusByBornIndex(int id, bool testStatus) = 0;

	/*! \fn virtual bool GetParticleTestStatus() = 0;
	*  \brief See SetParticleTestStatusByIndex()
	*/
	virtual bool GetParticleTestStatus() = 0;

	/*! \fn virtual void SetParticleTestStatus(bool testStatus) = 0;
	*  \brief See SetParticleTestStatusByIndex().
	*/
	virtual void SetParticleTestStatus(bool testStatus) = 0;

	/*! \fn virtual void SetGlobalTestStatus(bool testStatus) = 0;
	*  \brief Set the same test status for all particles
	*/
	virtual void SetGlobalTestStatus(bool testStatus) = 0;

	/*! \fn virtual void SetParticleTestTimeByIndex(int index, TimeValue testTime) = 0;
	*  \brief Defines the time when a particle satisfies the test. Particle is specified by either its index in the particle container, 
	or by its born index. If no index is specified then the "current" index is used.
	\param id: int, particle born index
	\param index: int, particle index in the particle group
	\param testTime: TimeValue; time when a particle satisfies the test
	\param testTimeDelta: float, if testTime parameter doesn't have enough precision then use the delta for fine tuning the test time
	*/
	virtual void SetParticleTestTimeByIndex(int index, TimeValue testTime) = 0;

	/*! \fn virtual void SetParticleTestTimeByIndex(int index, TimeValue testTime, float testTimeDelta) = 0;
	*  \brief See SetParticleTestTimeByIndex().
	*/
	virtual void SetParticleTestTimeByIndex(int index, TimeValue testTime, float testTimeDelta) = 0;

	/*! \fn virtual void SetParticleTestTimeByBornIndex(int id, TimeValue testTime) = 0;
	*  \brief See SetParticleTestTimeByIndex().
	*/
	virtual void SetParticleTestTimeByBornIndex(int id, TimeValue testTime) = 0;

	/*! \fn virtual void SetParticleTestTimeByBornIndex(int id, TimeValue testTime, float testTimeDelta) = 0;
	*  \brief See SetParticleTestTimeByIndex().
	*/
	virtual void SetParticleTestTimeByBornIndex(int id, TimeValue testTime, float testTimeDelta) = 0;

	/*! \fn
	*  \brief See SetParticleTestTimeByIndex().
	*/
	virtual TimeValue GetParticleTestTime() = 0;

	/*! \fn
	*  \brief See SetParticleTestTimeByIndex().
	*/
	virtual void SetParticleTestTime(TimeValue testTime) = 0;

	/*! \fn virtual void SetParticleTestTime(TimeValue testTime, float testTimeDelta) = 0;
	*  \brief See SetParticleTestTimeByIndex().
	*/
	virtual void SetParticleTestTime(TimeValue testTime, float testTimeDelta) = 0;

	/*! \fn virtual void SetGlobalTestTime(TimeValue testTime) = 0;
	*  \brief set the same test status for all particles
	*/
	virtual void SetGlobalTestTime(TimeValue testTime) = 0;

	/*! \fn virtual void SetGlobalTestTime(TimeValue testTime, float testTimeDelta) = 0;
	*  \brief set the same test status for all particles
	*/
	virtual void SetGlobalTestTime(TimeValue testTime, float testTimeDelta) = 0;

	/*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(MXSPARTICLECONTAINER_INTERFACE); }
	*  \brief
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(MXSPARTICLECONTAINER_INTERFACE); }
};

