/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_PREVIEW_H__
#define __APEX_PREVIEW_H__

#include "ApexRenderable.h"
#include "ApexInterface.h"
#include "PxMat34Legacy.h"


#if NX_SDK_VERSION_MAJOR == 2
class NxActorDescBase;
#endif

namespace physx
{
namespace apex
{

class ApexContext;

/**
	Class that implements preview interface
*/
class ApexPreview : public ApexRenderable
{
public:
	ApexPreview();
	virtual  						~ApexPreview();

	// Each class that derives from ApexPreview may optionally implement this function
	virtual NxApexRenderable*		getRenderable()
	{
		return NULL;
	}

	virtual void					setPose(const physx::PxMat44& pose);
	virtual const physx::PxMat44	getPose() const;

	virtual void					release() = 0;
	void							destroy();

protected:
	bool					mInRelease;

	physx::PxMat34Legacy	mPose;
};

}
} // end namespace physx::apex

#endif // __APEX_PREVIEW_H__
