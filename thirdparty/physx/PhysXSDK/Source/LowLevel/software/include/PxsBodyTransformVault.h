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


#ifndef PXS_BODY_TRANSFORM_HASH_H
#define PXS_BODY_TRANSFORM_HASH_H

#include "PxTransform.h"
#include "PsPool.h"
#include "PxvConfig.h"

namespace physx
{

struct PxsBodyCore;

#define PXS_BODY_TRANSFORM_HASH_SIZE		1024	// Size of hash table for last frame's body to world transforms
													// NOTE: Needs to be power of 2

/*!
Structure to store the current and the last frame's body to world transformations
for bodies that collide with particles.
*/
class PxsBodyTransformVault
{
public:
	PxsBodyTransformVault();
	~PxsBodyTransformVault();


	void					addBody(const PxsBodyCore& body);
	void					removeBody(const PxsBodyCore& body);
	void					teleportBody(const PxsBodyCore& body);
	const PxTransform*		getTransform(const PxsBodyCore& body) const;
	void					update();
	PX_FORCE_INLINE bool	isInVault(const PxsBodyCore& body) const;
	PX_FORCE_INLINE PxU32	getBodyCount() const { return mBodyCount;}

private:
	struct PxsBody2World
	{
		PxTransform			b2w;		// The old transform
		const PxsBodyCore*	body;
		PxsBody2World*		next;
		PxU32				refCount;
	};

	PX_FORCE_INLINE		PxU32			getHashIndex(const PxsBodyCore& body) const;
	PX_FORCE_INLINE		PxsBody2World*	createEntry(const PxsBodyCore& body);
	PX_FORCE_INLINE		bool			findEntry(const PxsBodyCore& body, PxsBody2World*& entry, PxsBody2World*& prevEntry) const;

						void			updateInternal();
						bool			isInVaultInternal(const PxsBodyCore& body) const;

private:
	PxsBody2World*			mBody2WorldHash[PXS_BODY_TRANSFORM_HASH_SIZE];	// Hash table for last frames world to shape transforms.
	Ps::Pool<PxsBody2World>	mBody2WorldPool;								// Pool of last frames body to world transforms.
	PxU32					mBodyCount;
};

bool PxsBodyTransformVault::isInVault(const PxsBodyCore& body) const
{
	//if the vault is not even used this should be fast and inlined
	if (mBodyCount == 0)
		return false;
		
	return isInVaultInternal(body);
}

}

#endif // PXS_BODY_TRANSFORM_HASH_H
