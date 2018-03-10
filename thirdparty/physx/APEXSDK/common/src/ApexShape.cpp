/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexShape.h"
#include "NxApexRenderDebug.h"

#include "PsMathUtils.h"

namespace physx
{
namespace apex
{

bool isBoundsEmpty(physx::PxBounds3 bounds)
{
	return bounds.minimum.x >= bounds.maximum.x || bounds.minimum.y >= bounds.maximum.y || bounds.minimum.z >= bounds.maximum.z;
}


ApexSphereShape::ApexSphereShape()
{
	mRadius = 0.5;
	mTransform4x4 = physx::PxMat44::createIdentity();
	mOldTransform4x4 = physx::PxMat44::createIdentity();
	calculateAABB();
}

void ApexSphereShape::setRadius(physx::PxF32 radius)
{
	mRadius = radius;
	calculateAABB();
};

void ApexSphereShape::setPose(physx::PxMat44 pose)
{
	mOldTransform4x4 = mTransform4x4;
	mTransform4x4 = pose;
	calculateAABB();
};

void ApexSphereShape::calculateAABB()
{
	//find extreme points of the (transformed by mPose and mTranslation) sphere and construct an AABB from it
	PxVec3 points[6];
	points[0] = PxVec3( mRadius, 0.0f, 0.0f);
	points[1] = PxVec3(-mRadius, 0.0f, 0.0f);
	points[2] = PxVec3(0.0f,  mRadius, 0.0f);
	points[3] = PxVec3(0.0f, -mRadius, 0.0f);
	points[4] = PxVec3(0.0f, 0.0f,  mRadius);
	points[5] = PxVec3(0.0f, 0.0f, -mRadius);

	PxVec3 maxBounds(0.0f), minBounds(0.0f);
	for (int i = 0; i < 6; i++)
	{
		PxVec3 tempPoint = mTransform4x4.transform(points[i]);
		if (i == 0)
		{
			minBounds = maxBounds = tempPoint;
		}
		else
		{
			if (tempPoint.x < minBounds.x)
			{
				minBounds.x = tempPoint.x;
			}
			if (tempPoint.x > maxBounds.x)
			{
				maxBounds.x = tempPoint.x;
			}
			if (tempPoint.y < minBounds.y)
			{
				minBounds.y = tempPoint.y;
			}
			if (tempPoint.y > maxBounds.y)
			{
				maxBounds.y = tempPoint.y;
			}
			if (tempPoint.z < minBounds.z)
			{
				minBounds.z = tempPoint.z;
			}
			if (tempPoint.z > maxBounds.z)
			{
				maxBounds.z = tempPoint.z;
			}
		}
	}
	mBounds = physx::PxBounds3(minBounds, maxBounds);
}

bool ApexSphereShape::intersectAgainstAABB(physx::PxBounds3 bounds)
{
	if (!isBoundsEmpty(mBounds) && !isBoundsEmpty(bounds)) //if the bounds have some real volume then check bounds intersection
	{
		return bounds.intersects(mBounds);
	}
	else
	{
		return false;    //if the bounds had no volume then return false
	}
}

void ApexSphereShape::visualize(NxApexRenderDebug* renderer) const
{
	//physx::PxMat44 scale(physx::PxVec4(mRadius, 1));
	renderer->debugOrientedSphere(mRadius, 2, mTransform4x4);
}

ApexCapsuleShape::ApexCapsuleShape()
{
	mRadius = 1.0f;
	mHeight = 1.0f;
	mTransform4x4 = physx::PxMat44::createIdentity();
	mOldTransform4x4 = physx::PxMat44::createIdentity();
	calculateAABB();
}

void ApexCapsuleShape::setDimensions(physx::PxF32 height, physx::PxF32 radius)
{
	mRadius = radius;
	mHeight = height;
	calculateAABB();
};

void ApexCapsuleShape::setPose(physx::PxMat44 pose)
{
	mOldTransform4x4 = mTransform4x4;
	mTransform4x4 = pose;
	calculateAABB();
};

void ApexCapsuleShape::calculateAABB()
{
	//find extreme points of the (transformed by mPose and mTranslation) capsule and construct an AABB from it
	PxVec3 points[6];
	points[0] = PxVec3(mRadius, 0.0f, 0.0f);
	points[1] = PxVec3(-mRadius, 0.0f, 0.0f);
	points[2] = PxVec3(0.0f, mRadius + mHeight / 2.0f, 0.0f);
	points[3] = PxVec3(0.0f, -(mRadius + mHeight / 2.0f), 0.0f);
	points[4] = PxVec3(0.0f, 0.0f, mRadius);
	points[5] = PxVec3(0.0f, 0.0f, -mRadius);

	PxVec3 maxBounds(0.0f), minBounds(0.0f);
	for (int i = 0; i < 6; i++)
	{
		PxVec3 tempPoint = mTransform4x4.transform(points[i]);
		if (i == 0)
		{
			minBounds = maxBounds = tempPoint;
		}
		else
		{
			if (tempPoint.x < minBounds.x)
			{
				minBounds.x = tempPoint.x;
			}
			if (tempPoint.x > maxBounds.x)
			{
				maxBounds.x = tempPoint.x;
			}
			if (tempPoint.y < minBounds.y)
			{
				minBounds.y = tempPoint.y;
			}
			if (tempPoint.y > maxBounds.y)
			{
				maxBounds.y = tempPoint.y;
			}
			if (tempPoint.z < minBounds.z)
			{
				minBounds.z = tempPoint.z;
			}
			if (tempPoint.z > maxBounds.z)
			{
				maxBounds.z = tempPoint.z;
			}
		}
	}
	mBounds = physx::PxBounds3(minBounds, maxBounds);
}

bool ApexCapsuleShape::intersectAgainstAABB(physx::PxBounds3 bounds)
{
	if (!isBoundsEmpty(mBounds) && !isBoundsEmpty(bounds)) //if the bounds have some real volume then check bounds intersection
	{
		return bounds.intersects(mBounds);
	}
	else
	{
		return false;    //if the bounds had no volume then return false
	}
}

void ApexCapsuleShape::visualize(NxApexRenderDebug* renderer) const
{
	renderer->debugOrientedCapsule(mRadius, mHeight, 2, mTransform4x4);
}

ApexBoxShape::ApexBoxShape()
{
	mSize = physx::PxVec3(1, 1, 1);
	mTransform4x4 = physx::PxMat44::createIdentity();
	mOldTransform4x4 = physx::PxMat44::createIdentity();
	calculateAABB();
}

void ApexBoxShape::setSize(physx::PxVec3 size)
{
	mSize = size;
	calculateAABB();
};

void ApexBoxShape::setPose(physx::PxMat44 pose)
{
	mOldTransform4x4 = mTransform4x4;
	mTransform4x4 = pose;
	calculateAABB();
};

void ApexBoxShape::calculateAABB()
{
	//find extreme points of the (transformed by mPose and mTranslation) box and construct an AABB from it
	PxVec3 points[8];
	PxVec3 sizeHalf = PxVec3(mSize.x / 2.0f, mSize.y / 2.0f, mSize.z / 2.0f);
	points[0] = PxVec3(sizeHalf.x, sizeHalf.y, sizeHalf.z);
	points[1] = PxVec3(-sizeHalf.x, sizeHalf.y, sizeHalf.z);
	points[2] = PxVec3(sizeHalf.x, -sizeHalf.y, sizeHalf.z);
	points[3] = PxVec3(sizeHalf.x, sizeHalf.y, -sizeHalf.z);
	points[4] = PxVec3(sizeHalf.x, -sizeHalf.y, -sizeHalf.z);
	points[5] = PxVec3(-sizeHalf.x, -sizeHalf.y, sizeHalf.z);
	points[6] = PxVec3(-sizeHalf.x, sizeHalf.y, -sizeHalf.z);
	points[7] = PxVec3(-sizeHalf.x, -sizeHalf.y, -sizeHalf.z);

	PxVec3 maxBounds(0.0f), minBounds(0.0f);
	for (int i = 0; i < 8; i++)
	{
		PxVec3 tempPoint = mTransform4x4.transform(points[i]);
		if (i == 0)
		{
			minBounds = maxBounds = tempPoint;
		}
		else
		{
			if (tempPoint.x < minBounds.x)
			{
				minBounds.x = tempPoint.x;
			}
			if (tempPoint.x > maxBounds.x)
			{
				maxBounds.x = tempPoint.x;
			}
			if (tempPoint.y < minBounds.y)
			{
				minBounds.y = tempPoint.y;
			}
			if (tempPoint.y > maxBounds.y)
			{
				maxBounds.y = tempPoint.y;
			}
			if (tempPoint.z < minBounds.z)
			{
				minBounds.z = tempPoint.z;
			}
			if (tempPoint.z > maxBounds.z)
			{
				maxBounds.z = tempPoint.z;
			}
		}
	}
	mBounds = physx::PxBounds3(minBounds, maxBounds);
}

bool ApexBoxShape::intersectAgainstAABB(physx::PxBounds3 bounds)
{
	if (!isBoundsEmpty(mBounds) && !isBoundsEmpty(bounds)) //if the bounds have some real volume then check bounds intersection
	{
		return bounds.intersects(mBounds);
	}
	else
	{
		return false;    //if the bounds had no volume then return false
	}
}

void ApexBoxShape::visualize(NxApexRenderDebug* renderer) const
{
	PxVec3 halfSize = mSize / 2;
	renderer->debugOrientedBound(-halfSize, halfSize, mTransform4x4);
}

bool ApexHalfSpaceShape::isPointInside(physx::PxVec3 pos)
{
	physx::PxVec3 vecToPos = pos - mOrigin;
	if (mNormal.dot(vecToPos) < 0.0f)
	{
		return true;
	}
	return false;
}

//if any of the corners of the bounds is inside then the bounds is inside
bool ApexHalfSpaceShape::intersectAgainstAABB(physx::PxBounds3 bounds)
{
	PxVec3 center = bounds.getCenter();
	PxVec3 sizeHalf = bounds.getDimensions() / 2.0f;

	if (isPointInside(physx::PxVec3(center.x + sizeHalf.x, center.y + sizeHalf.y, center.z + sizeHalf.z)))
	{
		return true;
	}
	if (isPointInside(physx::PxVec3(center.x - sizeHalf.x, center.y + sizeHalf.y, center.z + sizeHalf.z)))
	{
		return true;
	}
	if (isPointInside(physx::PxVec3(center.x + sizeHalf.x, center.y - sizeHalf.y, center.z + sizeHalf.z)))
	{
		return true;
	}
	if (isPointInside(physx::PxVec3(center.x + sizeHalf.x, center.y + sizeHalf.y, center.z - sizeHalf.z)))
	{
		return true;
	}
	if (isPointInside(physx::PxVec3(center.x + sizeHalf.x, center.y - sizeHalf.y, center.z - sizeHalf.z)))
	{
		return true;
	}
	if (isPointInside(physx::PxVec3(center.x - sizeHalf.x, center.y - sizeHalf.y, center.z + sizeHalf.z)))
	{
		return true;
	}
	if (isPointInside(physx::PxVec3(center.x - sizeHalf.x, center.y + sizeHalf.y, center.z - sizeHalf.z)))
	{
		return true;
	}
	if (isPointInside(physx::PxVec3(center.x - sizeHalf.x, center.y - sizeHalf.y, center.z - sizeHalf.z)))
	{
		return true;
	}

	return false;

}

ApexHalfSpaceShape::ApexHalfSpaceShape()
{
	mOrigin = mPreviousOrigin = mNormal = mPreviousNormal = physx::PxVec3(0, 0, 0);
}

void ApexHalfSpaceShape::setOriginAndNormal(physx::PxVec3 origin, physx::PxVec3 normal)
{
	mPreviousOrigin = mOrigin;
	mOrigin = origin;
	mPreviousNormal = mNormal;
	mNormal = normal;
};

void ApexHalfSpaceShape::visualize(NxApexRenderDebug* renderer) const
{
	physx::PxF32 radius = 2 * mNormal.magnitude();
	renderer->debugPlane(mNormal, - mOrigin.dot(mNormal), radius, radius);
	renderer->debugRay(mOrigin, mOrigin + mNormal);
}

static PX_INLINE physx::PxMat44 OriginAndNormalToPose(const physx::PxVec3& origin, const physx::PxVec3& normal)
{
	return physx::PxMat44(rotFrom2Vectors(physx::PxVec3(0, 1, 0), normal), origin);
}

physx::PxMat44 ApexHalfSpaceShape::getPose() const
{
	return OriginAndNormalToPose(mOrigin, mNormal);
}

physx::PxMat44 ApexHalfSpaceShape::getPreviousPose() const
{
	return OriginAndNormalToPose(mPreviousOrigin, mPreviousNormal);
}

}
} // end namespace physx::apex

