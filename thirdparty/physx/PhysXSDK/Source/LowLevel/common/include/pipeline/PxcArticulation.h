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



#ifndef PXC_ARTICULATION_H
#define PXC_ARTICULATION_H

#include "PsVecMath.h"
#include "CmSpatialVector.h"
#include "PxcSpatial.h"
#include "PsBitUtils.h"
#include "PxvArticulation.h"

namespace physx
{

#ifdef PX_PS3
#define PXC_ARTICULATION_DEBUG_VERIFY 0	// doesn't work on PS3 anyway
#else
#define PXC_ARTICULATION_DEBUG_VERIFY 0
#endif



static const size_t PXC_ARTICULATION_MAX_SIZE = 64;
static const size_t PXC_ARTICULATION_IDMASK = PXC_ARTICULATION_MAX_SIZE-1;

PX_FORCE_INLINE PxU32 PxcArticulationLowestSetBit(PxcArticulationBitField val)
{
#ifdef _XBOX
	return 63 - _CountLeadingZeros64(__int64(val ^ (val-1)));
#else
	PxU32 low = PxU32(val&0xffffffff), high = PxU32(val>>32);
	PxU32 mask = PxU32((!low)-1);
	PxU32 result = (mask&Ps::lowestSetBitUnsafe(low)) | ((~mask)&(Ps::lowestSetBitUnsafe(high)+32));
	PX_ASSERT(val & (PxU64(1)<<result));
	PX_ASSERT(!(val & ((PxU64(1)<<result)-1)));
	return result;
#endif

}

class PxsArticulation;
typedef size_t PxsArticulationLinkHandle;

PX_FORCE_INLINE bool isArticulationRootLink(PxsArticulationLinkHandle handle)
{
	return !(handle & PXC_ARTICULATION_IDMASK);
}

PX_FORCE_INLINE PxsArticulation* getArticulation(PxsArticulationLinkHandle handle)
{
	return reinterpret_cast<PxsArticulation*>(handle & ~PXC_ARTICULATION_IDMASK);
}


struct PxcFsAux;

using namespace Ps::aos;

PX_ALIGN_PREFIX(16)
struct PxcSIMDSpatial
{
	Vec3V linear;
	Vec3V angular;

	PX_FORCE_INLINE PxcSIMDSpatial() {}
	PX_FORCE_INLINE PxcSIMDSpatial(PxZERO): linear(V3Zero()), angular(V3Zero()) {}
	PX_FORCE_INLINE PxcSIMDSpatial(const Cm::SpatialVector& v): linear(V3LoadU(v.linear)), angular(V3LoadU(v.angular)) {}
	PX_FORCE_INLINE PxcSIMDSpatial(const Vec3VArg l, const Vec3VArg a): linear(l), angular(a) {}
	PX_FORCE_INLINE PxcSIMDSpatial(const PxcSIMDSpatial& other): linear(other.linear), angular(other.angular) {}
	PX_FORCE_INLINE PxcSIMDSpatial& operator=(const PxcSIMDSpatial& other) { linear = other.linear; angular = other.angular; return *this; }

	PX_FORCE_INLINE PxcSIMDSpatial operator+(const PxcSIMDSpatial& other) const { return PxcSIMDSpatial(V3Add(linear,other.linear),
																								  V3Add(angular, other.angular)); }
	
	PX_FORCE_INLINE PxcSIMDSpatial& operator+=(const PxcSIMDSpatial& other) { linear = V3Add(linear,other.linear); 
																			  angular = V3Add(angular, other.angular);
																			  return *this;
																			}
																								    
	PX_FORCE_INLINE PxcSIMDSpatial operator-(const PxcSIMDSpatial& other) const { return PxcSIMDSpatial(V3Sub(linear,other.linear),
																								  V3Sub(angular, other.angular)); }
	
	PX_FORCE_INLINE PxcSIMDSpatial operator-() const { return PxcSIMDSpatial(V3Neg(linear), V3Neg(angular)); }

	PX_FORCE_INLINE PxcSIMDSpatial operator*(FloatVArg r) const { return PxcSIMDSpatial(V3Scale(linear,r), V3Scale(angular,r)); }

	PX_FORCE_INLINE PxcSIMDSpatial& operator-=(const PxcSIMDSpatial& other) { linear = V3Sub(linear,other.linear); 
																			  angular = V3Sub(angular, other.angular);
																			  return *this;
																			}

	PX_FORCE_INLINE FloatV dot(const PxcSIMDSpatial& other) { return FAdd(V3Dot(linear, other.linear), V3Dot(angular, other.angular)); }

	
}PX_ALIGN_SUFFIX(16);


PX_FORCE_INLINE Cm::SpatialVector& unsimdRef(PxcSIMDSpatial& v)				{ return reinterpret_cast<Cm::SpatialVector&>(v); }
PX_FORCE_INLINE const Cm::SpatialVector& unsimdRef(const PxcSIMDSpatial& v) { return reinterpret_cast<const Cm::SpatialVector&>(v); }


PX_ALIGN_PREFIX(16)
struct PxcFsJointVectors
{
	Vec3V					parentOffset;		// 16 bytes world-space offset from parent to child
	Vec3V					jointOffset;		// 16 bytes world-space offset from child to joint
}
PX_ALIGN_SUFFIX(16);

PX_ALIGN_PREFIX(16)
struct PxcFsRow
{	
	PxcSIMDSpatial			DSI[3];				// 96 bytes
	Mat33V					D;					// 48 bytes
	PxcArticulationBitField	children;			// 8 bytes bitmap of children
	PxcArticulationBitField	pathToRoot;			// 8 bytes bitmap of nodes to root, including self and root
}
PX_ALIGN_SUFFIX(16);

PX_COMPILE_TIME_ASSERT(sizeof(PxcFsRow)==160);



PX_ALIGN_PREFIX(16)
struct PxcFsInertia
{
	Mat33V ll, la, aa;
	PX_FORCE_INLINE PxcFsInertia(const Mat33V& _ll, const Mat33V& _la, const Mat33V& _aa): ll(_ll), la(_la), aa(_aa) {}
	PX_FORCE_INLINE PxcFsInertia(const SpInertia& I)
	: ll(Mat33V_From_PxMat33(I.mLL)), la(Mat33V_From_PxMat33(I.mLA)), aa(Mat33V_From_PxMat33(I.mAA)) {}
	PX_FORCE_INLINE PxcFsInertia() {}

	PX_FORCE_INLINE void operator=(const PxcFsInertia& other)
	{
		ll.col0 = other.ll.col0;	ll.col1 = other.ll.col1;	ll.col2 = other.ll.col2;
		la.col0 = other.la.col0;	la.col1 = other.la.col1;	la.col2 = other.la.col2;
		aa.col0 = other.aa.col0;	aa.col1 = other.aa.col1;	aa.col2 = other.aa.col2;
	}

	PX_FORCE_INLINE PxcFsInertia(const PxcFsInertia& other)
	{
		ll.col0 = other.ll.col0;	ll.col1 = other.ll.col1;	ll.col2 = other.ll.col2;
		la.col0 = other.la.col0;	la.col1 = other.la.col1;	la.col2 = other.la.col2;
		aa.col0 = other.aa.col0;	aa.col1 = other.aa.col1;	aa.col2 = other.aa.col2;
	}

}PX_ALIGN_SUFFIX(16);

PX_ALIGN_PREFIX(16)
struct PxcLtbRow
{
	PxcFsInertia		inertia;			// body inertia in world space
	PxcSIMDSpatial		j0[3], j1[3];		// jacobians
	Mat33V				jResponse;			// inverse response matrix of joint
	Vec3V				jC;
} PX_ALIGN_SUFFIX(16);

PX_ALIGN_PREFIX(16)
struct PxcFsRowAux
{
	PxcSIMDSpatial		S[3];				// motion subspace
}PX_ALIGN_SUFFIX(16);

class PxsArticulation;


struct PxcFsData
{
	PxsArticulation*	articulationX;																//4
	
#if !defined(PX_P64)
	PxU32				pad0;																		//8	
#endif
	PxU16				linkCount;						// number of links							//10
	PxU16				jointVectorOffset;				// offset of read-only data					//12
	PxU16				maxSolverNormalProgress;													//14
	PxU16				maxSolverFrictionProgress;													//16

	PxU64				dirty;																		//24
	PxU16				ltbDataOffset;					// offset of save-velocity data				//26
	PxU16				fsDataOffset;					// offset of joint references				//28
	PxU32				solverProgress;																//32
	

	PxcSIMDSpatial		deferredZ;																	//64
	PxU8				parent[PXC_ARTICULATION_MAX_SIZE];											//128
};

PX_COMPILE_TIME_ASSERT(0 == (sizeof(PxcFsData) & 0x0f));

#define SOLVER_BODY_SOLVER_PROGRESS_OFFSET 28	
#define SOLVER_BODY_MAX_SOLVER_PROGRESS_OFFSET 12

#define MAX_NUM_SPU_ARTICULATIONS 32						//Max number articulated bodies that can be handled on spu
 
namespace
{
	template<class T> PX_FORCE_INLINE T addAddr(void* addr, PxU32 increment) 
	{ 
		return reinterpret_cast<T>(reinterpret_cast<char*>(addr)+increment);
	}

	template<class T> PX_FORCE_INLINE T addAddr(const void* addr, PxU32 increment) 
	{ 
		return reinterpret_cast<T>(reinterpret_cast<const char*>(addr)+increment);
	}
}

PX_FORCE_INLINE PxcSIMDSpatial* getVelocity(PxcFsData& matrix)
{
	return addAddr<PxcSIMDSpatial*>(&matrix, sizeof(PxcFsData));
}

struct PxsArticulationLink;
struct PxsArticulationCore;


PX_FORCE_INLINE const PxcSIMDSpatial* getVelocity(const PxcFsData& matrix)
{
	return addAddr<const PxcSIMDSpatial*>(&matrix, sizeof(PxcFsData));
}

PX_FORCE_INLINE PxcSIMDSpatial* getDeferredVel(PxcFsData& matrix)
{
	return addAddr<PxcSIMDSpatial*>(getVelocity(matrix), sizeof(PxcSIMDSpatial) * matrix.linkCount);
}

PX_FORCE_INLINE const PxcSIMDSpatial* getDeferredVel(const PxcFsData& matrix)
{
	return addAddr<const PxcSIMDSpatial*>(getVelocity(matrix), sizeof(PxcSIMDSpatial) * matrix.linkCount);
}

PX_FORCE_INLINE Vec3V* getDeferredSZ(PxcFsData& matrix)
{
	return addAddr<Vec3V*>(getDeferredVel(matrix), sizeof(PxcSIMDSpatial) * matrix.linkCount);
}

PX_FORCE_INLINE const Vec3V* getDeferredSZ(const PxcFsData& matrix)
{
	return addAddr<const Vec3V*>(getDeferredVel(matrix), sizeof(PxcSIMDSpatial) * matrix.linkCount);
}

PX_FORCE_INLINE const PxReal* getMaxPenBias(const PxcFsData& matrix)
{
	return addAddr<const PxReal*>(getDeferredSZ(matrix), sizeof(Vec3V) * matrix.linkCount);
}

PX_FORCE_INLINE PxReal* getMaxPenBias(PxcFsData& matrix)
{
	return addAddr<PxReal*>(getDeferredSZ(matrix), sizeof(Vec3V) * matrix.linkCount);
}


PX_FORCE_INLINE PxcFsJointVectors* getJointVectors(PxcFsData& matrix)
{
	return addAddr<PxcFsJointVectors *>(&matrix,matrix.jointVectorOffset);
}

PX_FORCE_INLINE const PxcFsJointVectors* getJointVectors(const PxcFsData& matrix)
{
	return addAddr<const PxcFsJointVectors *>(&matrix,matrix.jointVectorOffset);
}

PX_FORCE_INLINE PxcFsInertia& getRootInverseInertia(PxcFsData& matrix)
{
	return *addAddr<PxcFsInertia*>(&matrix,matrix.fsDataOffset);
}

PX_FORCE_INLINE const PxcFsInertia& getRootInverseInertia(const PxcFsData& matrix)
{
	return *addAddr<const PxcFsInertia*>(&matrix,matrix.fsDataOffset);
	
}

PX_FORCE_INLINE PxcFsRow* getFsRows(PxcFsData& matrix)
{
	return addAddr<PxcFsRow*>(&getRootInverseInertia(matrix),sizeof(PxcFsInertia));
}

PX_FORCE_INLINE const PxcFsRow* getFsRows(const PxcFsData& matrix)
{
	return addAddr<const PxcFsRow*>(&getRootInverseInertia(matrix),sizeof(PxcFsInertia));
}


PX_FORCE_INLINE PxcLtbRow* getLtbRows(PxcFsData& matrix)
{
	return addAddr<PxcLtbRow*>(&matrix,matrix.ltbDataOffset);
}

PX_FORCE_INLINE const PxcLtbRow* getLtbRows(const PxcFsData& matrix)
{
	return addAddr<const PxcLtbRow*>(&matrix,matrix.ltbDataOffset);
}


PX_FORCE_INLINE PxcSIMDSpatial* getRefVelocity(PxcFsData& matrix)
{
	return addAddr<PxcSIMDSpatial*>(getLtbRows(matrix), sizeof(PxcLtbRow)*matrix.linkCount);
}

PX_FORCE_INLINE const PxcSIMDSpatial* getRefVelocity(const PxcFsData& matrix)
{
	return addAddr<const PxcSIMDSpatial*>(getLtbRows(matrix), sizeof(PxcLtbRow)*matrix.linkCount);
}

PX_FORCE_INLINE PxcFsRowAux* getAux(PxcFsData& matrix)
{
	return addAddr<PxcFsRowAux*>(getRefVelocity(matrix),sizeof(PxcSIMDSpatial)*matrix.linkCount);
}

PX_FORCE_INLINE const PxcFsRowAux* getAux(const PxcFsData& matrix)
{
	return addAddr<const PxcFsRowAux*>(getRefVelocity(matrix),sizeof(PxcSIMDSpatial)*matrix.linkCount);
}

void PxcFsApplyImpulse(PxcFsData& matrix,
					   PxU32 linkID,
					   Vec3V linear,
					   Vec3V angular);

PxcSIMDSpatial PxcFsGetVelocity(PxcFsData& matrix,
							    PxU32 linkID);


#if PXC_ARTICULATION_DEBUG_VERIFY
namespace PxcArticulationRef 
{	
	Cm::SpatialVector propagateVelocity(const PxcFsRow& row,
										const PxcFsJointVectors& jv,
										const PxVec3& SZ, 
										const Cm::SpatialVector& v, 
										const PxcFsRowAux& aux); 

	Cm::SpatialVector propagateImpulse(const PxcFsRow& row, 
									   const PxcFsJointVectors& jv,
									   PxVec3& SZ, 
									   const Cm::SpatialVector& Z,	
									   const PxcFsRowAux& aux); 

	void applyImpulse(const PxcFsData& matrix,
					  Cm::SpatialVector* velocity,
					  PxU32 linkID, 
					  const Cm::SpatialVector& impulse);

}
#endif

}

#endif //PXC_ARTICULATION
