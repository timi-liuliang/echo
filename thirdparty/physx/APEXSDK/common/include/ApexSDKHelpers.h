/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEXSDKHELPERS_H__
#define __APEXSDKHELPERS_H__


#include "PsArray.h"
#include "PsSort.h"
#include "ApexInterface.h"
#include "ApexString.h"
#include "NiApexSDK.h"
#include "NiResourceProvider.h"
#include "PsMathUtils.h"
#include "PxMat33Legacy.h"

namespace physx
{
namespace debugger
{
	namespace comm
	{
		class PvdDataStream;
	}
}

namespace apex
{

class NxApexResource;
class NiApexSDK;

enum NxApexStreamPointerToken
{
	NX_SPT_INVALID_PTR,
	NX_SPT_VALID_PTR
};


/*
	Resource list - holds a list of NxApexResource objects, for quick removal
 */
class NxResourceList: public physx::shdfnd::UserAllocated
{
	physx::Array<NxApexResource*>	mArray;
	ReadWriteLock					mRWLock;

#ifndef WITHOUT_PVD
	// for PVD
	const void*						mOwner;
	ApexSimpleString				mListName;
	ApexSimpleString				mEntryName;
#endif

public:

	NxResourceList()
#ifndef WITHOUT_PVD
	 : mOwner(NULL) 
#endif
	{}
	~NxResourceList();

	void			clear(); // explicitely free children

	void			add(NxApexResource& resource);
	void			remove(physx::PxU32 index);
	physx::PxU32			getSize() const
	{
		ScopedReadLock scopedLock(const_cast<ReadWriteLock&>(mRWLock));
		return mArray.size();
	}
	NxApexResource*	getResource(physx::PxU32 index) const
	{
		ScopedReadLock scopedLock(const_cast<ReadWriteLock&>(mRWLock));
		return mArray[index];
	}

	template<typename Predicate>
	void			sort(const Predicate& compare)
	{
		ScopedWriteLock scopedLock(mRWLock);
		PxU32 size = mArray.size();
		if (size > 0)
		{
			shdfnd::sort(&mArray[0], size, compare);
		}

		for (PxU32 i = 0; i < size; ++i)
		{
			mArray[i]->setListIndex(*this, i);
		}
	}

#ifndef WITHOUT_PVD
	void setupForPvd(const void* owner, const char* listName, const char* entryName);
	void initPvdInstances(physx::debugger::comm::PvdDataStream& pvdStream);
#endif
};


#ifndef M_SQRT1_2	//1/sqrt(2)
#define M_SQRT1_2 double(0.7071067811865475244008443621048490)
#endif


/*
	Creates a rotation matrix which rotates about the axisAngle vector.  The length of
	the axisAngle vector is the desired rotation angle.  In this approximation, however,
	this is only the case for small angles.  As the length of axisAngle grows (is no
	longer very much less than 1 radian) the approximation becomes worse.  As the length
	of axisAngle approaches infinity, the rotation angle approaches pi.  The exact
	relation is:

		rotation_angle = 2*atan( axisAngle.magnitude()/2 )

	One use for this construction is the rotation applied to mesh particle system particles.  With a
	decent frame rate, the rotation angle should be small, unless the particle is going
	very fast or has very small radius.  In that case, or if the frame rate is poor,
	the inaccuracy in this construction probably won't be noticed.

	Error: The rotation angle is accurate to:
		1%	up to 20 degrees
		10% up to 70 degrees
*/
PX_INLINE void approxAxisAngleToMat33(const physx::PxVec3& axisAngle, physx::PxMat33Legacy& rot)
{
	const physx::PxF32 x = 0.5f * axisAngle.x;
	const physx::PxF32 y = 0.5f * axisAngle.y;
	const physx::PxF32 z = 0.5f * axisAngle.z;
	const physx::PxF32 xx = x * x;
	const physx::PxF32 yy = y * y;
	const physx::PxF32 zz = z * z;
	const physx::PxF32 xy = x * y;
	const physx::PxF32 yz = y * z;
	const physx::PxF32 zx = z * x;
	const physx::PxF32 twoRecipNorm2 = 2.0f / (1.0f + xx + yy + zz);	// w = 1
	rot(0, 0) = 1.0f - twoRecipNorm2 * (yy + zz);
	rot(0, 1) = twoRecipNorm2 * (xy - z);
	rot(0, 2) = twoRecipNorm2 * (zx + y);
	rot(1, 0) = twoRecipNorm2 * (xy + z);
	rot(1, 1) = 1.0f - twoRecipNorm2 * (zz + xx);
	rot(1, 2) = twoRecipNorm2 * (yz - x);
	rot(2, 0) = twoRecipNorm2 * (zx - y);
	rot(2, 1) = twoRecipNorm2 * (yz + x);
	rot(2, 2) = 1.0f - twoRecipNorm2 * (xx + yy);
}


// stl hash
PX_INLINE physx::PxU32 hash(const char* str, physx::PxU32 len)
{
	physx::PxU32 hash = 0;

	for (physx::PxU32 i = 0; i < len; i++)
	{
		hash = 5 * hash + str[i];
	}

	return hash;
}

PX_INLINE physx::PxU32 GetStamp(ApexSimpleString& name)
{
	return hash(name.c_str(), name.len());
}

#if 0
// these are poison
void writeStreamHeader(physx::PxFileBuf& stream, ApexSimpleString& streamName, physx::PxU32 versionStamp);
physx::PxU32 readStreamHeader(const physx::PxFileBuf& stream, ApexSimpleString& streamName);
#endif

PX_INLINE physx::PxU32 MaxElementIndex(const physx::PxVec3& v)
{
	const physx::PxU32 m01 = (physx::PxU32)(v.y > v.x);
	const physx::PxU32 m2 = (physx::PxU32)(v.z > v[m01]);
	return m2 << 1 | m01 >> m2;
}

PX_INLINE physx::PxU32 MinElementIndex(const physx::PxVec3& v)
{
	const physx::PxU32 m01 = (physx::PxU32)(v.y < v.x);
	const physx::PxU32 m2 = (physx::PxU32)(v.z < v[m01]);
	return m2 << 1 | m01 >> m2;
}

PX_INLINE physx::PxU32 MaxAbsElementIndex(const physx::PxVec3& v)
{
	const physx::PxVec3 a(physx::PxAbs(v.x), physx::PxAbs(v.y), physx::PxAbs(v.z));
	const physx::PxU32 m01 = (physx::PxU32)(a.y > a.x);
	const physx::PxU32 m2 = (physx::PxU32)(a.z > a[m01]);
	return m2 << 1 | m01 >> m2;
}

PX_INLINE physx::PxU32 MinAbsElementIndex(const physx::PxVec3& v)
{
	const physx::PxVec3 a(physx::PxAbs(v.x), physx::PxAbs(v.y), physx::PxAbs(v.z));
	const physx::PxU32 m01 = (physx::PxU32)(a.y < a.x);
	const physx::PxU32 m2 = (physx::PxU32)(a.z < a[m01]);
	return m2 << 1 | m01 >> m2;
}


/******************************************************************************
 * Helper functions for loading assets
 *****************************************************************************/
class ApexAssetHelper
{
public:
	static void* getAssetFromName(NiApexSDK*	sdk,
	                              const char*	authoringTypeName,
	                              const char*	assetName,
	                              NxResID&		inOutResID,
	                              NxResID		optionalNsID = INVALID_RESOURCE_ID);

	static void* getAssetFromNameList(NiApexSDK*	sdk,
	                                  const char* authoringTypeName,
	                                  physx::Array<AssetNameIDMapping*>& nameIdList,
	                                  const char* assetName,
	                                  NxResID assetNsId = INVALID_RESOURCE_ID);

	static void* getIosAssetFromName(NiApexSDK*	sdk,
	                                 const char* iosTypeName,
	                                 const char* iosAssetName);

};

}
} // end namespace physx::apex

#endif	// __APEXSDKHELPERS_H__
