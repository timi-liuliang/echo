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

#ifndef PXC_CONTACT_CACHE_H
#define PXC_CONTACT_CACHE_H

#include "foundation/PxTransform.h"
#include "PxvConfig.h"


namespace physx
{


#if CACHE_LOCAL_CONTACTS_XP

struct PxcLocalContactsCache
{
	PxTransform	mTransform0;
	PxTransform	mTransform1;
	PxU16		mNbCachedContacts;
	bool		mUseFaceIndices;
	bool		mSameNormal;

	PX_FORCE_INLINE void operator = (const PxcLocalContactsCache& other)
	{
		mTransform0			= other.mTransform0;
		mTransform1			= other.mTransform1;
		mNbCachedContacts	= other.mNbCachedContacts;
		mUseFaceIndices		= other.mUseFaceIndices;
		mSameNormal			= other.mSameNormal;
	}
};


}


#endif  // CACHE_LOCAL_CONTACTS_XP

#endif  // PXC_CONTACT_CACHE_H
