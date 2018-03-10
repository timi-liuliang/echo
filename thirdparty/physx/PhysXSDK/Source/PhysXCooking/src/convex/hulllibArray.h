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

#ifndef HULL_LIB_ARRAY_H
#define HULL_LIB_ARRAY_H

#include "CmPhysXCommon.h"
#include "PxAssert.h"

#define STANDALONE 0  // This #define is used when tranferring this source code to other projects

#if STANDALONE

#undef PX_ALLOC
#undef PX_FREE_AND_RESET

#define PX_ALLOC(x) malloc(x)
#define PX_FREE_AND_RESET(x) free(x)

#else
#include "PsUserAllocated.h"
#endif

namespace physx
{
	namespace hullLibArray
	{

	template <class Type> class ArrayRet;
	template <class Type> class Array
	{
		public:
					Array(int s=0);
					Array(Array<Type> &array);
					Array(ArrayRet<Type> &array);
					~Array();
		void		allocate(int s);
		void		SetSize(int s);
		void		Pack();
		Type&		Add(Type);
		void		AddUnique(Type);
		int 		Contains(Type);
		void		Insert(Type,int);
		int			IndexOf(Type);
		void		Remove(Type);
		void		DelIndex(int i);
		Type *		element;
		int			count;
		int			array_size;
		const Type	&operator[](int i) const { PX_ASSERT(i>=0 && i<count);  return element[i]; }
		Type		&operator[](int i)  { PX_ASSERT(i>=0 && i<count);  return element[i]; }
		Type		&Pop() { PX_ASSERT(count); count--;  return element[count]; }
		Array<Type> &operator=(Array<Type> &array);
		Array<Type> &operator=(ArrayRet<Type> &array);
		// operator ArrayRet<Type> &() { return *(ArrayRet<Type> *)this;} // this worked but i suspect could be dangerous
	};

	template <class Type> class ArrayRet:public Array<Type>
	{
	};

	template <class Type> Array<Type>::Array(int s)
	{
		count=0;
		array_size = 0;
		element = NULL;
		if(s)
		{
			allocate(s);
		}
	}


	template <class Type> Array<Type>::Array(Array<Type> &array)
	{
		count=0;
		array_size = 0;
		element = NULL;
		for(int i=0;i<array.count;i++)
		{
			Add(array[i]);
		}
	}


	template <class Type> Array<Type>::Array(ArrayRet<Type> &array)
	{
		*this = array;
	}
	template <class Type> Array<Type> &Array<Type>::operator=(ArrayRet<Type> &array)
	{
		count=array.count;
		array_size = array.array_size;
		element = array.element;
		array.element=NULL;
		array.count=0;
		array.array_size=0;
		return *this;
	}


	template <class Type> Array<Type> &Array<Type>::operator=(Array<Type> &array)
	{
		count=0;
		for(int i=0;i<array.count;i++)
		{
			Add(array[i]);
		}
		return *this;
	}

	template <class Type> Array<Type>::~Array()
	{
		if (element != NULL)
		{
		  PX_FREE_AND_RESET(element);
		}
		count=0;array_size=0;element=NULL;
	}

	template <class Type> void Array<Type>::allocate(int s)
	{
		PX_ASSERT(s>0);
		PX_ASSERT(s>=count);
		Type *old = element;
		array_size =s;
		element = (Type *) PX_ALLOC_TEMP( sizeof(Type)*array_size, PX_DEBUG_EXP("hull array memory"));
		PX_ASSERT(element && (!count || old));
		for(int i=0;i<count;i++)
		{
			element[i]=old[i];
		}
		if(old)
		{
			PX_FREE_AND_RESET(old);
		}
	}

	template <class Type> void Array<Type>::SetSize(int s)
	{
		if(s==0)
		{
			if(element)
			{
				PX_FREE_AND_RESET(element);
				element = NULL;
			}
 		  array_size = s;
		}
		else
		{
			allocate(s);
		}
		count=s;
	}

	template <class Type> void Array<Type>::Pack()
	{
		allocate(count);
	}

	template <class Type> Type& Array<Type>::Add(Type t)
	{
		PX_ASSERT(count<=array_size);
		if(count==array_size)
		{
			allocate((array_size)?array_size *2:16);
		}
		element[count++] = t;
		return element[count-1];
	}

	template <class Type> int Array<Type>::Contains(Type t)
	{
		int i;
		int found=0;
		for(i=0;i<count;i++)
		{
			if(element[i] == t) found++;
		}
		return found;
	}

	template <class Type> void Array<Type>::AddUnique(Type t)
	{
		if(!Contains(t)) Add(t);
	}


	template <class Type> void Array<Type>::DelIndex(int i)
	{
		PX_ASSERT(i<count);
		count--;
		while(i<count)
		{
			element[i] = element[i+1];
			i++;
		}
	}

	template <class Type> void Array<Type>::Remove(Type t)
	{
		int i;
		for(i=0;i<count;i++)
		{
			if(element[i] == t)
			{
				break;
			}
		}
		PX_ASSERT(i<count); // PX_ASSERT object t is in the array.
		DelIndex(i);
		for(i=0;i<count;i++)
		{
			PX_ASSERT(element[i] != t);
		}
	}

	template <class Type> void Array<Type>::Insert(Type t,int k)
	{
		int i=count;
		Add(t); // to allocate space
		while(i>k)
		{
			element[i]=element[i-1];
			i--;
		}
		PX_ASSERT(i==k);
		element[k]=t;
	}


	template <class Type> int Array<Type>::IndexOf(Type t)
	{
		int i;
		for(i=0;i<count;i++)
		{
			if(element[i] == t)
			{
				return i;
			}
		}
		PX_ASSERT(0);
		return -1;
	}

	}

}

#endif
