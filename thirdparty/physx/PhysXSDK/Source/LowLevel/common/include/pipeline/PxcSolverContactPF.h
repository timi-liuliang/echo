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



#ifndef PXC_SOLVERCONTACTPF_H
#define PXC_SOLVERCONTACTPF_H

#include "PxvConfig.h"
#include "PxSimpleTypes.h"
#include "PxVec3.h"

#include "PsVecMath.h"

namespace physx
{

using namespace Ps::aos;


struct PxcSolverContactCoulombHeader
{
	PxU8	type;					//Note: mType should be first as the solver expects a type in the first byte.
	PxU8	numNormalConstr;
	PxU16	frictionOffset;
	PxF32	restitution;
	PxF32	dominance0;
	PxF32	dominance1;
	PxVec3	normalXYZ;
	PxF32	angDom0;
	PxF32	angDom1;
	PxU8	flags;
	PxU8	pad[3];
	PxU32	pad32[2];
	
	PX_FORCE_INLINE void setDominance0(const FloatV f)		{FStore(f, &dominance0);}
	PX_FORCE_INLINE void setDominance1(const FloatV f)		{FStore(f, &dominance1);}
	PX_FORCE_INLINE void setNormal(const Vec3V n)			{V3StoreA(n, normalXYZ);}
	
	PX_FORCE_INLINE FloatV getDominance0() const			{return FLoad(dominance0);}
	PX_FORCE_INLINE FloatV getDominance1() const			{return FLoad(dominance1);}
	PX_FORCE_INLINE FloatV getRestitution() const			{return FLoad(restitution);}
	PX_FORCE_INLINE	Vec3V getNormal()const					{return V3LoadA(normalXYZ);}

  
	PX_FORCE_INLINE void setDominance0(PxF32 f)				{ dominance0 = f; }
	PX_FORCE_INLINE void setDominance1(PxF32 f)				{ dominance1 = f;}
	PX_FORCE_INLINE void setRestitution(PxF32 f)			{ restitution = f;}

	PX_FORCE_INLINE PxF32 getDominance0PxF32() const		{return dominance0;}
	PX_FORCE_INLINE PxF32 getDominance1PxF32() const		{return dominance1;}
	PX_FORCE_INLINE PxF32 getRestitutionPxF32() const		{return restitution;}

}; 
PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverContactCoulombHeader) == 48);


struct PxcSolverContact
{
	Vec4V raXnXYZ_appliedForceW;
	Vec4V rbXnXYZ_velMultiplierW;
	Vec3V scaledBiasX_targetVelocityY_maxImpulseZ;
	Vec4V delAngVel0_InvMassADom;
	Vec4V delAngVel1_InvMassBDom;

	PX_FORCE_INLINE void setAppliedForce(const FloatV f)	{raXnXYZ_appliedForceW=V4SetW(raXnXYZ_appliedForceW,f);}
	PX_FORCE_INLINE void setVelMultiplier(const FloatV f)	{rbXnXYZ_velMultiplierW=V4SetW(rbXnXYZ_velMultiplierW,f);}

	PX_FORCE_INLINE void setScaledBias(const FloatV f)	 	{scaledBiasX_targetVelocityY_maxImpulseZ=V3SetX(scaledBiasX_targetVelocityY_maxImpulseZ,f);}
	PX_FORCE_INLINE void setTargetVelocity(const FloatV f)	{scaledBiasX_targetVelocityY_maxImpulseZ=V3SetY(scaledBiasX_targetVelocityY_maxImpulseZ,f);}
	PX_FORCE_INLINE void setMaxImpulse(const FloatV f)		{scaledBiasX_targetVelocityY_maxImpulseZ=V3SetZ(scaledBiasX_targetVelocityY_maxImpulseZ,f);}

	PX_FORCE_INLINE FloatV getVelMultiplier() const			{return V4GetW(rbXnXYZ_velMultiplierW);}
	PX_FORCE_INLINE FloatV getAppliedForce() const			{return V4GetW(raXnXYZ_appliedForceW);}

	PX_FORCE_INLINE FloatV getScaledBias() const			{return V3GetX(scaledBiasX_targetVelocityY_maxImpulseZ);}
	PX_FORCE_INLINE FloatV getTargetVelocity() const		{return V3GetY(scaledBiasX_targetVelocityY_maxImpulseZ);}
	PX_FORCE_INLINE FloatV getMaxImpulse() const			{return V3GetZ(scaledBiasX_targetVelocityY_maxImpulseZ);}



#ifdef PX_SUPPORT_SIMD
	PX_FORCE_INLINE Vec3V getRaXn() const					{return Vec3V_From_Vec4V(raXnXYZ_appliedForceW);}
	PX_FORCE_INLINE Vec3V getRbXn() const					{return Vec3V_From_Vec4V(rbXnXYZ_velMultiplierW);}
#endif

	PX_FORCE_INLINE void setRaXn(const PxVec3& v)			{V4WriteXYZ(raXnXYZ_appliedForceW, v);}
	PX_FORCE_INLINE void setRbXn(const PxVec3& v)			{V4WriteXYZ(rbXnXYZ_velMultiplierW, v);}
	PX_FORCE_INLINE void setVelMultiplier(PxF32 f)			{V4WriteW(rbXnXYZ_velMultiplierW, f);}
	PX_FORCE_INLINE void setAppliedForce(PxF32 f)			{V4WriteW(raXnXYZ_appliedForceW, f);}

	PX_FORCE_INLINE void setScaledBias(PxF32 f)				{V3WriteX(scaledBiasX_targetVelocityY_maxImpulseZ, f);}
	PX_FORCE_INLINE void setTargetVelocity(PxF32 f)			{V3WriteY(scaledBiasX_targetVelocityY_maxImpulseZ, f);}

	PX_FORCE_INLINE PxF32 getVelMultiplierPxF32() const		{return V4ReadW(rbXnXYZ_velMultiplierW);}
	PX_FORCE_INLINE const PxVec3& getRaXnPxVec3() const		{return V4ReadXYZ(raXnXYZ_appliedForceW);}
	PX_FORCE_INLINE PxF32 getAppliedForcePxF32() const		{return V4ReadW(raXnXYZ_appliedForceW);}
	PX_FORCE_INLINE const PxVec3& getRbXnPxVec3() const		{return V4ReadXYZ(rbXnXYZ_velMultiplierW);}
	PX_FORCE_INLINE PxF32 getScaledBiasPxF32() const		{return V3ReadX(scaledBiasX_targetVelocityY_maxImpulseZ);}
	PX_FORCE_INLINE PxF32 getTargetVelocityPxF32() const	{return V3ReadY(scaledBiasX_targetVelocityY_maxImpulseZ);}	
};

PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverContact) == 80);


struct PxcSolverContactExt : public PxcSolverContact
{
	Vec4V linDeltaVA;
	Vec4V linDeltaVB;

	PX_FORCE_INLINE void setDeltaVA(const PxVec3& lin, const PxVec3 ang)
	{
		V4WriteXYZ(linDeltaVA, lin); V4WriteW(linDeltaVA,0);
		V4WriteXYZ(delAngVel0_InvMassADom, ang); V4WriteW(delAngVel0_InvMassADom,0);
	}

	PX_FORCE_INLINE void setDeltaVB(const PxVec3& lin, const PxVec3 ang)
	{
		V4WriteXYZ(linDeltaVB, lin); V4WriteW(linDeltaVB,0);
		V4WriteXYZ(delAngVel1_InvMassBDom, ang); V4WriteW(delAngVel1_InvMassBDom,0);
	}
};

PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverContactExt) == 112);

struct PxcSolverFrictionHeader
{
	PxU8	type;					//Note: mType should be first as the solver expects a type in the first byte.
	PxU8	numNormalConstr;
	PxU8	numFrictionConstr;
	PxU8	pad;
	PxF32   staticFriction;
	PxF32   dominance0;
	PxF32	dominance1;
	PxF32	angDom0;
	PxF32	angDom1;
	PxU32	pad2[2];

	PX_FORCE_INLINE void setStaticFriction(const FloatV f)	{FStore(f, &staticFriction);}
	
	PX_FORCE_INLINE FloatV getStaticFriction() const		{return FLoad(staticFriction);}
	PX_FORCE_INLINE FloatV getDominance0() const			{return FLoad(dominance0);}
	PX_FORCE_INLINE FloatV getDominance1() const			{return FLoad(dominance1);}
	
	PX_FORCE_INLINE void setStaticFriction(PxF32 f)			{staticFriction = f;}
	PX_FORCE_INLINE void setDominance0(PxF32 f)				{dominance0 = f;}
	PX_FORCE_INLINE void setDominance1(PxF32 f)				{dominance1 = f;}

	PX_FORCE_INLINE PxF32 getStaticFrictionPxF32() const	{return staticFriction;}
	PX_FORCE_INLINE PxF32 getDominance0PxF32() const		{return dominance0;}
	PX_FORCE_INLINE PxF32 getDominance1PxF32() const		{return dominance1;}
	

	PX_FORCE_INLINE PxU32 getAppliedForcePaddingSize() const {return sizeof(PxU32)*((4 * ((numNormalConstr + 3)/4)));}
	static PX_FORCE_INLINE PxU32 getAppliedForcePaddingSize(const PxU32 numConstr) {return sizeof(PxU32)*((4 * ((numConstr + 3)/4)));}
}; 

PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverFrictionHeader) == 32);

struct PxcSolverFriction
{
	Vec4V normalXYZ_appliedForceW;		//16
	Vec4V raXnXYZ_velMultiplierW;		//32
	Vec4V rbXnXYZ_targetVelocityW;		//48
	Vec4V delAngVel0_InvMassADom;		//64
	Vec4V delAngVel1_InvMassBDom;		//80
	
	PxU32 contactIndex;					//88 or 92 //can be PxU8
	
	PxU32 mPad[3];						//92 or 96

	PX_FORCE_INLINE void setAppliedForce(const FloatV f)	{normalXYZ_appliedForceW=V4SetW(normalXYZ_appliedForceW,f);}
	PX_FORCE_INLINE void setVelMultiplier(const FloatV f)	{raXnXYZ_velMultiplierW=V4SetW(raXnXYZ_velMultiplierW,f);}

	PX_FORCE_INLINE FloatV getAppliedForce() const			{return V4GetW(normalXYZ_appliedForceW);}
	PX_FORCE_INLINE FloatV getVelMultiplier() const			{return V4GetW(raXnXYZ_velMultiplierW);}
	
#ifdef PX_SUPPORT_SIMD
	PX_FORCE_INLINE Vec3V getNormal() const					{return Vec3V_From_Vec4V(normalXYZ_appliedForceW);}
	PX_FORCE_INLINE Vec3V getRaXn() const					{return Vec3V_From_Vec4V(raXnXYZ_velMultiplierW);}
#endif	

	PX_FORCE_INLINE void setNormal(const PxVec3& v)			{V4WriteXYZ(normalXYZ_appliedForceW, v);}
	PX_FORCE_INLINE void setRaXn(const PxVec3& v)			{V4WriteXYZ(raXnXYZ_velMultiplierW, v);}
	PX_FORCE_INLINE void setRbXn(const PxVec3& v)			{V4WriteXYZ(rbXnXYZ_targetVelocityW, v);}

	PX_FORCE_INLINE const PxVec3& getNormalPxVec3() const	{return V4ReadXYZ(normalXYZ_appliedForceW);}
	PX_FORCE_INLINE const PxVec3& getRaXnPxVec3() const		{return V4ReadXYZ(raXnXYZ_velMultiplierW);}
	PX_FORCE_INLINE const PxVec3& getRbXnPxVec3() const		{return V4ReadXYZ(rbXnXYZ_targetVelocityW);}

	PX_FORCE_INLINE void setAppliedForce(PxF32 f)			{V4WriteW(normalXYZ_appliedForceW, f);}
	PX_FORCE_INLINE void setVelMultiplier(PxF32 f)			{V4WriteW(raXnXYZ_velMultiplierW, f);}
	
	PX_FORCE_INLINE PxF32 getAppliedForcePxF32() const		{return V4ReadW(normalXYZ_appliedForceW);}
	PX_FORCE_INLINE PxF32 getVelMultiplierPxF32() const		{return V4ReadW(raXnXYZ_velMultiplierW);}

}; 

PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverFriction) == 96);


struct PxcSolverFrictionExt : public PxcSolverFriction
{
	Vec4V linDeltaVA;
	Vec4V linDeltaVB;

	PX_FORCE_INLINE void setDeltaVA(const PxVec3& lin, const PxVec3 ang)
	{
		V4WriteXYZ(linDeltaVA, lin); V4WriteW(linDeltaVA,0);
		V4WriteXYZ(delAngVel0_InvMassADom, ang); V4WriteW(delAngVel0_InvMassADom,0);
	}

	PX_FORCE_INLINE void setDeltaVB(const PxVec3& lin, const PxVec3 ang)
	{
		V4WriteXYZ(linDeltaVB, lin); V4WriteW(linDeltaVB,0);
		V4WriteXYZ(delAngVel1_InvMassBDom, ang); V4WriteW(delAngVel1_InvMassBDom,0);
	}

	PX_FORCE_INLINE void getDeltaVAPxVec3(PxVec3& lin, PxVec3& ang) const
	{	
		lin = V4ReadXYZ(linDeltaVA);
		ang = V4ReadXYZ(delAngVel0_InvMassADom);
	}

	PX_FORCE_INLINE void getDeltaVBPxVec3(PxVec3& lin, PxVec3& ang) const
	{	
		lin = V4ReadXYZ(linDeltaVB);
		ang = V4ReadXYZ(delAngVel1_InvMassBDom);
	}
};

PX_COMPILE_TIME_ASSERT(sizeof(PxcSolverFrictionExt) == 128);


}

#endif
