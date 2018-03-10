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

#ifndef GU_CONVEX_SUPPORT_TABLE_H
#define GU_CONVEX_SUPPORT_TABLE_H

#include "GuVecConvex.h"
#include "PsVecTransform.h"
#include "PxPhysXCommonConfig.h"

namespace physx
{
namespace Gu  
{

	class TriangleV; 
	class CapsuleV;
	class BoxV;
	class ConvexHullV;
	class ShrunkConvexHullV;
	class ConvexHullNoScaleV;
	class ShrunkConvexHullNoScaleV;

	class SupportMap
	{
	public:
		virtual void doWarmStartSupport(const PxI32 index, Ps::aos::Vec3V& point) const = 0;
		virtual Ps::aos::Vec3V doSupport(const Ps::aos::Vec3VArg dir) const = 0;
		virtual Ps::aos::Vec3V doSupportSweep(const Ps::aos::Vec3VArg dir) const = 0;
        virtual ~SupportMap() {}
	};

	template <typename Convex>
	class SupportMapRelativeImpl : public SupportMap
	{
		const Convex& conv;
		const Ps::aos::PsMatTransformV& aToB;
	public:
		SupportMapRelativeImpl(const Convex& _conv, const Ps::aos::PsMatTransformV& _aToB) : conv(_conv), aToB(_aToB)
		{
		}

		void doWarmStartSupport(const PxI32 index, Ps::aos::Vec3V& point) const
		{
			point = aToB.transform(conv.supportPoint(index));
		}

		Ps::aos::Vec3V doSupport(const Ps::aos::Vec3VArg dir) const
		{
			return conv.supportRelative(dir, aToB);
		}

		Ps::aos::Vec3V doSupportSweep(const Ps::aos::Vec3VArg dir) const
		{
			return conv.supportSweepRelative(dir, aToB);
		}

	private:
		SupportMapRelativeImpl& operator=(const SupportMapRelativeImpl&);
	};

	template <typename Convex>
	class SupportMapLocalImpl : public SupportMap
	{
		const Convex& conv;
	public:

		SupportMapLocalImpl(const Convex& _conv) : conv(_conv)
		{
		}

		void doWarmStartSupport(const PxI32 index, Ps::aos::Vec3V& point) const
		{
			point = conv.supportPoint(index);
		}

		Ps::aos::Vec3V doSupport(const Ps::aos::Vec3VArg dir) const
		{
			return conv.supportLocal(dir);
		}

		Ps::aos::Vec3V doSupportSweep(const Ps::aos::Vec3VArg dir) const
		{
			return conv.supportSweepLocal(dir);
		}
	private:
		SupportMapLocalImpl& operator=(const SupportMapLocalImpl&);
	};

#if defined(PX_VC) 
    #pragma warning(push)   
	#pragma warning( disable : 4324 ) // Padding was added at the end of a structure because of a __declspec(align) value.
#endif
	class SupportLocal
	{
	public:		
		Ps::aos::Vec3V shapeSpaceCenterOfMass;
		const Ps::aos::PsTransformV& transform;
		const Ps::aos::Mat33V& vertex2Shape;
		const Ps::aos::Mat33V& shape2Vertex;
		const bool isIdentityScale;	

		SupportLocal(const Ps::aos::PsTransformV& _transform, const Ps::aos::Mat33V& _vertex2Shape, const Ps::aos::Mat33V& _shape2Vertex, const bool _isIdentityScale = true): transform(_transform), 
			vertex2Shape(_vertex2Shape), shape2Vertex(_shape2Vertex), isIdentityScale(_isIdentityScale)
		{
		}

		void setShapeSpaceCenterofMass(const Ps::aos::Vec3VArg _shapeSpaceCenterOfMass)
		{
			shapeSpaceCenterOfMass = _shapeSpaceCenterOfMass;
		}
        virtual ~SupportLocal() {}
		virtual Ps::aos::Vec3V doSupport(const Ps::aos::Vec3VArg dir) const = 0;
		virtual void doSupport(const Ps::aos::Vec3VArg dir, Ps::aos::FloatV& min, Ps::aos::FloatV& max) const = 0;
		virtual void populateVerts(const PxU8* inds, PxU32 numInds, const PxVec3* originalVerts, Ps::aos::Vec3V* verts)const = 0;
		/*virtual PxGJKStatus gjkOverlapTriangle(const Gu::TriangleV& triangle, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::FloatV& pen, Ps::aos::Vec3V& normal, PxU8* aInd, PxU8* bInd, PxU8& size) const = 0;
		virtual PxGJKStatus epaOverlapTriangle(const Gu::TriangleV& triangle, const Ps::aos::FloatVArg contactDist, Ps::aos::Vec3V& closestA, Ps::aos::Vec3V& closestB, Ps::aos::FloatV& pen, Ps::aos::Vec3V& normal, PxU8* aInd, PxU8* bInd, PxU8& size)const =0;*/
	
	protected:
		SupportLocal& operator=(const SupportLocal&);
	};
#if defined(PX_VC) 
     #pragma warning(pop) 
#endif

	template <typename Convex>
	class SupportLocalImpl : public SupportLocal
	{
		
	public:
		const Convex& conv;
		SupportLocalImpl(const Convex& _conv, const Ps::aos::PsTransformV& _transform, const Ps::aos::Mat33V& _vertex2Shape, const Ps::aos::Mat33V& _shape2Vertex, const bool _isIdentityScale = true) : 
		SupportLocal(_transform, _vertex2Shape, _shape2Vertex, _isIdentityScale), conv(_conv)
		{
		}

		Ps::aos::Vec3V doSupport(const Ps::aos::Vec3VArg dir) const
		{
			//return conv.supportVertsLocal(dir);
			return conv.supportLocal(dir);
		}

		void doSupport(const Ps::aos::Vec3VArg dir, Ps::aos::FloatV& min, Ps::aos::FloatV& max) const
		{
			return conv.supportLocal(dir, min, max);
		}

		PX_FORCE_INLINE Ps::aos::Vec3V doSupportFast(const Ps::aos::Vec3VArg dir) const
		{
			//return conv.supportVertsLocal(dir);
			return conv.supportLocal(dir);
		}

		PX_FORCE_INLINE void doSupportFast(const Ps::aos::Vec3VArg dir, Ps::aos::FloatV& min, Ps::aos::FloatV& max) const
		{
			return conv.supportLocal(dir, min, max);
		}

		void populateVerts(const PxU8* inds, PxU32 numInds, const PxVec3* originalVerts, Ps::aos::Vec3V* verts) const 
		{
			conv.populateVerts(inds, numInds, originalVerts, verts);
		}

		PX_FORCE_INLINE void populateVertsFast(const PxU8* inds, PxU32 numInds, const PxVec3* originalVerts, Ps::aos::Vec3V* verts) const 
		{
			conv.populateVerts(inds, numInds, originalVerts, verts);
		}

	protected:
		SupportLocalImpl& operator=(const SupportLocalImpl&);

	};

	template <typename Convex, typename ShrunkConvex>
	class SupportLocalShrunkImpl : public SupportLocal
	{
		SupportLocalShrunkImpl& operator=(const SupportLocalShrunkImpl&);
	public:
		const Convex& conv;
		const ShrunkConvex& shrunkConvex;
		SupportLocalShrunkImpl(const Convex& _conv, const Ps::aos::PsTransformV& _transform, const Ps::aos::Mat33V& _vertex2Shape, const Ps::aos::Mat33V& _shape2Vertex, const bool _isIdentityScale = true) : 
		SupportLocal(_transform, _vertex2Shape, _shape2Vertex, _isIdentityScale), conv(_conv), shrunkConvex((const ShrunkConvex&)_conv)
		{
		}

		Ps::aos::Vec3V doSupport(const Ps::aos::Vec3VArg dir) const
		{
			//return conv.supportVertsLocal(dir);
			return conv.supportLocal(dir);
		}

		void doSupport(const Ps::aos::Vec3VArg dir, Ps::aos::FloatV& min, Ps::aos::FloatV& max) const
		{
			return conv.supportLocal(dir, min, max);
		}

		void populateVerts(const PxU8* inds, PxU32 numInds, const PxVec3* originalVerts, Ps::aos::Vec3V* verts) const 
		{
			conv.populateVerts(inds, numInds, originalVerts, verts);
		}

	};


	class EPASupportMapPair
	{
	public:
		
		EPASupportMapPair()
		{
		}
        virtual ~EPASupportMapPair() {}
		virtual void doWarmStartSupport(const PxI32 indexA, const PxI32 indexB, Ps::aos::Vec3V& supportA, Ps::aos::Vec3V& supportB, Ps::aos::Vec3V& support) const = 0;
		virtual void doSupport(const Ps::aos::Vec3VArg dir, Ps::aos::Vec3V& supportA, Ps::aos::Vec3V& supportB, Ps::aos::Vec3V& support) const = 0;
		virtual Ps::aos::Vec3V getDir() const =0;
	};

	class GJKSupportMapPair
	{
	public:
        virtual ~GJKSupportMapPair() {}
		virtual void doWarmStartSupport(const PxI32 indexA, const PxI32 indexB, Ps::aos::Vec3V& supportA, Ps::aos::Vec3V& supportB, Ps::aos::Vec3V& support) const = 0;
		virtual void doSupport(const Ps::aos::Vec3VArg dir, PxI32& indexA, PxI32& indexB, Ps::aos::Vec3V& supportA, Ps::aos::Vec3V& supportB, Ps::aos::Vec3V& support) const = 0;
		virtual Ps::aos::Vec3V doSupportSweep(const Ps::aos::Vec3VArg dir, const Ps::aos::Vec3VArg x, Ps::aos::Vec3V& supportA, Ps::aos::Vec3V& supportB) const = 0;
		virtual Ps::aos::Vec3V doSupportSweepOnB(const Ps::aos::Vec3VArg nvNorm, const Ps::aos::Vec3VArg x) const = 0;
		virtual Ps::aos::Vec3V getDir() const =0;
		virtual Ps::aos::Vec3V getSweepMargin() const = 0;
	};


	template <typename ConvexA, typename ConvexB>
	class EPASupportMapPairRelativeImpl : public EPASupportMapPair
	{
		const ConvexA& convA;
		const ConvexB& convB;
		const Ps::aos::PsMatTransformV& aToB;

	public:

		EPASupportMapPairRelativeImpl(const ConvexA& _convA, const ConvexB& _convB, const Ps::aos::PsMatTransformV& _aToB) : convA(_convA), convB(_convB), aToB(_aToB)
		{
		}

		void doWarmStartSupport(const PxI32 indexA, const PxI32 indexB, Ps::aos::Vec3V& supportA, Ps::aos::Vec3V& supportB, Ps::aos::Vec3V& support) const
		{
			using namespace Ps::aos;
			const Vec3V _sa = aToB.transform(convA.supportPoint(indexA));
			const Vec3V _sb = convB.supportPoint(indexB);
			supportA = _sa;
			supportB = _sb;
			support = V3Sub(_sa, _sb);
		}

		void doSupport(const Ps::aos::Vec3VArg dir, Ps::aos::Vec3V& supportA, Ps::aos::Vec3V& supportB, Ps::aos::Vec3V& support) const
		{
			using namespace Ps::aos;
			const Vec3V _sa = convA.supportRelative(V3Neg(dir), aToB);
			const Vec3V _sb = convB.supportLocal(dir);
			supportA = _sa;
			supportB = _sb;
			support = V3Sub(_sa, _sb);
		}

		//called by spu gjk code
		void doSupport(const Ps::aos::Vec3VArg dir, PxI32& indexA, PxI32& indexB, Ps::aos::Vec3V& supportA, Ps::aos::Vec3V& supportB, Ps::aos::Vec3V& support) const
		{
			using namespace Ps::aos;
			convA.supportRelative(V3Neg(dir), aToB, supportA, indexA);
			convB.supportLocal(dir, supportB, indexB);
			support = V3Sub(supportA, supportB);
		}

		Ps::aos::Vec3V getDir()const 
		{
			return aToB.p;
		}

	private:
		EPASupportMapPairRelativeImpl& operator=(const EPASupportMapPairRelativeImpl&);
	};  

	template <typename ConvexA, typename ConvexB>
	class EPASupportMapPairLocalImpl : public EPASupportMapPair
	{
		const ConvexA& convA;
		const ConvexB& convB;

	public:

		EPASupportMapPairLocalImpl(const ConvexA& _convA, const ConvexB& _convB) : convA(_convA), convB(_convB)
		{
		}

		void doWarmStartSupport(const PxI32 indexA, const PxI32 indexB, Ps::aos::Vec3V& supportA, Ps::aos::Vec3V& supportB, Ps::aos::Vec3V& support) const
		{
			using namespace Ps::aos;
			const Vec3V _sa = convA.supportPoint(indexA);
			const Vec3V _sb = convB.supportPoint(indexB);
			supportA = _sa;
			supportB = _sb;
			support = V3Sub(_sa, _sb);
		}

		void doSupport(const Ps::aos::Vec3VArg dir, Ps::aos::Vec3V& supportA, Ps::aos::Vec3V& supportB, Ps::aos::Vec3V& support) const
		{
			using namespace Ps::aos;
			const Vec3V _sa = convA.supportLocal(V3Neg(dir));
			const Vec3V _sb = convB.supportLocal(dir);
			supportA = _sa;
			supportB = _sb;
			support = V3Sub(_sa, _sb);
		}

		//called by spu gjk code
		void doSupport(const Ps::aos::Vec3VArg dir, PxI32& indexA, PxI32& indexB, Ps::aos::Vec3V& supportA, Ps::aos::Vec3V& supportB, Ps::aos::Vec3V& support) const
		{
			using namespace Ps::aos;
			convA.supportLocal(V3Neg(dir), supportA, indexA);
			convB.supportLocal(dir, supportB, indexB);
			support = V3Sub(supportA, supportB);
		}


		Ps::aos::Vec3V getDir()const
		{
			return Ps::aos::V3Sub(convA.getCenter(), convB.getCenter());
		}

	private:
		EPASupportMapPairLocalImpl& operator=(const EPASupportMapPairLocalImpl&);

	};


	template <typename ConvexA, typename ConvexB>
	class GJKSupportMapPairRelativeImpl : public GJKSupportMapPair
	{
		const ConvexA& convA;
		const ConvexB& convB;
		const Ps::aos::PsMatTransformV& aToB;

	public:

		GJKSupportMapPairRelativeImpl(const ConvexA& _convA, const ConvexB& _convB, const Ps::aos::PsMatTransformV& _aToB) : convA(_convA), convB(_convB), aToB(_aToB)
		{
		} 

		void doWarmStartSupport(const PxI32 indexA, const PxI32 indexB, Ps::aos::Vec3V& supportA, Ps::aos::Vec3V& supportB, Ps::aos::Vec3V& support) const
		{
			using namespace Ps::aos;
			const Vec3V _sa = aToB.transform(convA.supportPoint(indexA));
			const Vec3V _sb = convB.supportPoint(indexB);
			supportA = _sa;
			supportB = _sb;
			support = V3Sub(_sa, _sb);
		}

		void doSupport(const Ps::aos::Vec3VArg dir, Ps::aos::Vec3V& supportA, Ps::aos::Vec3V& supportB, Ps::aos::Vec3V& support) const
		{
			using namespace Ps::aos;
			const Vec3V _sa = convA.supportRelative(V3Neg(dir), aToB);
			const Vec3V _sb = convB.supportLocal(dir);
			supportA = _sa;
			supportB = _sb;
			support = V3Sub(_sa, _sb);
		}

		//called by spu gjk code
		void doSupport(const Ps::aos::Vec3VArg dir, PxI32& indexA, PxI32& indexB, Ps::aos::Vec3V& supportA, Ps::aos::Vec3V& supportB, Ps::aos::Vec3V& support) const
		{
			using namespace Ps::aos;
			convA.supportRelative(V3Neg(dir), aToB, supportA, indexA);
			convB.supportLocal(dir, supportB, indexB);
			support = V3Sub(supportA, supportB);
		}

		Ps::aos::Vec3V doSupportSweep(const Ps::aos::Vec3VArg vNorm, const Ps::aos::Vec3VArg x, Ps::aos::Vec3V& supportA, Ps::aos::Vec3V& supportB) const
		{
			using namespace Ps::aos;
			const Vec3V nvNorm = V3Neg(vNorm);

			const Vec3V _sa = convA.supportSweepRelative(vNorm, aToB);
			const Vec3V _sb = V3Add(x, convB.supportSweepLocal(nvNorm));
			supportA = _sa;
			supportB = _sb;
			return V3Sub(_sa, _sb);
		}

		Ps::aos::Vec3V doSupportSweepOnB(const Ps::aos::Vec3VArg nvNorm, const Ps::aos::Vec3VArg x)const 
		{
			using namespace Ps::aos;
			return V3Add(x, convB.supportSweepLocal(nvNorm));
		}

		Ps::aos::Vec3V getDir()const 
		{
			return aToB.p;
		}

		virtual Ps::aos::Vec3V getSweepMargin() const{ return Ps::aos::FMin(convA.getSweepMargin(), convB.getSweepMargin()); }

	private:
		GJKSupportMapPairRelativeImpl& operator=(const GJKSupportMapPairRelativeImpl&);
	};

	template <typename ConvexA, typename ConvexB>
	class GJKSupportMapPairLocalImpl : public GJKSupportMapPair
	{
		const ConvexA& convA;
		const ConvexB& convB;

	public:

		GJKSupportMapPairLocalImpl(const ConvexA& _convA, const ConvexB& _convB) : convA(_convA), convB(_convB)
		{
		}

		void doWarmStartSupport(const PxI32 indexA, const PxI32 indexB, Ps::aos::Vec3V& supportA, Ps::aos::Vec3V& supportB, Ps::aos::Vec3V& support) const
		{
			using namespace Ps::aos;
			const Vec3V _sa = convA.supportPoint(indexA);
			const Vec3V _sb = convB.supportPoint(indexB);
			supportA = _sa;
			supportB = _sb;
			support = V3Sub(_sa, _sb);
		}

		void doSupport(const Ps::aos::Vec3VArg dir, Ps::aos::Vec3V& supportA, Ps::aos::Vec3V& supportB, Ps::aos::Vec3V& support) const
		{
			using namespace Ps::aos;
			const Vec3V _sa = convA.supportLocal(V3Neg(dir));
			const Vec3V _sb = convB.supportLocal(dir);
			supportA = _sa;
			supportB = _sb;
			support = V3Sub(_sa, _sb);
		}

		//called by spu gjk code
		void doSupport(const Ps::aos::Vec3VArg dir, PxI32& indexA, PxI32& indexB, Ps::aos::Vec3V& supportA, Ps::aos::Vec3V& supportB, Ps::aos::Vec3V& support) const
		{
			using namespace Ps::aos;
			convA.supportLocal(V3Neg(dir), supportA, indexA);
			convB.supportLocal(dir, supportB, indexB);
			support = V3Sub(supportA, supportB);
		}

		Ps::aos::Vec3V doSupportSweep(const Ps::aos::Vec3VArg vNorm, const Ps::aos::Vec3VArg x, Ps::aos::Vec3V& supportA, Ps::aos::Vec3V& supportB) const
		{
			using namespace Ps::aos;
			const Vec3V nvNorm = V3Neg(vNorm);

			const Vec3V _sa = convA.supportSweepLocal(vNorm);
			const Vec3V _sb = V3Add(x, convB.supportSweepLocal(nvNorm));
			supportA = _sa;
			supportB = _sb;
			return V3Sub(_sa, _sb);
		}

		Ps::aos::Vec3V doSupportSweepOnB(const Ps::aos::Vec3VArg nvNorm, const Ps::aos::Vec3VArg x) const
		{
			using namespace Ps::aos;
			return V3Add(x, convB.supportSweepLocal(nvNorm));
		}

		Ps::aos::Vec3V getDir()const
		{
			return Ps::aos::V3Sub(convA.getCenter(), convB.getCenter());
		}

		virtual Ps::aos::Vec3V getSweepMargin() const{ return Ps::aos::FMin(convA.getSweepMargin(), convB.getSweepMargin()); }


	private:
		GJKSupportMapPairLocalImpl& operator=(const GJKSupportMapPairLocalImpl&);
	};

}

}

#endif
