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


#ifndef PX_PHYSICS_COMMON_UTILS
#define PX_PHYSICS_COMMON_UTILS


#include "PxVec3.h"
#include "PxMat33.h"

#include "CmPhysXCommon.h"
#include "PxSerialFramework.h"
#include "PxBase.h"
#include "PsUserAllocated.h"

namespace physx
{
namespace Cm
{

PX_INLINE void transformInertiaTensor(const PxVec3& invD, const PxMat33& M, PxMat33& mIInv)
{
	const float	axx = invD.x*M(0,0), axy = invD.x*M(1,0), axz = invD.x*M(2,0);
	const float	byx = invD.y*M(0,1), byy = invD.y*M(1,1), byz = invD.y*M(2,1);
	const float	czx = invD.z*M(0,2), czy = invD.z*M(1,2), czz = invD.z*M(2,2);

	mIInv(0,0) = axx*M(0,0) + byx*M(0,1) + czx*M(0,2);
	mIInv(1,1) = axy*M(1,0) + byy*M(1,1) + czy*M(1,2);
	mIInv(2,2) = axz*M(2,0) + byz*M(2,1) + czz*M(2,2);

	mIInv(0,1) = mIInv(1,0)	= axx*M(1,0) + byx*M(1,1) + czx*M(1,2);
	mIInv(0,2) = mIInv(2,0)	= axx*M(2,0) + byx*M(2,1) + czx*M(2,2);
	mIInv(1,2) = mIInv(2,1)	= axy*M(2,0) + byy*M(2,1) + czy*M(2,2);
}

template <typename T, PxU32 size>
struct Block
{
	PxU8 mem[sizeof(T)*size];
	Block() {}	// get around VS warning C4345, otherwise useless
};


// Array with externally managed storage.
// Allocation and resize policy are managed by the owner, 
// Very minimal functionality right now, just POD types

template <typename T, 
		  typename Owner, 
		  typename IndexType,
		  void (Owner::*realloc)(T*& currentMem, IndexType& currentCapacity, IndexType size, IndexType requiredMinCapacity)>
class OwnedArray
{
public:
	OwnedArray() 
	: mData(0)
	, mCapacity(0) 
	, mSize(0)
	{}

	~OwnedArray()		// owner must call releaseMem before destruction
	{
		PX_ASSERT(mCapacity==0);
	}

	void pushBack(T& element, Owner& owner)
	{
		// there's a failure case if here if we push an existing element which causes a resize -
		// a rare case not worth coding around; if you need it, copy the element then push it.

		PX_ASSERT(&element<mData || &element>=mData+mSize);
		if(mSize==mCapacity)
			(owner.*realloc)(mData, mCapacity, mSize, PxU16(mSize+1));

		PX_ASSERT(mData && mSize<mCapacity);
		mData[mSize++] = element;
	}

	IndexType size() const
	{
		return mSize;
	}

	void replaceWithLast(IndexType index)
	{
		PX_ASSERT(index<mSize);
		mData[index] = mData[--mSize];
	}

	T* begin() const
	{
		return mData;
	}

	T* end() const
	{
		return mData+mSize;
	}

	T& operator [](IndexType index)
	{
		PX_ASSERT(index<mSize);
		return mData[index];
	}

	const T& operator [](IndexType index) const
	{
		PX_ASSERT(index<mSize);
		return mData[index];
	}


	void reserve(IndexType capacity, Owner &owner)
	{
		if(capacity>=mCapacity)
			(owner.*realloc)(mData, mCapacity, mSize, capacity);
	}

	void releaseMem(Owner &owner)
	{
		mSize = 0;
		(owner.*realloc)(mData, mCapacity, 0, 0);
	}

private:
	T*					mData;
	IndexType			mCapacity;
	IndexType			mSize;

	// just in case someone tries to use a non-POD in here
	union FailIfNonPod
	{
		T t;
		int x;
	};
};

PX_INLINE PxU32 getPadding(size_t value, PxU32 alignment)
{
	const PxU32 mask = alignment-1;
	const PxU32 overhead = PxU32(value) & mask;
	return (alignment - overhead) & mask;
}

PX_INLINE PxU8* alignPtr(PxU8* ptr, PxU32 alignment = PX_SERIAL_ALIGN)
{
	if(!alignment)
		return ptr;

	const PxU32 padding = getPadding(size_t(ptr), alignment);
	PX_ASSERT(!getPadding(size_t(ptr + padding), alignment));
	return ptr + padding;
}

/**
Any object deriving from PxBase needs to call this function instead of 'delete object;'. 

We don't want implement 'operator delete' in PxBase because that would impose how
memory of derived classes is allocated. Even though most or all of the time derived classes will 
be user allocated, we don't want to put UserAllocatable into the API and derive from that.
*/
template<typename T>
PX_INLINE void deletePxBase(T* object)
{
	if(object->getBaseFlags() & PxBaseFlag::eOWNS_MEMORY)
		PX_DELETE(object);
	else
		object->~T();
}

#if defined(PX_CHECKED)
/**
Mark a specified amount of memory with 0xcd pattern. This is used to check that the meta data 
definition for serialized classes is complete in checked builds.
*/
PX_INLINE void markSerializedMem(void* ptr, PxU32 byteSize)
{
	for (PxU32 i = 0; i < byteSize; ++i)
      reinterpret_cast<PxU8*>(ptr)[i] = 0xcd;
}

/**
Macro to instantiate a type for serialization testing. 
Note: Only use PX_NEW_SERIALIZED once in a scope.
*/
#define PX_NEW_SERIALIZED(v,T) 															        \
    void* _buf = physx::shdfnd::ReflectionAllocator<T>().allocate(sizeof(T),__FILE__,__LINE__);  \
	Cm::markSerializedMem(_buf, sizeof(T));                                                      \
    v = PX_PLACEMENT_NEW(_buf, T)

#else
PX_INLINE void markSerializedMem(void*, PxU32){}

#define PX_NEW_SERIALIZED(v,T)  v = PX_NEW(T)
#endif

template<typename T, class Alloc>
struct ArrayAccess: public Ps::Array<T, Alloc> 
{
	void store(PxSerializationContext& context) const
	{
		if(this->mData && (this->mSize || this->capacity()))
			context.writeData(this->mData, this->capacity()*sizeof(T));
	}

	void load(PxDeserializationContext& context)
	{
		if(this->mData && (this->mSize || this->capacity()))
			this->mData = context.readExtraData<T>(this->capacity());
	}
};

template<typename T, typename Alloc>
void exportArray(const Ps::Array<T, Alloc>& a, PxSerializationContext& context)
{
	static_cast<const ArrayAccess<T, Alloc>&>(a).store(context);
}

template<typename T, typename Alloc>
void importArray(Ps::Array<T, Alloc>& a, PxDeserializationContext& context)
{
	static_cast<ArrayAccess<T, Alloc>&>(a).load(context);
}

template<typename T, PxU32 N, typename Alloc>
void exportInlineArray(const Ps::InlineArray<T, N, Alloc>& a, PxSerializationContext& context)
{
	if(!a.isInlined())
		Cm::exportArray(a, context);
}

template<typename T, PxU32 N, typename Alloc>
void importInlineArray(Ps::InlineArray<T, N, Alloc>& a, PxDeserializationContext& context)
{
	if(!a.isInlined())
		Cm::importArray(a, context);
}
} // namespace Cm



}

#endif
