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

#ifndef OPC_TREEBUILDERS_H
#define OPC_TREEBUILDERS_H

#include "PxPhysXCommonConfig.h"
#include "PsUserAllocated.h"
#include "PsVecMath.h"
#include "PxBounds3.h"
#include "CmPhysXCommon.h"

namespace physx
{
using namespace shdfnd::aos;
namespace Sq
{
	class MeshInterface;

	//! Tree splitting rules
	enum SplittingRules
	{
		// Primitive split
		SPLIT_LARGEST_AXIS		= (1<<0),		//!< Split along the largest axis
		SPLIT_SPLATTER_POINTS	= (1<<1),		//!< Splatter primitive centers (QuickCD-style)
		SPLIT_BEST_AXIS			= (1<<2),		//!< Try largest axis, then second, then last
		SPLIT_BALANCED			= (1<<3),		//!< Try to keep a well-balanced tree
		SPLIT_FIFTY				= (1<<4),		//!< Arbitrary 50-50 split
		//
		SPLIT_FORCE_DWORD		= 0x7fffffff
	};

	//! Simple wrapper around build-related settings [Opcode 1.3]
	struct BuildSettings
	{
		PX_FORCE_INLINE	BuildSettings() : mLimit(1), mRules(SPLIT_FORCE_DWORD) {}

		PxU32		mLimit;				//!< Limit number of primitives / node. If limit is 1, build a complete tree (2*N-1 nodes)
		PxU32		mRules;				//!< Building/Splitting rules (a combination of SplittingRules flags)
	};

	class AABBTreeNode;

	class AABBTreeBuilder : public Ps::UserAllocated
	{
		public:
									AABBTreeBuilder() : mAABBArray(NULL) { reset(); }
									~AABBTreeBuilder() {}

		PX_FORCE_INLINE	void		reset()
									{
										mTotalPrims			= 0;
										mNbPrimitives		= 0;
										mNodeBase			= NULL;
										mCount				= 0;
										mNbInvalidSplits	= 0;
									}

		/**
		 *	Computes the AABB of a set of primitives.
		 *	\param		primitives	[in] list of indices of primitives
		 *	\param		nbPrims		[in] number of indices
		 *	\param		global_box	[out] global AABB enclosing the set of input primitives
		 *	\return		true if success
		 */
		PX_FORCE_INLINE void computeGlobalBox(const PxU32* primitives, PxU32 nbPrims, PxBounds3& result, Vec3V* mn, Vec3V* mx) const
		{
			PX_ASSERT(primitives && nbPrims);

			// Initialize global box
			Vec3V resultMin = V3LoadU(mAABBArray[primitives[0]].minimum);
			Vec3V resultMax = V3LoadU(mAABBArray[primitives[0]].maximum);

			// Loop through boxes
			for(PxU32 i=1;i<nbPrims;i++)
			{
				const PxBounds3& b = mAABBArray[primitives[i]];
				resultMin = V3Min(resultMin, V3LoadU(b.minimum));
				resultMax = V3Max(resultMax, V3LoadU(b.maximum));
			}

			*mn = resultMin;
			*mx = resultMax;
			V3StoreU(resultMin, result.minimum);
			V3StoreU(resultMax, result.maximum);
		}

		/**
		 *	Computes the splitting value along a given axis for a given primitive.
		 *	\param		index	[in] index of the primitive to split
		 *	\param		axis	[in] axis index (0,1,2)
		 *	\return		splitting value
		 */
		PX_FORCE_INLINE float getSplittingValue(PxU32 index, PxU32 axis) const
		{
			// For an AABB, the splitting value is the middle of the given axis,
			// i.e. the corresponding component of the center point
			return mAABBArray[index].getCenter(axis);
		}

		PX_FORCE_INLINE Vec3V getSplittingValues(PxU32 index) const
		{
			const PxBounds3& b = mAABBArray[index];
			const Vec3V half_ = V3Load(0.5f);
			return V3Mul(half_, V3Add(V3LoadU(b.minimum), V3LoadU(b.maximum)));
		}

		/**
		 *	Computes the splitting value along a given axis for a given node.
		 *	\param		primitives	[in] list of indices of primitives
		 *	\param		nbPrims		[in] number of indices
		 *	\param		global_box	[in] global AABB enclosing the set of input primitives
		 *	\param		axis		[in] axis index (0,1,2)
		 *	\return		splitting value
		 */
		PX_FORCE_INLINE	float		getSplittingValue(const PxBounds3& global_box, PxU32 axis)	const
									{
										// Default split value = middle of the axis (using only the box)
										return global_box.getCenter(axis);
									}

		/**
		 *	Validates node subdivision. This is called each time a node is considered for subdivision, during tree building.
		 *	\param		primitives	[in] list of indices of primitives
		 *	\param		nbPrims		[in] number of indices
		 *	\param		global_box	[in] global AABB enclosing the set of input primitives
		 *	\return		Ps::IntTrue if the node should be subdivised
		 */
		PX_FORCE_INLINE	Ps::IntBool	validateSubdivision(PxU32 nbPrims)
									{
										// Check the user-defined limit
										if(nbPrims<=mSettings.mLimit)
											return Ps::IntFalse;

										return Ps::IntTrue;
									}

					BuildSettings	mSettings;			//!< Splitting rules & split limit [Opcode 1.3]
					PxU32			mNbPrimitives;		//!< Total number of primitives.
					AABBTreeNode*	mNodeBase;			//!< Address of node pool [Opcode 1.3]
		// Stats
		PX_FORCE_INLINE	void		setCount(PxU32 nb)				{ mCount=nb;				}
		PX_FORCE_INLINE	void		increaseCount(PxU32 nb)			{ mCount+=nb;				}
		PX_FORCE_INLINE	PxU32		getCount()				const	{ return mCount;			}
		PX_FORCE_INLINE	void		setNbInvalidSplits(PxU32 nb)	{ mNbInvalidSplits=nb;		}
		PX_FORCE_INLINE	void		increaseNbInvalidSplits()		{ mNbInvalidSplits++;		}
		PX_FORCE_INLINE	PxU32		getNbInvalidSplits()	const	{ return mNbInvalidSplits;	}

						PxU32		mTotalPrims;
				const	PxBounds3*	mAABBArray;			//!< Shortcut to an app-controlled array of AABBs.
		private:
						PxU32		mCount;				//!< Stats: number of nodes created
						PxU32		mNbInvalidSplits;	//!< Stats: number of invalid splits
	};

} // namespace Gu

}

#endif // OPC_TREEBUILDERS_H
