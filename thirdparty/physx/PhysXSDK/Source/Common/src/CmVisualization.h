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


#ifndef PX_PHYSICS_COMMON_VISUALIZATION
#define PX_PHYSICS_COMMON_VISUALIZATION

#include "CmPhysXCommon.h"
#include "PxTransform.h"
#include "PxConstraintDesc.h"

namespace physx
{

class RenderOutput;

namespace Cm
{
	PX_PHYSX_COMMON_API void visualizeJointFrames(RenderOutput& out,
							  PxReal scale,
							  const PxTransform& parent,
							  const PxTransform& child);

	PX_PHYSX_COMMON_API void visualizeLinearLimit(RenderOutput& out,
							  PxReal scale,
							  const PxTransform& t0,
							  const PxTransform& t1,
							  PxReal value,
							  bool active);

	PX_PHYSX_COMMON_API void visualizeAngularLimit(RenderOutput& out,
							   PxReal scale,
							   const PxTransform& t0,
							   PxReal lower,
							   PxReal upper,
							   bool active);


	PX_PHYSX_COMMON_API void visualizeLimitCone(RenderOutput& out,
							PxReal scale,
							const PxTransform& t,
							PxReal ySwing,
							PxReal zSwing,
							bool active);

	PX_PHYSX_COMMON_API void visualizeDoubleCone(RenderOutput& out,
							 PxReal scale,
							 const PxTransform& t,
							 PxReal angle,
							 bool active);
	

	struct ConstraintImmediateVisualizer : public PxConstraintVisualizer
	{
		PxF32			mFrameScale;
		PxF32			mLimitScale;
		RenderOutput&	mCmOutput;

		//Not possible to implement
		ConstraintImmediateVisualizer& operator=( const ConstraintImmediateVisualizer& );

		ConstraintImmediateVisualizer( PxF32 _frameScale, PxF32 _limitScale, RenderOutput& _output )
			: mFrameScale( _frameScale )
			, mLimitScale( _limitScale )
			//, mCmOutput(static_cast<RenderBuffer &>(_output))
			, mCmOutput( _output )
		{
		}

		virtual void visualizeJointFrames( const PxTransform& parent, const PxTransform& child )
		{
			Cm::visualizeJointFrames(mCmOutput, mFrameScale, parent, child );
		}

		virtual void visualizeLinearLimit( const PxTransform& t0, const PxTransform& t1, PxReal value, bool active )
		{
			Cm::visualizeLinearLimit( mCmOutput, mLimitScale, t0, t1, value, active );
		}

		virtual void visualizeAngularLimit( const PxTransform& t0, PxReal lower, PxReal upper, bool active)
		{
			Cm::visualizeAngularLimit( mCmOutput, mLimitScale, t0, lower, upper, active );
		}

		virtual void visualizeLimitCone( const PxTransform& t, PxReal ySwing, PxReal zSwing, bool active)
		{
			Cm::visualizeLimitCone( mCmOutput, mLimitScale, t, ySwing, zSwing, active );
		}

		virtual void visualizeDoubleCone( const PxTransform& t, PxReal angle, bool active)
		{
			Cm::visualizeDoubleCone( mCmOutput, mLimitScale, t, angle, active );
		}
	};
}

}

#endif
