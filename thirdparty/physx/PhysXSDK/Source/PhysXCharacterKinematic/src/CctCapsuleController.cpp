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

#include "PxController.h"
#include "CctCapsuleController.h"
#include "CctCharacterControllerManager.h"
#include "PxCapsuleGeometry.h"
#include "PxRigidDynamic.h"
#include "PxShape.h"

using namespace physx;
using namespace Cct;

static PX_FORCE_INLINE float CCTtoProxyRadius(float r, PxF32 coeff)	{ return r * coeff;			}
static PX_FORCE_INLINE float CCTtoProxyHeight(float h, PxF32 coeff)	{ return 0.5f * h * coeff;	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CapsuleController::CapsuleController(const PxControllerDesc& desc, PxPhysics& sdk, PxScene* s) : Controller(desc, s)
{
	mType = PxControllerShapeType::eCAPSULE;

	const PxCapsuleControllerDesc& cc = static_cast<const PxCapsuleControllerDesc&>(desc);

	mRadius			= cc.radius;
	mHeight			= cc.height;
	mClimbingMode	= cc.climbingMode;

	// Create kinematic actor under the hood
	PxCapsuleGeometry capsGeom;
	capsGeom.radius		= CCTtoProxyRadius(cc.radius, mProxyScaleCoeff);
	capsGeom.halfHeight	= CCTtoProxyHeight(cc.height, mProxyScaleCoeff);

	createProxyActor(sdk, capsGeom, *desc.material);
}

CapsuleController::~CapsuleController()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CapsuleController::invalidateCache()
{
	if(mManager->mLockingEnabled)
	{
		mWriteLock.lock();
		mCctModule.voidTestCache();
		mWriteLock.unlock();
	}
	else
	{
		mCctModule.voidTestCache();
	}	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CapsuleController::getWorldBox(PxExtendedBounds3& box) const
{
	setCenterExtents(box, mPosition, PxVec3(mRadius, mRadius+mHeight*0.5f, mRadius));
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CapsuleController::setRadius(PxF32 r)
{
	// Set radius for CCT volume
	mRadius = r;

	// Set radius for kinematic proxy
	if(mKineActor)
	{
		PxShape* shape = getKineShape();

		PX_ASSERT(shape->getGeometryType() == PxGeometryType::eCAPSULE);
		PxCapsuleGeometry cg;
		shape->getCapsuleGeometry(cg);

		cg.radius = CCTtoProxyRadius(r, mProxyScaleCoeff);
		shape->setGeometry(cg);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CapsuleController::setHeight(PxF32 h)
{
	// Set height for CCT volume
	mHeight = h;

	// Set height for kinematic proxy
	if(mKineActor)
	{
		PxShape* shape = getKineShape();

		PX_ASSERT(shape->getGeometryType() == PxGeometryType::eCAPSULE);
		PxCapsuleGeometry cg;
		shape->getCapsuleGeometry(cg);

		cg.halfHeight = CCTtoProxyHeight(h, mProxyScaleCoeff);
		shape->setGeometry(cg);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PxCapsuleClimbingMode::Enum CapsuleController::getClimbingMode() const
{
	return mClimbingMode;
}

bool CapsuleController::setClimbingMode(PxCapsuleClimbingMode::Enum mode)
{
	if(mode>=PxCapsuleClimbingMode::eLAST)
		return false;
	mClimbingMode = mode;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PxExtendedVec3 CapsuleController::getFootPosition() const
{
	PxExtendedVec3 groundPosition = mPosition;														// Middle of the CCT
	groundPosition -= mUserParams.mUpDirection * (mUserParams.mContactOffset+mRadius+mHeight*0.5f);	// Ground
	return groundPosition;
}

bool CapsuleController::setFootPosition(const PxExtendedVec3& position)
{
	PxExtendedVec3 centerPosition = position;
	centerPosition += mUserParams.mUpDirection * (mUserParams.mContactOffset+mRadius+mHeight*0.5f);
	return setPosition(centerPosition);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CapsuleController::getCapsule(PxExtendedCapsule& capsule) const
{
	// PT: TODO: optimize this
	PxExtendedVec3 p0 = mPosition;
	PxExtendedVec3 p1 = mPosition;
	const PxVec3 extents = mUserParams.mUpDirection*mHeight*0.5f;
	p0 -= extents;
	p1 += extents;

	capsule.p0		= p0;
	capsule.p1		= p1;
	capsule.radius	= mRadius;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CapsuleController::resize(PxReal height)
{
	const float oldHeight = getHeight();
	setHeight(height);

	const float delta = height - oldHeight;
	PxExtendedVec3 pos = getPosition();
	pos += mUserParams.mUpDirection * delta * 0.5f;
	setPosition(pos);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
