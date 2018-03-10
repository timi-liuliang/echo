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



#include "PxsBodyTransformVault.h"
#include "PxvGeometry.h"
#include "PxvDynamics.h"
#include "PsHash.h"

using namespace physx;

PxsBodyTransformVault::PxsBodyTransformVault() :
	mBody2WorldPool("body2WorldPool", 256),
	mBodyCount(0)
{
	// Make sure the hash size is a power of 2
	PX_ASSERT((((PXS_BODY_TRANSFORM_HASH_SIZE-1)^PXS_BODY_TRANSFORM_HASH_SIZE)+1) == (2*PXS_BODY_TRANSFORM_HASH_SIZE));

	PxMemSet(mBody2WorldHash, 0, PXS_BODY_TRANSFORM_HASH_SIZE * sizeof(PxsBody2World*));
}

PxsBodyTransformVault::~PxsBodyTransformVault()
{
}

PX_FORCE_INLINE PxU32 PxsBodyTransformVault::getHashIndex(const PxsBodyCore& body) const
{
	PxU32 index =  Ps::hash(&body);
	return (index & (PXS_BODY_TRANSFORM_HASH_SIZE - 1));	// Modulo hash size
}

void PxsBodyTransformVault::addBody(const PxsBodyCore& body)
{
	PxsBody2World* entry;
	PxsBody2World* dummy;

	bool hasEntry = findEntry(body, entry, dummy);
	if (!hasEntry)
	{
		PxsBody2World* newEntry;
		if (entry)
		{
			// No entry for the given body but the hash entry has other bodies
			// --> create new entry, link into list
			newEntry = createEntry(body);
			entry->next = newEntry;
		}
		else
		{
			// No entry for the given body and no hash entry --> create new entry
			PxU32 hashIndex = getHashIndex(body);
			newEntry = createEntry(body);
			mBody2WorldHash[hashIndex] = newEntry;
		}
		newEntry->refCount = 1;
		mBodyCount++;
	}
	else
	{
		entry->refCount++;
	}
}


void PxsBodyTransformVault::removeBody(const PxsBodyCore& body)
{
	PxsBody2World* entry;
	PxsBody2World* prevEntry;

	bool hasEntry = findEntry(body, entry, prevEntry);
	PX_ASSERT(hasEntry);
	PX_UNUSED(hasEntry);

	if (entry->refCount == 1)
	{
		if (prevEntry)
		{
			prevEntry->next = entry->next;
		}
		else
		{
			// Shape entry was first in list
			PxU32 hashIndex = getHashIndex(body);

			mBody2WorldHash[hashIndex] = entry->next;
		}
		mBody2WorldPool.destroy(entry);
		PX_ASSERT(mBodyCount > 0);
		mBodyCount--;
	}
	else
	{
		entry->refCount--;
	}
}

void PxsBodyTransformVault::teleportBody(const PxsBodyCore& body)
{
	PxsBody2World* entry;
	PxsBody2World* dummy;

	bool hasEntry = findEntry(body, entry, dummy);
	PX_ASSERT(hasEntry);
	PX_ASSERT(entry);
	PX_UNUSED(hasEntry);

	PX_CHECK_AND_RETURN(body.body2World.isValid(), "PxsBodyTransformVault::teleportBody: body.body2World is not valid.");

	entry->b2w = body.body2World;
}

const PxTransform* PxsBodyTransformVault::getTransform(const PxsBodyCore& body) const
{
	PxsBody2World* entry;
	PxsBody2World* dummy;

	bool hasEntry = findEntry(body, entry, dummy);
	//PX_ASSERT(hasEntry);
	//PX_UNUSED(hasEntry);
	//PX_ASSERT(entry);
	return hasEntry ? &entry->b2w : NULL;
}

void PxsBodyTransformVault::update()
{
	if(mBodyCount)
	{
		for(PxU32 i=0; i < PXS_BODY_TRANSFORM_HASH_SIZE; i++)
		{
			PxsBody2World* entry = mBody2WorldHash[i];

			while(entry)
			{
				PX_ASSERT(entry->body);
				entry->b2w = entry->body->body2World;
				entry = entry->next;
			}
		}
	}
}

PxsBodyTransformVault::PxsBody2World* PxsBodyTransformVault::createEntry(const PxsBodyCore& body)
{
	PxsBody2World* entry = mBody2WorldPool.construct();

	if (entry)
	{
		entry->b2w = body.body2World;
		entry->next = NULL;
		entry->body = &body;
	}

	return entry;
}

bool PxsBodyTransformVault::isInVaultInternal(const PxsBodyCore& body) const
{
	PxU32 hashIndex = getHashIndex(body);

	if (mBody2WorldHash[hashIndex])
	{
		PxsBody2World* curEntry = mBody2WorldHash[hashIndex];

		while(curEntry->next)
		{
			if (curEntry->body == &body)
				break;

			curEntry = curEntry->next;
		}

		if (curEntry->body == &body)
			return true;
	}

	return false;
}

bool PxsBodyTransformVault::findEntry(const PxsBodyCore& body, PxsBody2World*& entry, PxsBody2World*& prevEntry) const
{
	PxU32 hashIndex = getHashIndex(body);

	prevEntry = NULL;
	bool hasEntry = false;
	if (mBody2WorldHash[hashIndex])
	{
		PxsBody2World* curEntry = mBody2WorldHash[hashIndex];

		while(curEntry->next)
		{
			if (curEntry->body == &body)
				break;

			prevEntry = curEntry;
			curEntry = curEntry->next;
		}

		entry = curEntry;
		if (curEntry->body == &body)
		{
			// An entry already exists for the given body
			hasEntry = true;
		}
	}
	else
	{
		entry = NULL;
	}

	return hasEntry;
}
