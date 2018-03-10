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

#ifndef CCT_CHARACTER_CONTROLLER
#define CCT_CHARACTER_CONTROLLER

//#define USE_CONTACT_NORMAL_FOR_SLOPE_TEST

#include "PxController.h"
#include "PxControllerObstacles.h"
#include "CctUtils.h"
#include "PxTriangle.h"
#include "PsArray.h"
#include "PsHashSet.h"
#include "CmPhysXCommon.h"

namespace physx
{

struct PxFilterData;
class PxQueryFilterCallback;
class PxObstacle;

namespace Cm
{
	class RenderBuffer;
}

namespace Cct
{
	struct CCTParams
	{
											CCTParams();

		PxControllerNonWalkableMode::Enum	mNonWalkableMode;
		PxQuat								mQuatFromUp;
		PxVec3								mUpDirection;
		PxF32								mSlopeLimit;
		PxF32								mContactOffset;
		PxF32								mStepOffset;
		PxF32								mInvisibleWallHeight;
		PxF32								mMaxJumpHeight;
		PxF32								mMaxEdgeLength2;
		bool								mTessellation;
		bool								mHandleSlope;		// True to handle walkable parts according to slope
		bool								mOverlapRecovery;
		bool								mPreciseSweeps;
		bool								mPreventVerticalSlidingAgainstCeiling;
	};

	template<class T, class A>
	PX_INLINE T* reserve(Ps::Array<T, A>& array, PxU32 nb)
	{
		const PxU32 currentSize = array.size();
		array.resizeUninitialized(array.size() + nb);
		return array.begin() + currentSize;
	}

	typedef Ps::Array<PxTriangle>	TriArray;
	typedef Ps::Array<PxU32>		IntArray;

	/* Exclude from documentation */
	/** \cond */

	struct TouchedGeomType
	{
		enum Enum
		{
			eUSER_BOX,
			eUSER_CAPSULE,
			eMESH,
			eBOX,
			eSPHERE,
			eCAPSULE,

			eLAST,

			eFORCE_DWORD	= 0x7fffffff
		};
	};

	class SweptVolume;

// PT: apparently .Net aligns some of them on 8-bytes boundaries for no good reason. This is bad.
#if !defined(PX_WIIU) && !defined(PX_PSP2)	// Whenever a variable points to a field of a specially aligned struct, it has to be declared with __packed (see GHS docu, Structure Packing, page 111).
				// Every reference to such a field needs the __packed declaration: all function parameters and assignment operators etc.
#pragma pack(push,4)
#endif

	struct TouchedGeom
	{
		TouchedGeomType::Enum	mType;
		const void*				mTGUserData;	// PxController or PxShape pointer
		const PxRigidActor*		mActor;			// PxActor for PxShape pointers (mandatory with shared shapes)
		PxExtendedVec3			mOffset;		// Local origin, typically the center of the world bounds around the character. We translate both
												// touched shapes & the character so that they are nearby this PxVec3, then add the offset back to
												// computed "world" impacts.
	protected:
		~TouchedGeom(){}
	};

	struct TouchedUserBox : public TouchedGeom
	{
		PxExtendedBox			mBox;
	};
	PX_COMPILE_TIME_ASSERT(sizeof(TouchedUserBox)==sizeof(TouchedGeom)+sizeof(PxExtendedBox));

	struct TouchedUserCapsule : public TouchedGeom
	{
		PxExtendedCapsule		mCapsule;
	};
	PX_COMPILE_TIME_ASSERT(sizeof(TouchedUserCapsule)==sizeof(TouchedGeom)+sizeof(PxExtendedCapsule));

	struct TouchedMesh : public TouchedGeom
	{
		PxU32			mNbTris;
		PxU32			mIndexWorldTriangles;
	};

	struct TouchedBox : public TouchedGeom
	{
		PxVec3			mCenter;
		PxVec3			mExtents;
		PxQuat			mRot;
	};

	struct TouchedSphere : public TouchedGeom
	{
		PxVec3			mCenter;		//!< Sphere's center
		PxF32			mRadius;		//!< Sphere's radius
	};

	struct TouchedCapsule : public TouchedGeom
	{
		PxVec3			mP0;		//!< Start of segment
		PxVec3			mP1;		//!< End of segment
		PxF32			mRadius;	//!< Capsule's radius
	};

#if !defined(PX_WIIU) && !defined(PX_PSP2)
#pragma pack(pop)
#endif

	struct SweptContact
	{
		PxExtendedVec3		mWorldPos;		// Contact position in world space
		PxVec3				mWorldNormal;	// Contact normal in world space
		PxF32				mDistance;		// Contact distance
		PxU32				mInternalIndex;	// Reserved for internal usage
		PxU32				mTriangleIndex;	// Triangle index for meshes/heightfields
		TouchedGeom*		mGeom;

		PX_FORCE_INLINE		void	setWorldPos(const PxVec3& localImpact, const PxExtendedVec3& offset)
		{
			mWorldPos.x = localImpact.x + offset.x;
			mWorldPos.y = localImpact.y + offset.y;
			mWorldPos.z = localImpact.z + offset.z;
		}
	};

	// PT: user-defined obstacles. Note that "user" is from the SweepTest class' point of view,
	// i.e. the PhysX CCT module is the user in this case. This is to limit coupling between the
	// core CCT module and the PhysX classes.
	struct UserObstacles// : PxObstacleContext
	{
		PxU32						mNbBoxes;
		const PxExtendedBox*		mBoxes;
		const void**				mBoxUserData;

		PxU32						mNbCapsules;
		const PxExtendedCapsule*	mCapsules;
		const void**				mCapsuleUserData;
	};

	struct InternalCBData_OnHit{};
	struct InternalCBData_FindTouchedGeom{};

	enum SweepTestFlag
	{
		STF_HIT_NON_WALKABLE		= (1<<0),
		STF_WALK_EXPERIMENT			= (1<<1),
		STF_VALIDATE_TRIANGLE_DOWN	= (1<<2),	// Validate touched triangle data (down pass)
		STF_VALIDATE_TRIANGLE_SIDE	= (1<<3),	// Validate touched triangle data (side pass)
		STF_TOUCH_OTHER_CCT			= (1<<4),	// Are we standing on another CCT or not? (only updated for down pass)
		STF_TOUCH_OBSTACLE			= (1<<5),	// Are we standing on an obstacle or not? (only updated for down pass)
		STF_NORMALIZE_RESPONSE		= (1<<6),
		STF_FIRST_UPDATE			= (1<<7),
		STF_IS_MOVING_UP			= (1<<8)
	};	

	enum SweepPass
	{
		SWEEP_PASS_UP,
		SWEEP_PASS_SIDE,
		SWEEP_PASS_DOWN,
		SWEEP_PASS_SENSOR
	};

	class SweepTest
	{
	public:
										SweepTest();
										~SweepTest();

		PxControllerCollisionFlags		moveCharacter(	const InternalCBData_FindTouchedGeom* userData,
														const InternalCBData_OnHit* user_data2,
														SweptVolume& volume,
														const PxVec3& direction,
														const UserObstacles& userObstacles,
														PxF32 min_dist,
														const PxControllerFilters& filters,
														bool constrainedClimbingMode,
														bool standingOnMoving
														);

					bool				doSweepTest(const InternalCBData_FindTouchedGeom* userDataTouchedGeom,
													const InternalCBData_OnHit* userDataOnHit,
													const UserObstacles& userObstacles,
													SweptVolume& swept_volume,
													const PxVec3& direction, const PxVec3& sideVector, PxU32 max_iter,
													PxU32* nb_collisions, PxF32 min_dist, const PxControllerFilters& filters, SweepPass sweepPass);

					void				findTouchedObstacles(const UserObstacles& userObstacles, const PxExtendedBounds3& world_box);

					void				voidTestCache();
					void				onRelease(const PxBase& observed);

		// observer notifications
					void				onObstacleRemoved(ObstacleHandle index);
					void				onObstacleUpdated(ObstacleHandle index, const PxObstacleContext* context, const PxVec3& origin, const PxVec3& unitDir, const PxReal distance);
					void				onObstacleAdded(ObstacleHandle index, const PxObstacleContext* context, const PxVec3& origin, const PxVec3& unitDir, const PxReal distance);

					void				onOriginShift(const PxVec3& shift);

					Cm::RenderBuffer*	mRenderBuffer;
					PxU32				mRenderFlags;
					TriArray			mWorldTriangles;
					IntArray			mTriangleIndices;
					IntArray			mGeomStream;
					PxExtendedBounds3	mCacheBounds;
					PxU32				mCachedTriIndexIndex;
					mutable	PxU32		mCachedTriIndex[3];
					PxU32				mNbCachedStatic;
					PxU32				mNbCachedT;
	public:
#ifdef USE_CONTACT_NORMAL_FOR_SLOPE_TEST
					PxVec3				mContactNormalDownPass;
#else
					PxVec3				mContactNormalDownPass;
					PxVec3				mContactNormalSidePass;
					float				mTouchedTriMin;
					float				mTouchedTriMax;
					//PxTriangle		mTouchedTriangle;
#endif
					//
					ObstacleHandle		mTouchedObstacleHandle;	// Obstacle on which the CCT is standing
					PxShape*			mTouchedShape;		// Shape on which the CCT is standing
					const PxRigidActor*	mTouchedActor;		// Actor from touched shape
					PxVec3				mTouchedPos;		// Last known position of mTouchedShape/mTouchedObstacle
					// PT: TODO: union those
					PxVec3				mTouchedPosShape_Local;
					PxVec3				mTouchedPosShape_World;
					PxVec3				mTouchedPosObstacle_Local;
					PxVec3				mTouchedPosObstacle_World;
					//
					CCTParams			mUserParams;
					PxF32				mVolumeGrowth;		// Must be >1.0f and not too big
					PxF32				mContactPointHeight;	// UBI
					PxU32				mSQTimeStamp;
					PxU16				mNbFullUpdates;
					PxU16				mNbPartialUpdates;
					PxU16				mNbTessellation;
					PxU16				mNbIterations;
					PxU32				mFlags;

	PX_FORCE_INLINE	void				resetStats()
										{
											mNbFullUpdates		= 0;
											mNbPartialUpdates	= 0;
											mNbTessellation		= 0;
											mNbIterations		= 0;
										}
	private:
				void					updateTouchedGeoms(	const InternalCBData_FindTouchedGeom* userData, const UserObstacles& userObstacles,
															const PxExtendedBounds3& worldBox, const PxControllerFilters& filters, const PxVec3& sideVector);
	};

	class CCTFilter	// PT: internal filter data, could be replaced with PxControllerFilters eventually
	{
		public:
		PX_FORCE_INLINE	CCTFilter() :
			mFilterData		(NULL),
			mFilterCallback	(NULL),
			mStaticShapes	(false),
			mDynamicShapes	(false),
			mPreFilter		(false),
			mPostFilter		(false),
			mCCTShapes		(NULL)
		{
		}
		const PxFilterData*		mFilterData;
		PxQueryFilterCallback*	mFilterCallback;
		bool					mStaticShapes;
		bool					mDynamicShapes;
		bool					mPreFilter;
		bool					mPostFilter;
		Ps::HashSet<PxShape>*	mCCTShapes;
	};

	PxU32 getSceneTimestamp(const InternalCBData_FindTouchedGeom* userData);

	void findTouchedGeometry(const InternalCBData_FindTouchedGeom* userData,
		const PxExtendedBounds3& world_aabb,

		TriArray& world_triangles,
		IntArray& triIndicesArray,
		IntArray& geomStream,

		const CCTFilter& filter,
		const CCTParams& params,
		PxU16& nbTessellation);

	PxU32 shapeHitCallback(const InternalCBData_OnHit* userData, const SweptContact& contact, const PxVec3& dir, PxF32 length);
	PxU32 userHitCallback(const InternalCBData_OnHit* userData, const SweptContact& contact, const PxVec3& dir, PxF32 length);

} // namespace Cct

}

/** \endcond */
#endif
