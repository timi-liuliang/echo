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

#ifndef CCT_CAPSULE_CONTROLLER
#define CCT_CAPSULE_CONTROLLER

/* Exclude from documentation */
/** \cond */

#include "CctController.h"
#include "PxCapsuleController.h"

namespace physx
{

class PxPhysics;

namespace Cct
{

	class CapsuleController : public PxCapsuleController, public Controller
	{
	public:
													CapsuleController(const PxControllerDesc& desc, PxPhysics& sdk, PxScene* scene);
		virtual										~CapsuleController();

		// Controller
		virtual	PxF32								getHalfHeightInternal()				const					{ return mRadius+mHeight*0.5f;			}
		virtual	bool								getWorldBox(PxExtendedBounds3& box) const;
		virtual	PxController*						getPxController()											{ return this;							}
		//~Controller

		// PxController
		virtual	PxControllerShapeType::Enum			getType()							const					{ return mType;							}
		virtual void								release()													{ releaseInternal();					}
		virtual	PxControllerCollisionFlags			move(const PxVec3& disp, PxF32 minDist, PxF32 elapsedTime, const PxControllerFilters& filters, const PxObstacleContext* obstacles);
		virtual	bool								setPosition(const PxExtendedVec3& position)					{ return setPos(position);				}
		virtual	const PxExtendedVec3&				getPosition()						const					{ return mPosition;						}
		virtual	bool								setFootPosition(const PxExtendedVec3& position);
		virtual	PxExtendedVec3						getFootPosition()					const;
		virtual	PxRigidDynamic*						getActor()							const					{ return mKineActor;					}
		virtual	void								setStepOffset(const float offset)							{ if(offset>0.0f)
																													mUserParams.mStepOffset = offset;	}
		virtual	PxF32								getStepOffset()						const					{ return mUserParams.mStepOffset;		}
		virtual	void								setNonWalkableMode(PxControllerNonWalkableMode::Enum flag)	{ mUserParams.mNonWalkableMode = flag;	}
		virtual	PxControllerNonWalkableMode::Enum	getNonWalkableMode()				const					{ return mUserParams.mNonWalkableMode;	}
		virtual PxF32								getContactOffset()					const					{ return mUserParams.mContactOffset;	}
		virtual	void								setContactOffset(PxF32 offset)								{ if(offset>0.0f)
																													mUserParams.mContactOffset = offset;}
		virtual PxVec3								getUpDirection()					const					{ return mUserParams.mUpDirection;		}
		virtual	void								setUpDirection(const PxVec3& up)							{ setUpDirectionInternal(up);			}
		virtual PxF32								getSlopeLimit()						const					{ return mUserParams.mSlopeLimit;		}
		virtual void								setSlopeLimit(PxF32 slopeLimit)								{ if(slopeLimit>0.0f)
																													mUserParams.mSlopeLimit = slopeLimit;}
		virtual	void								invalidateCache();
		virtual	PxScene*							getScene()													{ return mScene;						}
		virtual	void*								getUserData()						const					{ return mUserData;						}
		virtual	void								setUserData(void* userData)									{ mUserData = userData;					}
		virtual	void								getState(PxControllerState& state)	const					{ return getInternalState(state);		}
		virtual	void								getStats(PxControllerStats& stats)	const					{ return getInternalStats(stats);		}
		virtual	void								resize(PxReal height);
		//~PxController

		// PxCapsuleController
		virtual	PxF32						getRadius()									const					{ return mRadius;						}
		virtual	PxF32						getHeight()									const					{ return mHeight;						}
		virtual	PxCapsuleClimbingMode::Enum	getClimbingMode()							const;
		virtual	bool						setRadius(PxF32 radius);
		virtual	bool						setHeight(PxF32 height);
		virtual	bool						setClimbingMode(PxCapsuleClimbingMode::Enum);
		//~ PxCapsuleController

				void						getCapsule(PxExtendedCapsule& capsule)		const;

				PxF32						mRadius;
				PxF32						mHeight;
				PxCapsuleClimbingMode::Enum	mClimbingMode;
	};

} // namespace Cct

}

/** \endcond */
#endif
