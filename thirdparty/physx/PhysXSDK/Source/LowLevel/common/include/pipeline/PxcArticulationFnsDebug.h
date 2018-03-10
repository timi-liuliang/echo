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



#ifndef PXC_ARTICULATION_DEBUG_FNS_H
#define PXC_ARTICULATION_DEBUG_FNS_H

#include "PxcArticulationFnsScalar.h"
#include "PxcArticulationFnsSimd.h"

namespace physx
{
#if 0
	void printMomentum(const char* id, PxTransform* pose, Cm::SpatialVector* velocity, PxcFsInertia* inertia, PxU32 linkCount)
	{
		typedef PxcArticulationFnsScalar Fns;

		Cm::SpatialVector m = Cm::SpatialVector::zero();
		for(PxU32 i=0;i<linkCount;i++)
			m += Fns::translateForce(pose[i].p - pose[0].p, Fns::multiply(inertia[i], velocity[i]));
		printf("momentum (%20s): (%f, %f, %f), (%f, %f, %f)\n", id, m.linear.x, m.linear.y, m.linear.z, m.angular.x, m.angular.y, m.angular.z);
	}
#endif

class PxcArticulationFnsDebug
{
	typedef PxcArticulationFnsSimdBase SimdBase;
	typedef PxcArticulationFnsSimd<PxcArticulationFnsDebug> Simd;
	typedef PxcArticulationFnsScalar Scalar;

public:

	static PX_FORCE_INLINE	PxcFsInertia addInertia(const PxcFsInertia& in1, const PxcFsInertia& in2)
	{
		return PxcFsInertia(M33Add(in1.ll, in2.ll),
							M33Add(in1.la, in2.la),
							M33Add(in1.aa, in2.aa));
	}

	static PX_FORCE_INLINE	PxcFsInertia subtractInertia(const PxcFsInertia& in1, const PxcFsInertia& in2)
	{
		return PxcFsInertia(M33Sub(in1.ll, in2.ll),
							M33Sub(in1.la, in2.la),
							M33Sub(in1.aa, in2.aa));
	}

	static Mat33V invertSym33(const Mat33V &m)
	{
		PxMat33 n_ = Scalar::invertSym33(unsimdify(m));
		Mat33V n = SimdBase::invertSym33(m);
		compare33(n_, unsimdify(n));

		return n;
	}

	static Mat33V invSqrt(const Mat33V &m)
	{
		PxMat33 n_ = Scalar::invSqrt(unsimdify(m));
		Mat33V n = SimdBase::invSqrt(m);
		compare33(n_, unsimdify(n));

		return n;
	}



	static PxcFsInertia invertInertia(const PxcFsInertia &I)
	{
		SpInertia J_ = Scalar::invertInertia(unsimdify(I));
		PxcFsInertia J = SimdBase::invertInertia(I);
		compareInertias(J_,unsimdify(J));

		return J;
	}

	static Mat33V computeSIS(const PxcFsInertia &I, const PxcSIMDSpatial S[3], PxcSIMDSpatial*PX_RESTRICT IS)
	{
		Cm::SpatialVector IS_[3];
		Scalar::multiply(IS_, unsimdify(I), unsimdify(&S[0]));
		PxMat33 D_ = Scalar::multiplySym(IS_, unsimdify(&S[0]));

		Mat33V D = SimdBase::computeSIS(I, S, IS);

		compare33(unsimdify(D), D_);

		return D;
	}


	static PxcFsInertia multiplySubtract(const PxcFsInertia &I, const Mat33V &D, const PxcSIMDSpatial IS[3], PxcSIMDSpatial*PX_RESTRICT DSI)
	{
		Cm::SpatialVector DSI_[3];

		Scalar::multiply(DSI_, unsimdify(IS), unsimdify(D));
		SpInertia J_ = Scalar::multiplySubtract(unsimdify(I), DSI_, unsimdify(IS));

		PxcFsInertia J = SimdBase::multiplySubtract(I, D, IS, DSI);

		compareInertias(unsimdify(J), J_);

		return J;
	} 


	static PxcFsInertia multiplySubtract(const PxcFsInertia &I, const PxcSIMDSpatial S[3])
	{
		SpInertia J_ = Scalar::multiplySubtract(unsimdify(I), unsimdify(S), unsimdify(S));
		PxcFsInertia J = SimdBase::multiplySubtract(I, S);
		compareInertias(unsimdify(J), J_);
		return J;
	} 


	static PxcFsInertia translateInertia(Vec3V offset, const PxcFsInertia &I)
	{
		PxVec3 offset_;
		V3StoreU(offset, offset_);
		SpInertia J_ = Scalar::translate(offset_, unsimdify(I));
		PxcFsInertia J = SimdBase::translateInertia(offset, I);
		compareInertias(J_, unsimdify(J));

		return J;
	}


	static PX_FORCE_INLINE PxcFsInertia propagate(const PxcFsInertia &I,
												  const PxcSIMDSpatial S[3],
												  const Mat33V &load,
												  const FloatV isf)
	{
		SpInertia J_ = Scalar::propagate(unsimdify(I), unsimdify(&S[0]), unsimdify(load), unsimdify(isf));
		PxcFsInertia J = Simd::propagate(I, S, load, isf);

		compareInertias(J_, unsimdify(J));
		return J;
	}


	static PX_FORCE_INLINE Mat33V computeDriveInertia(const PxcFsInertia &I0, 
													  const	PxcFsInertia &I1, 
													  const PxcSIMDSpatial S[3])
	{
		PxMat33 m_ = Scalar::computeDriveInertia(unsimdify(I0), unsimdify(I1), unsimdify(&S[0]));
		Mat33V m = Simd::computeDriveInertia(I0, I1, S);

		compare33(m_, unsimdify(m));
		return m;
	}

	static const PxMat33 unsimdify(const Mat33V &m)
	{
		PX_ALIGN(16, PxMat33) m_;
		PxMat33_From_Mat33V(m, m_);
		return m_;
	}

	static PxReal unsimdify(const FloatV &m)
	{
		return FStore(m);
	}

	static SpInertia unsimdify(const PxcFsInertia &I)
	{
		return SpInertia (unsimdify(I.ll),
						  unsimdify(I.la),
						  unsimdify(I.aa));
	}

	static const Cm::SpatialVector* unsimdify(const PxcSIMDSpatial *S)
	{
		return reinterpret_cast<const Cm::SpatialVector*>(S);
	}


private:

	static PxReal absmax(const PxVec3& n)
	{
		return PxMax(PxAbs(n.x), PxMax(PxAbs(n.y),PxAbs(n.z)));
	}

	static PxReal norm(const PxMat33& n)
	{
		return PxMax(absmax(n.column0), PxMax(absmax(n.column1), absmax(n.column2)));
	}

	static void compare33(const PxMat33& ref, const PxMat33& n)
	{
		PxReal errNorm = norm(ref-n);	
		PX_UNUSED(errNorm);
		PX_ASSERT(errNorm <= PxMax(norm(ref)*1e-3, 1e-4));
	}

	static void compareInertias(const SpInertia& a, const SpInertia& b)
	{
		compare33(a.mLL, b.mLL);
		compare33(a.mLA, b.mLA);
		compare33(a.mAA, b.mAA);
	}


};

#if PXC_ARTICULATION_DEBUG_VERIFY
static bool isPositiveDefinite(const Mat33V& m)
{
	PX_ALIGN_PREFIX(16) PxMat33 m1 PX_ALIGN_SUFFIX(16);
	PxMat33_From_Mat33V(m, m1);
	return isPositiveDefinite(m1);
}


static bool isPositiveDefinite(const PxcFsInertia& s)
{
	return isPositiveDefinite(PxcArticulationFnsDebug::unsimdify(s));
}

static PxReal magnitude(const PxcSIMDSpatial &v)
{
	return PxSqrt(FStore(V3Dot(v.linear, v.linear)) + FStore(V3Dot(v.angular, v.angular)));
}

static bool almostEqual(const PxcSIMDSpatial &ref, const PxcSIMDSpatial& test, PxReal tolerance)
{
	return magnitude(ref-test)<=tolerance*magnitude(ref);
}
#endif
}

#endif //PXC_ARTICULATION
