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


#include "PxcArticulation.h"
#include "PxcArticulationScalar.h"
#include "PxcArticulationReference.h"
#include "PxcArticulationFnsDebug.h"

namespace physx
{
namespace PxcArticulationRef
{
	Cm::SpatialVector propagateImpulse(const PxcFsRow& row, 
									   const PxcFsJointVectors& j,
									   PxVec3& SZ,
									   const Cm::SpatialVector& Z,
									   const PxcFsRowAux& aux)
	{
		typedef PxcArticulationFnsScalar Fns;
	
		SZ = Fns::axisDot(reinterpret_cast<const Cm::SpatialVector*>(aux.S), Z);
		return Fns::translateForce(getParentOffset(j), Z - Fns::axisMultiply(getDSI(row), SZ));
	}

	Cm::SpatialVector propagateVelocity(const PxcFsRow& row,
									    const PxcFsJointVectors& j,
										const PxVec3& SZ,
										const Cm::SpatialVector& v,
										const PxcFsRowAux& aux)
	{
		typedef PxcArticulationFnsScalar Fns;

		Cm::SpatialVector w = Fns::translateMotion(-getParentOffset(j), v);
		PxVec3 DSZ = Fns::multiply(row.D, SZ);

		return w - Fns::axisMultiply(reinterpret_cast<const Cm::SpatialVector*>(aux.S), DSZ + Fns::axisDot(getDSI(row), w));
	}

	void applyImpulse(const PxcFsData& matrix, 
					  Cm::SpatialVector* velocity,
					  PxU32 linkID, 
					  const Cm::SpatialVector& impulse)
	{
		typedef PxcArticulationFnsScalar Fns;

		PX_ASSERT(matrix.linkCount<=PXC_ARTICULATION_MAX_SIZE);

		const PxcFsRow* rows = getFsRows(matrix);
		const PxcFsRowAux* aux = getAux(matrix);
		const PxcFsJointVectors* jointVectors = getJointVectors(matrix);

		Cm::SpatialVector dV[PXC_ARTICULATION_MAX_SIZE];
		PxVec3 SZ[PXC_ARTICULATION_MAX_SIZE];

		for(PxU32 i=0;i<matrix.linkCount;i++)
			SZ[i] = PxVec3(0);

		Cm::SpatialVector Z = -impulse;

		for(;linkID!=0; linkID = matrix.parent[linkID])
			Z = PxcArticulationRef::propagateImpulse(rows[linkID], jointVectors[linkID], SZ[linkID], Z, aux[linkID]);

		dV[0] = Fns::getRootDeltaV(matrix,-Z);

		for(PxU32 i=1;i<matrix.linkCount; i++)
			dV[i] = PxcArticulationRef::propagateVelocity(rows[i], jointVectors[i], SZ[i], dV[matrix.parent[i]], aux[i]);

		for(PxU32 i=0;i<matrix.linkCount;i++)
			velocity[i] += dV[i];
	}

	void ltbFactor(PxcFsData& m)
	{
		typedef PxcArticulationFnsScalar Fns;
		PxcLtbRow* rows = getLtbRows(m);

		SpInertia inertia[PXC_ARTICULATION_MAX_SIZE];
		for(PxU32 i=0;i<m.linkCount;i++)
			inertia[i] = PxcArticulationFnsDebug::unsimdify(rows[i].inertia);

		Cm::SpatialVector j[3];
		for(PxU32 i=m.linkCount; --i>0;)
		{
			PxcLtbRow& b = rows[i];
			inertia[i] = Fns::invertInertia(inertia[i]);
			PxU32 p = m.parent[i];

			Cm::SpatialVector* j0 = &reinterpret_cast<Cm::SpatialVector&>(*b.j0),
							 * j1 = &reinterpret_cast<Cm::SpatialVector&>(*b.j1);

			Fns::multiply(j, inertia[i], j1);
			PxMat33 jResponse = Fns::invertSym33(-Fns::multiplySym(j, j1));
			j1[0] = j[0]; j1[1] = j[1]; j1[2] = j[2];

			b.jResponse = Mat33V_From_PxMat33(jResponse);
			Fns::multiply(j, j0, jResponse);
			inertia[p] = Fns::multiplySubtract(inertia[p], j, j0);
			j0[0] = j[0]; j0[1] = j[1]; j0[2] = j[2];
		}

		rows[0].inertia = Fns::invertInertia(inertia[0]);
		for(PxU32 i=1;i<m.linkCount;i++)
			rows[i].inertia = inertia[i];
	}


}

#if 0


void ltbSolve(const PxcFsData& m, 
			  Vec3V* c,					// rhs error to solve for
			  Cm::SpatialVector* y)		// velocity delta output
{
	typedef PxcArticulationFnsScalar Fns;

	PxVec4* b = reinterpret_cast<PxVec4*>(c);
	const PxcLtbRow* rows = getLtbRows(m);
	PxMemZero(y, m.linkCount*sizeof(Cm::SpatialVector));

	for(PxU32 i=m.linkCount;i-->1;)
	{
		PxU32 p = m.parent[i];
		const PxcLtbRow& r = rows[i];
		b[i] -= PxVec4(Fns::axisDot(&static_cast<const Cm::SpatialVector&>(*r.j1), y[i]),0);
		y[p] -= Fns::axisMultiply(&static_cast<const Cm::SpatialVector&>(*r.j0), b[i].getXYZ());
	}

	y[0] = Fns::multiply(rows[0].inertia,y[0]);

	for(PxU32 i=1; i<m.linkCount; i++)
	{
		PxU32 p = m.parent[i];
		const PxcLtbRow& r = rows[i];
		PxVec3 t = Fns::multiply(r.jResponse, b[i].getXYZ()) - Fns::axisDot(&static_cast<const Cm::SpatialVector&>(*r.j0), y[p]);
		y[i] = Fns::multiply(r.inertia, y[i]) - Fns::axisMultiply(&static_cast<const Cm::SpatialVector&>(*r.j1), t);
	}
}

void PxcFsPropagateDrivenInertiaScalar(PxcFsData& matrix,
								 const PxcFsInertia* baseInertia,
								 const PxReal* isf,
								 const Mat33V* load)
{
	typedef PxcArticulationFnsScalar Fns;

	Cm::SpatialVector IS[3], DSI[3];
	PxMat33 D;

	PxcFsRow* rows = getFsRows(matrix);
	const PxcFsRowAux* aux = getAux(matrix);
	const PxcFsJointVectors* jointVectors = getJointVectors(matrix);

	SpInertia inertia[PXC_ARTICULATION_MAX_SIZE];
	for(PxU32 i=0;i<matrix.linkCount;i++)
		inertia[i] = PxcArticulationFnsDebug::unsimdify(baseInertia[i]);

	for(PxU32 i=matrix.linkCount; --i>0;)
	{
		PxcFsRow& r = rows[i];
		const PxcFsRowAux& a = aux[i];
		const PxcFsJointVectors& jv = jointVectors[i];

		Fns::multiply(IS, inertia[i], &static_cast<const Cm::SpatialVector&>(*a.S));

		PX_ALIGN(16, PxMat33) L;
		PxMat33_From_Mat33V(load[i], L);
		D = Fns::invertSym33(Fns::multiplySym(&static_cast<const Cm::SpatialVector&>(*a.S), IS) + L*isf[i]);

		Fns::multiply(DSI, IS, D);

		r.D = Mat33V_From_PxMat33(D);
		static_cast<Cm::SpatialVector&>(r.DSI[0]) = DSI[0];
		static_cast<Cm::SpatialVector&>(r.DSI[1]) = DSI[1];
		static_cast<Cm::SpatialVector&>(r.DSI[2]) = DSI[2];

		inertia[matrix.parent[i]] += Fns::translate(getParentOffset(jv), Fns::multiplySubtract(inertia[i], DSI, IS));
	}

	PxcFsInertia& m = getRootInverseInertia(matrix);
	m = PxcFsInertia(Fns::invertInertia(inertia[0]));
}

// no need to compile this ecxcept for verification, and it consumes huge amounts of stack space
void PxcFsComputeJointLoadsScalar(const PxcFsData& matrix,
							      const PxcFsInertia*PX_RESTRICT baseInertia,
							      Mat33V*PX_RESTRICT load,
   							      const PxReal*PX_RESTRICT isf,
							      PxU32 linkCount,
							      PxU32 maxIterations)
{
	typedef PxcArticulationFnsScalar Fns;

	// the childward S
	SpInertia leafwardInertia[PXC_ARTICULATION_MAX_SIZE];
	SpInertia rootwardInertia[PXC_ARTICULATION_MAX_SIZE];
	SpInertia inertia[PXC_ARTICULATION_MAX_SIZE];
	SpInertia contribToParent[PXC_ARTICULATION_MAX_SIZE];

	// total articulated inertia assuming the articulation is rooted here

	const PxcFsRow* row = getFsRows(matrix);
	const PxcFsRowAux* aux = getAux(matrix);
	const PxcFsJointVectors* jointVectors = getJointVectors(matrix);

	PX_UNUSED(row);

	PxMat33 load_[PXC_ARTICULATION_MAX_SIZE];

	for(PxU32 iter=0;iter<maxIterations;iter++)
	{
		for(PxU32 i=0;i<linkCount;i++)
			inertia[i] = PxcArticulationFnsDebug::unsimdify(baseInertia[i]);
			
		for(PxU32 i=linkCount;i-->1;)
		{
			const PxcFsJointVectors& j = jointVectors[i];

			leafwardInertia[i] = inertia[i];
			contribToParent[i] = Fns::propagate(inertia[i], &static_cast<const Cm::SpatialVector&>(*aux[i].S), load_[i], isf[i]);
			inertia[matrix.parent[i]] += Fns::translate((PxVec3&)j.parentOffset, contribToParent[i]);
		}

		for(PxU32 i=1;i<linkCount;i++)
		{
			rootwardInertia[i] = Fns::translate(-(PxVec3&)jointVectors[i].parentOffset, inertia[matrix.parent[i]]) - contribToParent[i];				
			inertia[i] += Fns::propagate(rootwardInertia[i], &static_cast<const Cm::SpatialVector&>(*aux[i].S), load_[i], isf[i]);
		}

		for(PxU32 i=1;i<linkCount;i++)
		{
			load_[i] = Fns::computeDriveInertia(leafwardInertia[i], rootwardInertia[i], &static_cast<const Cm::SpatialVector&>(*aux[i].S));
			PX_ASSERT(load_[i][0].isFinite() && load_[i][1].isFinite() && load_[2][i].isFinite());
		}					
	}
	for(PxU32 i=1;i<linkCount;i++)
		load[i] = Mat33V_From_PxMat33(load_[i]);
}


void PxcFsApplyImpulse(const PxcFsData& matrix, 
					   PxU32 linkID, 
					   const Cm::SpatialVector& impulse)
{
#if PXC_ARTICULATION_DEBUG_VERIFY
	PxcFsRefApplyImpulse(matrix, state.refVelocity, linkID, impulse);
#endif

	Cm::SpatialVector Z = -impulse;

	for(PxU32 i = linkID; i!=0; i = matrix.row[i].parent)
	{
		PxVec3 SZ;
		Z = propagateImpulse(matrix.row[i], SZ, Z, matrix.aux[i]);
		deferredSZRef(state,i) += SZ;
	}

	static_cast<Cm::SpatialVector &>(state.deferredZ) += Z;
	state.dirty |= matrix.row[linkID].pathToRoot;
}

Cm::SpatialVector PxcFsGetVelocity(const PxcFsData& matrix,
								  PxU32 linkID)
{
	// find the dirty node on the path (including the root) with the lowest index
	PxcArticulationBitField toUpdate = matrix.row[linkID].pathToRoot & state.dirty;

	if(toUpdate)
	{
		PxcArticulationBitField ignoreNodes = (toUpdate & (0-toUpdate))-1;
		PxcArticulationBitField path = matrix.row[linkID].pathToRoot & ~ignoreNodes, p = path;
		PxcArticulationBitField newDirty = 0;

		Cm::SpatialVector dV = Cm::SpatialVector::zero();
		if(p & 1)
		{
			dV = getRootDeltaV(matrix, -deferredZ(state));

			velocityRef(state, 0) += dV;
			for(PxcArticulationBitField defer = matrix.row[0].children & ~path; defer; defer &= (defer-1))
				deferredVelRef(state, PxcArticulationLowestSetBit(defer)) += dV;

			deferredZRef(state) = Cm::SpatialVector::zero();
			newDirty = matrix.row[0].children;
			p--;
		}

		for(; p; p &= (p-1))
		{
			PxU32 i = PxcArticulationLowestSetBit(p);

			dV = propagateVelocity(matrix.row[i], deferredSZ(state,i), dV + state.deferredVel[i], matrix.aux[i]);

			velocityRef(state,i) += dV;
			for(PxcArticulationBitField defer = matrix.row[i].children & ~path; defer; defer &= (defer-1))
				deferredVelRef(state,PxcArticulationLowestSetBit(defer)) += dV;

			newDirty |= matrix.row[i].children;
			deferredVelRef(state,i) = Cm::SpatialVector::zero();
			deferredSZRef(state,i) = PxVec3(0);
		}

		state.dirty = (state.dirty | newDirty)&~path;
	}
#if PXC_ARTICULATION_DEBUG_VERIFY
	Cm::SpatialVector v = state.velocity[linkID];
	Cm::SpatialVector rv = state.refVelocity[linkID];
	PX_ASSERT((v-rv).magnitude()<1e-4f * rv.magnitude());
#endif

	return state.velocity[linkID];
}

void PxcFsFlushVelocity(const PxcFsData& matrix)
{
	Cm::SpatialVector V = getRootDeltaV(matrix, -deferredZ(state));
	deferredZRef(state) = Cm::SpatialVector::zero();
	velocityRef(state,0) += V;
	for(PxcArticulationBitField defer = matrix.row[0].children; defer; defer &= (defer-1))
		deferredVelRef(state,PxcArticulationLowestSetBit(defer)) += V;

	for(PxU32 i = 1; i<matrix.linkCount; i++)
	{
		Cm::SpatialVector V = propagateVelocity(matrix.row[i], deferredSZ(state,i), state.deferredVel[i], matrix.aux[i]);
		deferredVelRef(state,i) = Cm::SpatialVector::zero();
		deferredSZRef(state,i) = PxVec3(0);
		velocityRef(state,i) += V;
		for(PxcArticulationBitField defer = matrix.row[i].children; defer; defer &= (defer-1))
			deferredVelRef(state,PxcArticulationLowestSetBit(defer)) += V;
	}

	state.dirty = 0;
}

void PxcFsPropagateDrivenInertiaScalar(PxcFsData& matrix,
									   const PxcFsInertia* baseInertia,
									   const PxReal* isf,
									   const Mat33V* load,
									   PxcFsScratchAllocator allocator)
{
	typedef PxcArticulationFnsSimd<PxcArticulationFnsSimdBase> Fns;

	PxcSIMDSpatial IS[3];
	PxMat33 D;

	PxcFsRow* rows = getFsRows(matrix);
	const PxcFsRowAux* aux = getAux(matrix);
	const PxcFsJointVectors* jointVectors = getJointVectors(matrix);

	PxcFsInertia *inertia = allocator.alloc<PxcFsInertia>(matrix.linkCount);
	PxMemCopy(inertia, baseInertia, matrix.linkCount*sizeof(PxcFsInertia));

	for(PxU32 i=matrix.linkCount; --i>0;)
	{
		PxcFsRow& r = rows[i];
		const PxcFsRowAux& a = aux[i];
		const PxcFsJointVectors& jv = jointVectors[i];

		Mat33V m = Fns::computeSIS(inertia[i], a.S, IS);
		FloatV f = FLoad(isf[i]);

		Mat33V D = Fns::invertSym33(Mat33V(V3ScaleAdd(load[i].col0, f, m.col0),
										   V3ScaleAdd(load[i].col1, f, m.col1),
										   V3ScaleAdd(load[i].col2, f, m.col2)));
		r.D = D;

		inertia[matrix.parent[i]] = Fns::addInertia(inertia[matrix.parent[i]], 
													Fns::translateInertia(jv.parentOffset, Fns::multiplySubtract(inertia[i], D,  IS,  r.DSI)));
	}

	getRootInverseInertia(matrix) = Fns::invertInertia(inertia[0]);
}

void PxcLtbSolve(const PxcFsData& m, 
				 Vec3V* c,					// rhs error to solve for
				 Cm::SpatialVector* y)		// velocity delta output
{
	typedef PxcArticulationFnsScalar Fns;

	PxVec4* b = reinterpret_cast<PxVec4*>(c);
	const PxcLtbRow* rows = getLtbRows(m);
	PxMemZero(y, m.linkCount*sizeof(Cm::SpatialVector));

	for(PxU32 i=m.linkCount;i-->1;)
	{
		PxU32 p = m.parent[i];
		const PxcLtbRow& r = rows[i];
		b[i] -= PxVec4(Fns::axisDot(&static_cast<const Cm::SpatialVector&>(*r.j1), y[i]),0);
		y[p] -= Fns::axisMultiply(&static_cast<const Cm::SpatialVector&>(*r.j0), b[i].getXYZ());
	}

	y[0] = Fns::multiply(rows[0].inertia,y[0]);

	for(PxU32 i=1; i<m.linkCount; i++)
	{
		PxU32 p = m.parent[i];
		const PxcLtbRow& r = rows[i];
		PxVec3 t = Fns::multiply(r.jResponse, b[i].getXYZ()) - Fns::axisDot(&static_cast<const Cm::SpatialVector&>(*r.j0), y[p]);
		y[i] = Fns::multiply(r.inertia, y[i]) - Fns::axisMultiply(&static_cast<const Cm::SpatialVector&>(*r.j1), t);
	}
}


#endif


#if PXC_ARTICULATION_DEBUG_VERIFY
void PxcLtbFactorScalar(PxcFsData& m)
{
	typedef PxcArticulationFnsScalar Fns;
	PxcLtbRow* rows = getLtbRows(m);

	SpInertia inertia[PXC_ARTICULATION_MAX_SIZE];
	for(PxU32 i=0;i<m.linkCount;i++)
		inertia[i] = PxcArticulationFnsDebug::unsimdify(rows[i].inertia);

	Cm::SpatialVector j[3];
	for(PxU32 i=m.linkCount; --i>0;)
	{
		PxcLtbRow& b = rows[i];
		inertia[i] = Fns::invertInertia(inertia[i]);
		PxU32 p = m.parent[i];

		Cm::SpatialVector* j0 = &reinterpret_cast<Cm::SpatialVector&>(*b.j0),
						 * j1 = &reinterpret_cast<Cm::SpatialVector&>(*b.j1);

		Fns::multiply(j, inertia[i], j1);
		PxMat33 jResponse = Fns::invertSym33(-Fns::multiplySym(j, j1));
		j1[0] = j[0]; j1[1] = j[1]; j1[2] = j[2];

		b.jResponse = Mat33V_From_PxMat33(jResponse);
		Fns::multiply(j, j0, jResponse);
		inertia[p] = Fns::multiplySubtract(inertia[p], j, j0);
		j0[0] = j[0]; j0[1] = j[1]; j0[2] = j[2];
	}

	rows[0].inertia = Fns::invertInertia(inertia[0]);
	for(PxU32 i=1;i<m.linkCount;i++)
		rows[i].inertia = inertia[i];
}

void PxcFsPropagateDrivenInertiaScalar(PxcFsData& matrix,
									   const PxcFsInertia* baseInertia,
									   const PxReal* isf,
									   const Mat33V* load)
{
	typedef PxcArticulationFnsScalar Fns;

	Cm::SpatialVector IS[3], DSI[3];
	PxMat33 D;

	PxcFsRow* rows = getFsRows(matrix);
	const PxcFsRowAux* aux = getAux(matrix);
	const PxcFsJointVectors* jointVectors = getJointVectors(matrix);

	SpInertia inertia[PXC_ARTICULATION_MAX_SIZE];
	for(PxU32 i=0;i<matrix.linkCount;i++)
		inertia[i] = PxcArticulationFnsDebug::unsimdify(baseInertia[i]);

	for(PxU32 i=matrix.linkCount; --i>0;)
	{
		PxcFsRow& r = rows[i];
		const PxcFsRowAux& a = aux[i];
		const PxcFsJointVectors& jv = jointVectors[i];

		Fns::multiply(IS, inertia[i], &reinterpret_cast<const Cm::SpatialVector&>(*a.S));

		PX_ALIGN(16, PxMat33) L;
		PxMat33_From_Mat33V(load[i], L);
		D = Fns::invertSym33(Fns::multiplySym(&reinterpret_cast<const Cm::SpatialVector&>(*a.S), IS) + L*isf[i]);

		Fns::multiply(DSI, IS, D);

		r.D = Mat33V_From_PxMat33(D);
		reinterpret_cast<Cm::SpatialVector&>(r.DSI[0]) = DSI[0];
		reinterpret_cast<Cm::SpatialVector&>(r.DSI[1]) = DSI[1];
		reinterpret_cast<Cm::SpatialVector&>(r.DSI[2]) = DSI[2];

		inertia[matrix.parent[i]] += Fns::translate(getParentOffset(jv), Fns::multiplySubtract(inertia[i], DSI, IS));
	}

	PxcFsInertia& m = getRootInverseInertia(matrix);
	m = PxcFsInertia(Fns::invertInertia(inertia[0]));
}

// no need to compile this ecxcept for verification, and it consumes huge amounts of stack space
void PxcFsComputeJointLoadsScalar(const PxcFsData& matrix,
								  const PxcFsInertia*PX_RESTRICT baseInertia,
								  Mat33V*PX_RESTRICT load,
								  const PxReal*PX_RESTRICT isf,
								  PxU32 linkCount,
								  PxU32 maxIterations)
{
	typedef PxcArticulationFnsScalar Fns;

	// the childward S
	SpInertia leafwardInertia[PXC_ARTICULATION_MAX_SIZE];
	SpInertia rootwardInertia[PXC_ARTICULATION_MAX_SIZE];
	SpInertia inertia[PXC_ARTICULATION_MAX_SIZE];
	SpInertia contribToParent[PXC_ARTICULATION_MAX_SIZE];

	// total articulated inertia assuming the articulation is rooted here

	const PxcFsRow* row = getFsRows(matrix);
	const PxcFsRowAux* aux = getAux(matrix);
	const PxcFsJointVectors* jointVectors = getJointVectors(matrix);

	PX_UNUSED(row);

	PxMat33 load_[PXC_ARTICULATION_MAX_SIZE];

	for(PxU32 iter=0;iter<maxIterations;iter++)
	{
		for(PxU32 i=0;i<linkCount;i++)
			inertia[i] = PxcArticulationFnsDebug::unsimdify(baseInertia[i]);
			
		for(PxU32 i=linkCount;i-->1;)
		{
			const PxcFsJointVectors& j = jointVectors[i];

			leafwardInertia[i] = inertia[i];
			contribToParent[i] = Fns::propagate(inertia[i], &reinterpret_cast<const Cm::SpatialVector&>(*aux[i].S), load_[i], isf[i]);
			inertia[matrix.parent[i]] += Fns::translate((PxVec3&)j.parentOffset, contribToParent[i]);
		}

		for(PxU32 i=1;i<linkCount;i++)
		{
			rootwardInertia[i] = Fns::translate(-(PxVec3&)jointVectors[i].parentOffset, inertia[matrix.parent[i]]) - contribToParent[i];				
			inertia[i] += Fns::propagate(rootwardInertia[i], &reinterpret_cast<const Cm::SpatialVector&>(*aux[i].S), load_[i], isf[i]);
		}

		for(PxU32 i=1;i<linkCount;i++)
		{
			load_[i] = Fns::computeDriveInertia(leafwardInertia[i], rootwardInertia[i], &reinterpret_cast<const Cm::SpatialVector&>(*aux[i].S));
			PX_ASSERT(load_[i][0].isFinite() && load_[i][1].isFinite() && load_[2][i].isFinite());
		}					
	}
	for(PxU32 i=1;i<linkCount;i++)
		load[i] = Mat33V_From_PxMat33(load_[i]);
}
#endif


}
