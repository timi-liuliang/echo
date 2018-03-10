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


#ifndef PX_FOUNDATION_PSBASICTEMPLATES_H
#define PX_FOUNDATION_PSBASICTEMPLATES_H

#include "Ps.h"

namespace physx
{
namespace shdfnd
{
	template<typename A>
	struct Equal
	{
		bool operator()(const A& a, const A& b)	const { return a==b; }
	};

	template<typename A>
	struct Less
	{
		bool operator()(const A& a, const A& b)	const { return a<b; }
	};

	template<typename A>
	struct Greater
	{
		bool operator()(const A& a, const A& b)	const { return a>b; }
	};


	template <class F, class S> 
	class Pair
	{
	public:
		F	first;
		S	second;
		Pair(): first(F()), second(S()) {}
		Pair(const F& f, const S& s): first(f), second(s) {}
		Pair(const Pair& p): first(p.first), second(p.second) {}
		//CN - fix for /.../PsBasicTemplates.h(61) : warning C4512: 'physx::shdfnd::Pair<F,S>' : assignment operator could not be generated
		Pair& operator=(const Pair& p)
		{
			first = p.first;
			second = p.second;
			return *this;
		}
		bool operator==(const Pair &p) const
		{
			return first == p.first && second == p.second;
		}
		bool operator<(const Pair &p) const
		{
			if (first < p.first)
				return true;
			else
				return !(p.first < first) && (second < p.second);
		}
	};

	template<unsigned int A>	struct LogTwo	{	static const unsigned int value  = LogTwo<(A>>1)>::value + 1; };
	template<>					struct LogTwo<1>{	static const unsigned int value  = 0;	};

	template<typename T> struct UnConst	{ typedef T Type; };
	template<typename T> struct UnConst<const T> { typedef T Type; };

	template<typename T> T pointerOffset(void *p, ptrdiff_t offset) { return reinterpret_cast<T>(reinterpret_cast<char *>(p)+offset); }
	template<typename T> T pointerOffset(const void *p, ptrdiff_t offset) { return reinterpret_cast<T>(reinterpret_cast<const char *>(p)+offset); }

} // namespace shdfnd
} // namespace physx

#endif
