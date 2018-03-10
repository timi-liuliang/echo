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


#ifndef PX_PHYSICS_COMMON_SCALING
#define PX_PHYSICS_COMMON_SCALING

#include "foundation/PxBounds3.h"
#include "geometry/PxMeshScale.h"
#include "CmMatrix34.h"
#include "PsMathUtils.h"
#include "foundation/PxMat33.h"

namespace physx
{
namespace Cm
{
	PX_CUDA_CALLABLE PX_INLINE PxBounds3 transform(const Cm::Matrix34& matrix, const PxBounds3& bounds)
	{
		if( bounds.isEmpty() )
			return bounds;
		else
		{
			PxMat33 M(matrix.base0, matrix.base1, matrix.base2);
			return PxBounds3::basisExtent(matrix.transform( bounds.getCenter()), M, bounds.getExtents());
		}
	}

	// class that can perform scaling fast.  Relatively large size, generated from PxMeshScale on demand.
	// CS: I've removed most usages of this class, because most of the time only one-way transform is needed.
	// If you only need a temporary FastVertex2ShapeScaling, setup your transform as PxMat34Legacy and use
	// normal matrix multiplication or a transform() overload to convert points and bounds between spaces.
	class FastVertex2ShapeScaling
	{
	public:
		PX_INLINE FastVertex2ShapeScaling()
		{
			//no scaling by default:
			vertex2ShapeSkew = PxMat33(PxIdentity);
			shape2VertexSkew = PxMat33(PxIdentity);
		}

		PX_INLINE explicit FastVertex2ShapeScaling(const PxMeshScale& scale)
		{
			init(scale);
		}

		PX_INLINE FastVertex2ShapeScaling(const PxVec3& scale, const PxQuat& rotation)
		{
			init(scale, rotation);
		}

		PX_INLINE void init(const PxMeshScale& scale)
		{
			init(scale.scale, scale.rotation);
		}

		PX_INLINE void setIdentity()
		{
			vertex2ShapeSkew = PxMat33(PxIdentity);
			shape2VertexSkew = PxMat33(PxIdentity);
		}

		PX_INLINE void init(const PxVec3& scale, const PxQuat& rotation)
		{
			// TODO: may want to optimize this for cases where we have uniform or axis aligned scaling!
			// That would introduce branches and it's unclear to me whether that's faster than just doing the math.
			// Lazy computation would be another option, at the cost of introducing even more branches.

			PxMat33 R(rotation);
			vertex2ShapeSkew = R.getTranspose();
			PxMat33 diagonal = PxMat33::createDiagonal(scale);
			vertex2ShapeSkew = vertex2ShapeSkew * diagonal;
			vertex2ShapeSkew = vertex2ShapeSkew * R;

			/*
			The inverse, is, explicitly:
			shape2VertexSkew.setTransposed(R);
			shape2VertexSkew.multiplyDiagonal(PxVec3(1.0f/scale.x, 1.0f/scale.y, 1.0f/scale.z));
			shape2VertexSkew *= R;

			It may be competitive to compute the inverse -- though this has a branch in it:
			*/
			
			shape2VertexSkew = vertex2ShapeSkew.getInverse();			
		}

		PX_FORCE_INLINE PxVec3 operator*  (const PxVec3& src) const
		{
			return vertex2ShapeSkew * src;
		}
		PX_FORCE_INLINE PxVec3 operator%  (const PxVec3& src) const
		{
			return shape2VertexSkew * src;
		}

		PX_FORCE_INLINE const PxMat33& getVertex2ShapeSkew() const
		{
			return vertex2ShapeSkew;
		}

		PX_FORCE_INLINE const PxMat33& getShape2VertexSkew() const
		{
			return shape2VertexSkew;
		}


		PX_INLINE Cm::Matrix34 getVertex2WorldSkew(const Cm::Matrix34& shape2world) const
		{
			Cm::Matrix34 vertex2worldSkew = shape2world * getVertex2ShapeSkew();
			//vertex2worldSkew = shape2world * [vertex2shapeSkew, 0]
			//[aR at] * [bR bt] = [aR * bR		aR * bt + at]  NOTE: order of operations important so it works when this ?= left ?= right.
			return vertex2worldSkew;
		}

		PX_INLINE Cm::Matrix34 getWorld2VertexSkew(const Cm::Matrix34& shape2world) const
		{
			//world2vertexSkew = shape2vertex * invPQ(shape2world)
			//[aR 0] * [bR' -bR'*bt] = [aR * bR'		-aR * bR' * bt + 0]

			PxMat33 rotate( shape2world[0], shape2world[1], shape2world[2] );
			PxMat33 M = getShape2VertexSkew() * rotate.getTranspose();
			return Cm::Matrix34(M[0], M[1], M[2], -M * shape2world[3]);			
		}
	
		//! Transforms bounds first by this scale transform and then by absPose.  Useful for computing the bounds of a scaled object in world space.
		PX_INLINE PxBounds3 transformBounds(const PxBounds3& bounds, const PxTransform& shape2World) const
		{
			return transform(getVertex2WorldSkew(Cm::Matrix34(shape2World)), bounds);
		}

		//! same as transformBounds with absPose == id.
		PX_INLINE PxBounds3 transformBounds(const PxBounds3& bounds) const
		{
			PX_ASSERT(!bounds.isEmpty());
			return PxBounds3::transformFast(vertex2ShapeSkew, bounds);
		}

		//! Transforms a shape space OBB to a vertex space OBB.  All 3 params are in and out.
		void transformQueryBounds(PxVec3& center, PxVec3& extents, PxMat33& basis)	const
		{
			basis.column0 = shape2VertexSkew * (basis.column0 * extents.x);
			basis.column1 = shape2VertexSkew * (basis.column1 * extents.y);
			basis.column2 = shape2VertexSkew * (basis.column2 * extents.z);

			center = shape2VertexSkew * center;
			extents = Ps::optimizeBoundingBox(basis);
		}

		//! Transforms a shape space AABB to a vertex space AABB (conservative). Both parameters are in and out
		void transformQueryBounds(PxVec3& center, PxVec3& extents)	const
		{
			PxBounds3 bounds = PxBounds3::basisExtent(shape2VertexSkew * center, shape2VertexSkew, extents);
			center = bounds.getCenter();
			extents = bounds.getExtents();
		}

		void transformPlaneToShapeSpace(const PxVec3& nIn, const PxReal dIn, PxVec3& nOut, PxReal& dOut)	const
		{
			const PxVec3 tmp = shape2VertexSkew.transformTranspose(nIn);
			const PxReal Denom = 1.0f / tmp.magnitude();
			nOut = tmp * Denom;
			dOut = dIn * Denom;
		}

	private:
		PxMat33 vertex2ShapeSkew;
		PxMat33 shape2VertexSkew;
	};


} // namespace Cm


PX_INLINE Cm::Matrix34 operator*(const PxTransform& transform, const PxMeshScale& scale) 
{
	return Cm::Matrix34(PxMat33(transform.q) * scale.toMat33(), transform.p);
}

PX_INLINE Cm::Matrix34 operator*(const PxMeshScale& scale, const PxTransform& transform) 
{
	PxMat33 scaleMat = scale.toMat33();
	PxMat33 t = PxMat33(transform.q);
	PxMat33 r = scaleMat * t;
	PxVec3 p = scaleMat * transform.p;
	return Cm::Matrix34(r, p);
}

PX_INLINE Cm::Matrix34 operator*(const Cm::Matrix34& transform, const PxMeshScale& scale) 
{
	PxMat33 scaleMat = scale.toMat33();
	PxMat33 rotate( transform.base0, transform.base1, transform.base2 );
	return Cm::Matrix34(rotate* scaleMat, transform.base3);
}

PX_INLINE Cm::Matrix34 operator*(const PxMeshScale& scale, const Cm::Matrix34& transform) 
{
	PxMat33 scaleMat = scale.toMat33();
	PxMat33 rotate( transform.base0, transform.base1, transform.base2 );
	return Cm::Matrix34(scaleMat * rotate, scaleMat * transform.base3);
}

}

#endif
