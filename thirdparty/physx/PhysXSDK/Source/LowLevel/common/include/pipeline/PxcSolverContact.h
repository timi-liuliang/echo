/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  



#ifndef PXC_SOLVERCONTACT_H
#define PXC_SOLVERCONTACT_H

#include "PxvConfig.h"
#include "PxSimpleTypes.h"
#include "PxVec3.h"

#include "PsVecMath.h"

namespace physx
{

using namespace Ps::aos;


/**
\brief A header to represent a friction patch for the solver.
*/

struct PxcSolverContactHeader
{
	enum PxcSolverContactFlags
	{
		eHAS_FORCE_THRESHOLDS = 0x1
	};

	PxU8	type;					//Note: mType should be first as the solver expects a type in the first byte.
	PxU8	pad0[3];

	PxU8	flags;	
	PxU8	numNormalConstr;
	PxU8	numFrictionConstr;
	PxU8	pad1;
	PxReal	angDom0;
	PxReal	angDom1;
	Vec4V   staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W;

	PX_FORCE_INLINE void setStaticFriction(const FloatV f)	{staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W=V4SetX(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W,f);}
	PX_FORCE_INLINE void setDynamicFriction(const FloatV f)	{staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W=V4SetY(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W,f);}
	PX_FORCE_INLINE void setDominance0(const FloatV f)		{staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W=V4SetZ(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W,f);}
	PX_FORCE_INLINE void setDominance1(const FloatV f)		{staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W=V4SetW(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W,f);}

	PX_FORCE_INLINE FloatV getStaticFriction() const		{return V4GetX(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W);}
	PX_FORCE_INLINE FloatV getDynamicFriction() const		{return V4GetY(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W);}
	PX_FORCE_INLINE FloatV getDominance0() const			{return V4GetZ(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W);}
	PX_FORCE_INLINE FloatV getDominance1() const			{return V4GetW(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W);}

	PX_FORCE_INLINE void setStaticFriction(PxF32 f)			{V4WriteX(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W, f);}
	PX_FORCE_INLINE void setDynamicFriction(PxF32 f)		{V4WriteY(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W, f);}
	PX_FORCE_INLINE void setDominance0(PxF32 f)				{V4WriteZ(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W, f);}
	PX_FORCE_INLINE void setDominance1(PxF32 f)				{V4WriteW(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W, f);}

	PX_FORCE_INLINE PxF32 getStaticFrictionPxF32() const	{return V4ReadX(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W);}
	PX_FORCE_INLINE PxF32 getDynamicFrictionPxF32() const	{return V4ReadY(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W);}
	PX_FORCE_INLINE PxF32 getDominance0PxF32() const		{return V4ReadZ(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W);}
	PX_FORCE_INLINE PxF32 getDominance1PxF32() const		{return V4ReadW(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W);}
}; 

PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverContactHeader) == 32);

/**
\brief A single rigid body contact point for the solver.
*/
struct PxcSolverContactPoint
{
	Vec4V normalXYZ_velMultiplierW;
	Vec4V raXnXYZ_appliedForceW;
	Vec4V rbXnXYZ_scaledBiasW;
	Vec4V delAngVel0_targetVelocityW;
	Vec4V delAngVel1_maxImpulseW;

	PX_FORCE_INLINE void setAppliedForce(const FloatV f)	{raXnXYZ_appliedForceW=V4SetW(raXnXYZ_appliedForceW,f);}
	PX_FORCE_INLINE void setVelMultiplier(const FloatV f)	{normalXYZ_velMultiplierW=V4SetW(normalXYZ_velMultiplierW,f);}

	PX_FORCE_INLINE void setScaledBias(const FloatV f)	 	{rbXnXYZ_scaledBiasW=V4SetW(rbXnXYZ_scaledBiasW,f);}
	PX_FORCE_INLINE void setTargetVelocity(const FloatV f)	{delAngVel0_targetVelocityW=V4SetW(delAngVel0_targetVelocityW,f);}
	PX_FORCE_INLINE	void setMaxImpulse(const FloatV f)		{delAngVel1_maxImpulseW=V4SetW(delAngVel1_maxImpulseW,f);}

	PX_FORCE_INLINE FloatV getVelMultiplier() const			{return V4GetW(normalXYZ_velMultiplierW);}
	PX_FORCE_INLINE FloatV getAppliedForce() const			{return V4GetW(raXnXYZ_appliedForceW);}

	PX_FORCE_INLINE FloatV getScaledBias() const			{return V4GetW(rbXnXYZ_scaledBiasW);}
	PX_FORCE_INLINE FloatV getTargetVelocity() const		{return V4GetW(delAngVel0_targetVelocityW);}
	PX_FORCE_INLINE FloatV getMaxImpulse() const			{return V4GetW(delAngVel1_maxImpulseW);}


#ifdef PX_SUPPORT_SIMD
	PX_FORCE_INLINE Vec3V getNormal()						{return Vec3V_From_Vec4V(normalXYZ_velMultiplierW);}
	PX_FORCE_INLINE Vec3V getRaXn() const					{return Vec3V_From_Vec4V(raXnXYZ_appliedForceW);}
	PX_FORCE_INLINE Vec3V getRbXn() const					{return Vec3V_From_Vec4V(rbXnXYZ_scaledBiasW);}
#endif

	PX_FORCE_INLINE void setNormal(const PxVec3& v)			{V4WriteXYZ(normalXYZ_velMultiplierW, v);}
	PX_FORCE_INLINE void setRaXn(const PxVec3& v)			{V4WriteXYZ(raXnXYZ_appliedForceW, v);}
	PX_FORCE_INLINE void setRbXn(const PxVec3& v)			{V4WriteXYZ(rbXnXYZ_scaledBiasW, v);}
	PX_FORCE_INLINE void setVelMultiplier(PxF32 f)			{V4WriteW(normalXYZ_velMultiplierW, f);}
	PX_FORCE_INLINE void setAppliedForce(PxF32 f)			{V4WriteW(raXnXYZ_appliedForceW, f);}

	PX_FORCE_INLINE void setScaledBias(PxF32 f)				{V4WriteW(rbXnXYZ_scaledBiasW, f);}
	PX_FORCE_INLINE void setTargetVelocity(PxF32 f)			{V4WriteW(delAngVel0_targetVelocityW, f);}

	PX_FORCE_INLINE const PxVec3& getNormalPxVec3() const	{return V4ReadXYZ(normalXYZ_velMultiplierW);}
	PX_FORCE_INLINE PxF32 getVelMultiplierPxF32() const		{return V4ReadW(normalXYZ_velMultiplierW);}
	PX_FORCE_INLINE const PxVec3& getRaXnPxVec3() const		{return V4ReadXYZ(raXnXYZ_appliedForceW);}
	PX_FORCE_INLINE PxF32 getAppliedForcePxF32() const		{return V4ReadW(raXnXYZ_appliedForceW);}
	PX_FORCE_INLINE const PxVec3& getRbXnPxVec3() const		{return V4ReadXYZ(rbXnXYZ_scaledBiasW);}
	PX_FORCE_INLINE PxF32 getScaledBiasPxF32() const		{return V4ReadW(rbXnXYZ_scaledBiasW);}
	PX_FORCE_INLINE PxF32 getTargetVelocityPxF32() const	{return V4ReadW(delAngVel0_targetVelocityW);}	
}; 


PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverContactPoint) == 80);

/**
\brief A single extended articulation contact point for the solver.
*/
struct PxcSolverContactPointExt : public PxcSolverContactPoint
{
	Vec4V linDeltaVA;
	Vec4V linDeltaVB;

	PX_FORCE_INLINE void setDeltaVA(const PxVec3& lin, const PxVec3& ang)
	{
		V4WriteXYZ(linDeltaVA, lin); V4WriteW(linDeltaVA,0);
		V4WriteXYZ(delAngVel0_targetVelocityW, ang); V4WriteW(delAngVel0_targetVelocityW,0);
	}

	PX_FORCE_INLINE void setDeltaVB(const PxVec3& lin, const PxVec3& ang)
	{
		V4WriteXYZ(linDeltaVB, lin); V4WriteW(linDeltaVB,0);
		V4WriteXYZ(delAngVel1_maxImpulseW, ang); V4WriteW(delAngVel1_maxImpulseW,0);
	}
};

PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverContactPointExt) == 112);


/**
\brief A single friction constraint for the solver.
*/
struct PxcSolverContactFriction
{
	Vec4V normalXYZ_appliedForceW;		//16
	Vec4V raXnXYZ_velMultiplierW;		//32
	Vec4V rbXnXYZ_biasW;				//48
	Vec4V delAngVel0_BrokenW;			//64
	Vec4V delAngVel1_TargetVelW;		//80

	PxU8* frictionBrokenWritebackByte;	//84 or 88 depending on 64-bit machine

	PxU32 mPad0[2];						//92 or 96 depending on 64-bit


#if !defined(PX_P64)
	PxU32 mPad;							//96
#endif

	PX_FORCE_INLINE void setAppliedForce(const FloatV f)	{normalXYZ_appliedForceW=V4SetW(normalXYZ_appliedForceW,f);}
	PX_FORCE_INLINE void setVelMultiplier(const FloatV f)	{raXnXYZ_velMultiplierW=V4SetW(raXnXYZ_velMultiplierW,f);}
	PX_FORCE_INLINE void setBias(const FloatV f)			{rbXnXYZ_biasW=V4SetW(rbXnXYZ_biasW,f);}

	PX_FORCE_INLINE FloatV getAppliedForce() const			{return V4GetW(normalXYZ_appliedForceW);}
	PX_FORCE_INLINE FloatV getVelMultiplier() const			{return V4GetW(raXnXYZ_velMultiplierW);}
	PX_FORCE_INLINE FloatV getBias() const					{return V4GetW(rbXnXYZ_biasW);}

#ifdef PX_SUPPORT_SIMD
	PX_FORCE_INLINE Vec3V getNormal() const					{return Vec3V_From_Vec4V(normalXYZ_appliedForceW);}
	PX_FORCE_INLINE Vec3V getRaXn() const					{return Vec3V_From_Vec4V(raXnXYZ_velMultiplierW);}
	PX_FORCE_INLINE Vec3V getRbXn() const					{return Vec3V_From_Vec4V(rbXnXYZ_biasW);}
#endif	

	PX_FORCE_INLINE void setNormal(const PxVec3& v)			{V4WriteXYZ(normalXYZ_appliedForceW, v);}
	PX_FORCE_INLINE void setRaXn(const PxVec3& v)			{V4WriteXYZ(raXnXYZ_velMultiplierW, v);}
	PX_FORCE_INLINE void setRbXn(const PxVec3& v)			{V4WriteXYZ(rbXnXYZ_biasW, v);}

	PX_FORCE_INLINE const PxVec3& getNormalPxVec3() const	{return V4ReadXYZ(normalXYZ_appliedForceW);}
	PX_FORCE_INLINE const PxVec3& getRaXnPxVec3() const		{return V4ReadXYZ(raXnXYZ_velMultiplierW);}
	PX_FORCE_INLINE const PxVec3& getRbXnPxVec3() const		{return V4ReadXYZ(rbXnXYZ_biasW);}

	PX_FORCE_INLINE void setAppliedForce(PxF32 f)			{V4WriteW(normalXYZ_appliedForceW, f);}
	PX_FORCE_INLINE void setVelMultiplier(PxF32 f)			{V4WriteW(raXnXYZ_velMultiplierW, f);}
	PX_FORCE_INLINE void setBias(PxF32 f)					{V4WriteW(rbXnXYZ_biasW, f);}
	
	PX_FORCE_INLINE PxF32 getAppliedForcePxF32() const		{return V4ReadW(normalXYZ_appliedForceW);}
	PX_FORCE_INLINE PxF32 getVelMultiplierPxF32() const		{return V4ReadW(raXnXYZ_velMultiplierW);}
	PX_FORCE_INLINE PxF32 getBiasPxF32() const				{return V4ReadW(rbXnXYZ_biasW);}

}; 

PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverContactFriction) == 96);

/**
\brief A single extended articulation friction constraint for the solver.
*/
struct PxcSolverContactFrictionExt : public PxcSolverContactFriction
{
	Vec4V linDeltaVA;
	Vec4V linDeltaVB;

	PX_FORCE_INLINE void setDeltaVA(const PxVec3& lin, const PxVec3& ang)
	{
		V4WriteXYZ(linDeltaVA, lin); V4WriteW(linDeltaVA,0);
		V4WriteXYZ(delAngVel0_BrokenW, ang); V4WriteW(delAngVel0_BrokenW,0);
	}

	PX_FORCE_INLINE void setDeltaVB(const PxVec3& lin, const PxVec3& ang)
	{
		V4WriteXYZ(linDeltaVB, lin); V4WriteW(linDeltaVB,0);
		V4WriteXYZ(delAngVel1_TargetVelW, ang); V4WriteW(delAngVel1_TargetVelW,0);
	}

	PX_FORCE_INLINE void getDeltaVAPxVec3(PxVec3& lin, PxVec3& ang) const
	{	
		lin = V4ReadXYZ(linDeltaVA);
		ang = V4ReadXYZ(delAngVel0_BrokenW);
	}

	PX_FORCE_INLINE void getDeltaVBPxVec3(PxVec3& lin, PxVec3& ang) const
	{	
		lin = V4ReadXYZ(linDeltaVB);
		ang = V4ReadXYZ(delAngVel1_TargetVelW);
	}
};
PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverContactFrictionExt) == 128);

}



#endif
