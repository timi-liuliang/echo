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


#ifndef PX_PHYSICS_COMMON_RANGE
#define PX_PHYSICS_COMMON_RANGE

#include "CmPhysXCommon.h"

namespace physx
{
namespace Cm
{
	template<class T> 
	class Range 
	{
	private:
		T* mFirst;
		T* mLast; // past last element
	public:
		// c'tor
		Range();
		template <size_t N> 
		explicit Range(T (&array)[N]);
		Range(T* first, T* last);
		// mem functions
		PxU32 size() const;
		bool empty() const;
		void popFront();
		void popBack();
		T& front() const;
		T& back() const;
	};

	template <typename T, size_t N>
	Range<T> getRange(T (&array)[N])
	{
		return Range<T>(array);
	}

} // namespace Cm

template <typename T>
Cm::Range<T>::Range() : mFirst(0), mLast(0)
{}

template <typename T>
template <size_t N> 
Cm::Range<T>::Range(T (&array)[N]) : mFirst(array), mLast(array+N)
{}

template <typename T>
Cm::Range<T>::Range(T* first, T* last) : mFirst(first), mLast(last)
{
}

template <typename T>
PxU32 Cm::Range<T>::size() const
{
	return PxU32(mLast - mFirst); 
}

template <typename T>
bool Cm::Range<T>::empty() const
{
	return mFirst >= mLast; 
}

template <typename T>
void Cm::Range<T>::popFront()
{
	++mFirst;
}

template <typename T>
void Cm::Range<T>::popBack()
{
	--mLast;
}

template <typename T>
T& Cm::Range<T>::front() const
{
	return *mFirst;
}

template <typename T>
T& Cm::Range<T>::back() const
{
	return *(mLast-1);
}

}

#endif //PX_FRAMEWORK_PXITERATOR
