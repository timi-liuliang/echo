/*=============================================================================
	LMTemplates.h: Unreal common template definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once


namespace Lightmass
{



/*-----------------------------------------------------------------------------
	Standard templates.
-----------------------------------------------------------------------------*/

template< class T > inline T Abs( const T A )
{
	return (A>=(T)0) ? A : -A;
}
template< class T > inline T Sgn( const T A )
{
	return (A>0) ? 1 : ((A<0) ? -1 : 0);
}
template< class T > inline T Max( const T A, const T B )
{
	return (A>=B) ? A : B;
}
template< class T > inline T Min( const T A, const T B )
{
	return (A<=B) ? A : B;
}
template< class T > inline T Max3( const T A, const T B, const T C )
{
	return Max ( Max( A, B ), C );
}
template< class T > inline T Min3( const T A, const T B, const T C )
{
	return Min ( Min( A, B ), C );
}
template< class T > inline T Square( const T A )
{
	return A*A;
}
template< class T > inline T Clamp( const T X, const T Min, const T Max )
{
	return X<Min ? Min : X<Max ? X : Max;
}
/* Only works on powers of two, Alignment is in bytes */
template< class T > inline T Align( const T Ptr, INT Alignment )
{
	return (T)(((PTRINT)Ptr + Alignment - 1) & ~(Alignment-1));
}
template< class T > inline void Swap( T& A, T& B )
{
	const T Temp = A;
	A = B;
	B = Temp;
}
template< class T > inline void Exchange( T& A, T& B )
{
	Swap(A, B);
}
template< class T > FORCEINLINE void appSort2( T& A, T& B )
{
	if ( A > B )
	{
		Swap(A, B);
	}
}

/**
 * Chooses between the two parameters based on whether the first is NULL or not.
 * @return If the first parameter provided is non-NULL, it is returned; otherwise the second parameter is returned.
 */
template<typename ReferencedType>
ReferencedType* IfAThenAElseB(ReferencedType* A,ReferencedType* B)
{
	const PTRINT IntA = reinterpret_cast<PTRINT>(A);
	const PTRINT IntB = reinterpret_cast<PTRINT>(B);

	// Compute a mask which has all bits set if IntA is zero, and no bits set if it's non-zero.
	const PTRINT MaskB = -(!IntA);

	return reinterpret_cast<ReferencedType*>(IntA | (MaskB & IntB));
}

/** This is used to provide type specific behavior for a move which may change the value of B. */
template<typename T> void Move(T& A,typename TContainerTraits<T>::ConstInitType B)
{
	// Destruct the previous value of A.
	A.~T();

	// Use placement new and a copy constructor so types with const members will work.
	new(&A) T(B);
}

template< class T, class U > T Lerp( const T& A, const T& B, const U& Alpha )
{
	return (T)(A + Alpha * (B-A));
}

template<class T> T BiLerp(const T& P00,const T& P10,const T& P01,const T& P11,FLOAT FracX,FLOAT FracY)
{
	return Lerp(
			Lerp(P00,P10,FracX),
			Lerp(P01,P11,FracX),
			FracY
			);
}

// P - end points
// T - tangent directions at end points
// Alpha - distance along spline
template< class T, class U > T CubicInterp( const T& P0, const T& T0, const T& P1, const T& T1, const U& A )
{
	const FLOAT A2 = A  * A;
	const FLOAT A3 = A2 * A;

	return (T)(((2*A3)-(3*A2)+1) * P0) + ((A3-(2*A2)+A) * T0) + ((A3-A2) * T1) + (((-2*A3)+(3*A2)) * P1);
}

template< class T, class U > T CubicInterpDerivative( const T& P0, const T& T0, const T& P1, const T& T1, const U& A )
{
	T a = 6.f*P0 + 3.f*T0 + 3.f*T1 - 6.f*P1;
	T b = -6.f*P0 - 4.f*T0 - 2.f*T1 + 6.f*P1;
	T c = T0;

	const FLOAT A2 = A  * A;

	return (a * A2) + (b * A) + c;
}

template< class T, class U > T CubicInterpSecondDerivative( const T& P0, const T& T0, const T& P1, const T& T1, const U& A )
{
	T a = 12.f*P0 + 6.f*T0 + 6.f*T1 - 12.f*P1;
	T b = -6.f*P0 - 4.f*T0 - 2.f*T1 + 6.f*P1;

	return (a * A) + b;
}

/**
 * Sort elements, allowing access to local data in the sort function through the use of an instance of the comparison class. 
 * The sort is unstable, meaning that the ordering of equal items is not necessarily preserved.
 *
 * @param	First	pointer to the first element to sort
 * @param	Num		the number of items to sort
 */
template<class T, class CompareClass> void SortWithLocalDataComparison( T* First, INT Num, const CompareClass& CompareClassInstance )
{
	struct FStack
	{
		T* Min;
		T* Max;
	};

	if( Num < 2 )
	{
		return;
	}
	FStack RecursionStack[32]={{First,First+Num-1}}, Current, Inner;
	for( FStack* StackTop=RecursionStack; StackTop>=RecursionStack; --StackTop )
	{
		Current = *StackTop;
	Loop:
		INT Count = Current.Max - Current.Min + 1;
		if( Count <= 8 )
		{
			// Use simple bubble-sort.
			while( Current.Max > Current.Min )
			{
				T *Max, *Item;
				for( Max=Current.Min, Item=Current.Min+1; Item<=Current.Max; Item++ )
					if( CompareClassInstance.Compare(*Item, *Max) > 0 )
						Max = Item;
				Exchange( *Max, *Current.Max-- );
			}
		}
		else
		{
			// Grab middle element so sort doesn't exhibit worst-cast behavior with presorted lists.
			Exchange( Current.Min[Count/2], Current.Min[0] );

			// Divide list into two halves, one with items <=Current.Min, the other with items >Current.Max.
			Inner.Min = Current.Min;
			Inner.Max = Current.Max+1;
			for( ; ; )
			{
				while( ++Inner.Min<=Current.Max && CompareClassInstance.Compare(*Inner.Min, *Current.Min) <= 0 );
				while( --Inner.Max> Current.Min && CompareClassInstance.Compare(*Inner.Max, *Current.Min) >= 0 );
				if( Inner.Min>Inner.Max )
					break;
				Exchange( *Inner.Min, *Inner.Max );
			}
			Exchange( *Current.Min, *Inner.Max );

			// Save big half and recurse with small half.
			if( Inner.Max-1-Current.Min >= Current.Max-Inner.Min )
			{
				if( Current.Min+1 < Inner.Max )
				{
					StackTop->Min = Current.Min;
					StackTop->Max = Inner.Max - 1;
					StackTop++;
				}
				if( Current.Max>Inner.Min )
				{
					Current.Min = Inner.Min;
					goto Loop;
				}
			}
			else
			{
				if( Current.Max>Inner.Min )
				{
					StackTop->Min = Inner  .Min;
					StackTop->Max = Current.Max;
					StackTop++;
				}
				if( Current.Min+1<Inner.Max )
				{
					Current.Max = Inner.Max - 1;
					goto Loop;
				}
			}
		}
	}
}

/*----------------------------------------------------------------------------
	FLOAT specialization of templates.
----------------------------------------------------------------------------*/

#if PS3
template<> FORCEINLINE FLOAT Abs( const FLOAT A )
{
	return __fabsf( A );
}
#else
template<> FORCEINLINE FLOAT Abs( const FLOAT A )
{
	return fabsf( A );
}
#endif

#if XBOX
template<> FORCEINLINE FLOAT Max( const FLOAT A, const FLOAT B )
{
	return __fsel( A - B, A, B );
}
template<> FORCEINLINE FLOAT Min( const FLOAT A, const FLOAT B )
{
	return __fsel( A - B, B, A );
}
//@todo optimization: the below causes crashes in release mode when compiled with VS.NET 2003
#elif __HAS_SSE__ && 0
template<> FORCEINLINE FLOAT Max( const FLOAT A, const FLOAT B )
{
	return _mm_max_ss( _mm_set_ss(A), _mm_set_ss(B) ).m128_f32[0];
}
template<> FORCEINLINE FLOAT Min( const FLOAT A, const FLOAT B )
{
	return _mm_max_ss( _mm_set_ss(A), _mm_set_ss(B) ).m128_f32[0];
}
#elif PS3
template<> FORCEINLINE FLOAT Max( const FLOAT A, const FLOAT B )
{
	return __fsels( A - B, A, B );
}
template<> FORCEINLINE FLOAT Min( const FLOAT A, const FLOAT B )
{
	return __fsels( A - B, B, A );
}
#endif

/*----------------------------------------------------------------------------
	DWORD specialization of templates.
----------------------------------------------------------------------------*/

/** Returns the smaller of the two values */
template<> FORCEINLINE DWORD Min(const DWORD A, const DWORD B)
{
    // Negative if B is less than A (i.e. the high bit will be set)
	DWORD Delta  = B - A;
	// Relies on sign bit rotating in
    DWORD Mask   = static_cast<INT>(Delta) >> 31;
    DWORD Result = A + (Delta & Mask);

    return Result;
}

/*----------------------------------------------------------------------------
	Standard macros.
----------------------------------------------------------------------------*/

// Number of elements in an array.
#define ARRAY_COUNT( array ) \
	( sizeof(array) / sizeof((array)[0]) )

// Offset of a struct member.

#ifdef __GNUC__
/**
 * gcc3 thinks &((myclass*)NULL)->member is an invalid use of the offsetof
 * macro. This is a broken heuristic in the compiler and the workaround is
 * to use a non-zero offset.
 */
#define STRUCT_OFFSET( struc, member )	( ( (PTRINT)&((struc*)0x1)->member ) - 0x1 )
#else
#define STRUCT_OFFSET( struc, member )	( (PTRINT)&((struc*)NULL)->member )
#endif

/*-----------------------------------------------------------------------------
	Allocators.
-----------------------------------------------------------------------------*/

template <class T> class TAllocator
{};

/**
 * works just like std::min_element.
 */
template<class ForwardIt> inline
ForwardIt MinElement(ForwardIt First, ForwardIt Last)
{
	ForwardIt Result = First;
	for (; ++First != Last; )
	{
		if (*First < *Result) 
		{
			Result = First;
		}
	}
	return Result;
}

/**
 * works just like std::min_element.
 */
template<class ForwardIt, class PredicateType> inline
ForwardIt MinElement(ForwardIt First, ForwardIt Last, PredicateType Predicate)
{
	ForwardIt Result = First;
	for (; ++First != Last; )
	{
		if (Predicate(*First,*Result))
		{
			Result = First;
		}
	}
	return Result;
}

/**
* works just like std::max_element.
*/
template<class ForwardIt> inline
ForwardIt MaxElement(ForwardIt First, ForwardIt Last)
{
	ForwardIt Result = First;
	for (; ++First != Last; )
	{
		if (*Result < *First) 
		{
			Result = First;
		}
	}
	return Result;
}

/**
* works just like std::max_element.
*/
template<class ForwardIt, class PredicateType> inline
ForwardIt MaxElement(ForwardIt First, ForwardIt Last, PredicateType Predicate)
{
	ForwardIt Result = First;
	for (; ++First != Last; )
	{
		if (Predicate(*Result,*First))
		{
			Result = First;
		}
	}
	return Result;
}

/**
 * utility template for a class that should not be copyable.
 * Derive from this class to make your class non-copyable
 */
class FNoncopyable
{
protected:
	// ensure the class cannot be constructed directly
	FNoncopyable() {}
	// the class should not be used polymorphically
	~FNoncopyable() {}
private:
	FNoncopyable(const FNoncopyable&);
	FNoncopyable& operator=(const FNoncopyable&);
};


/** 
 * exception-safe guard around saving/restoring a value.
 * Commonly used to make sure a value is restored 
 * even if the code early outs in the future.
 * Usage:
 *  	TGuardValue<UBOOL> GuardSomeBool(bSomeBool, FALSE); // Sets bSomeBool to FALSE, and restores it in dtor.
 */
template <typename Type>
struct TGuardValue : private FNoncopyable
{
	TGuardValue(Type& ReferenceValue, const Type& NewValue)
	: RefValue(ReferenceValue), OldValue(ReferenceValue)
	{
		RefValue = NewValue;
	}
	~TGuardValue()
	{
		RefValue = OldValue;
	}

	/**
	 * Overloaded dereference operator.
	 * Provides read-only access to the original value of the data being tracked by this struct
	 *
	 * @return	a const reference to the original data value
	 */
	FORCEINLINE const Type& operator*() const
	{
		return OldValue;
	}

private:
	Type& RefValue;
	Type OldValue;
};

/** Chooses between two different classes based on a boolean. */
template<bool Predicate,typename TrueClass,typename FalseClass>
class TChooseClass {};

template<typename TrueClass,typename FalseClass>
class TChooseClass<true,TrueClass,FalseClass>
{
public:
	typedef TrueClass Result;
};

template<typename TrueClass,typename FalseClass>
class TChooseClass<false,TrueClass,FalseClass>
{
public:
	typedef FalseClass Result;
};



/*=============================================================================
	ContainerAllocationPolicies.h: Defines allocation policies for containers.
=============================================================================*/

#pragma once

// #include "MemoryBase.h"

/** The default slack calculation heuristic. */
extern SIZE_T DefaultCalculateSlack(SIZE_T NumElements,SIZE_T NumAllocatedElements,SIZE_T BytesPerElement);

/** Used to determine the alignment of an element type. */
template<typename ElementType>
class TElementAlignmentCalculator
{
private:

	/**
	 * This is a dummy type that's used to calculate the padding added between the byte and the element
	 * to fulfill the type's required alignment.
	 */
	struct FAlignedElements
	{
		BYTE MisalignmentPadding;
		ElementType Element;

		/** FAlignedElement's default constructor is declared but never implemented to avoid the need for a ElementType default constructor. */
		FAlignedElements();
	};

public:

	enum { Alignment = sizeof(FAlignedElements) - sizeof(ElementType) };
};


/** A type which is used to represent a script type that is unknown at compile time. */
struct FScriptContainerElement
{
};


/**
 * Used to declare an untyped array of data with compile-time alignment.
 * It needs to use template specialization as the MS_ALIGN and GCC_ALIGN macros require literal parameters.
 */
template<INT Size,DWORD Alignment>
class TAlignedBytes
{
	BYTE Data[-Size]; // this intentionally won't compile, we don't support the requested alignment
};

/** Unaligned storage. */
template<INT Size>
struct TAlignedBytes<Size,1>
{
	BYTE Pad[Size];
};


// C++/CLI doesn't support alignment of native types in managed code, so we enforce that the element
// size is a multiple of the desired alignment
#ifdef __cplusplus_cli
	#define IMPLEMENT_ALIGNED_STORAGE(Align) \
		template<INT Size>        \
		struct TAlignedBytes<Size,Align> \
		{ \
			BYTE Pad[Size]; \
			checkAtCompileTime( Size % Align == 0, CLRInteropTypesMustNotBeAligned ); \
		};
#else
/** A macro that implements TAlignedBytes for a specific alignment. */
#define IMPLEMENT_ALIGNED_STORAGE(Align) \
	template<INT Size>        \
	struct TAlignedBytes<Size,Align> \
	{ \
		struct MS_ALIGN(Align) TPadding \
		{ \
			BYTE Pad[Size]; \
		} GCC_ALIGN(Align); \
		TPadding Padding; \
	};
#endif

// Implement TAlignedBytes for these alignments.
IMPLEMENT_ALIGNED_STORAGE(16);
IMPLEMENT_ALIGNED_STORAGE(8);
IMPLEMENT_ALIGNED_STORAGE(4);
IMPLEMENT_ALIGNED_STORAGE(2);

#undef IMPLEMENT_ALIGNED_STORAGE

/** An untyped array of data with compile-time alignment and size derived from another type. */
template<typename ElementType>
class TTypeCompatibleBytes :
	public TAlignedBytes<
		sizeof(ElementType),
		TElementAlignmentCalculator<ElementType>::Alignment
		>
{};

/** This is the allocation policy interface; it exists purely to document the policy's interface, and should not be used. */
class FContainerAllocatorInterface
{
public:

	/** Determines whether the user of the allocator may use the ForAnyElementType inner class. */
	enum { NeedsElementType = TRUE };

	/**
	 * A class that receives both the explicit allocation policy template parameters specified by the user of the container,
	 * but also the implicit ElementType template parameter from the container type.
	 */
	template<typename ElementType>
	class ForElementType
	{
		/** Accesses the container's current data. */
		ElementType* GetAllocation() const;

		/**
		 * Resizes the container's allocation.
		 * @param PreviousNumElements - The number of elements that were stored in the previous allocation.
		 * @param NumElements - The number of elements to allocate space for.
		 * @param NumBytesPerElement - The number of bytes/element.
		 */
		void ResizeAllocation(
			SIZE_T PreviousNumElements,
			SIZE_T NumElements,
			SIZE_T NumBytesPerElement
			);

		/**
		 * Calculates the amount of slack to allocate for an array that has just grown to a given number of elements.
		 * @param NumElements - The number of elements to allocate space for.
		 * @param CurrentNumSlackElements - The current number of slack elements allocated.
		 * @param NumBytesPerElement - The number of bytes/element.
		 */
		SIZE_T CalculateSlack(
			SIZE_T NumElements,
			SIZE_T CurrentNumSlackElements,
			SIZE_T NumBytesPerElement
			) const;
	};

	/** A class that may be used when NeedsElementType=TRUE is specified. */
	typedef ForElementType<FScriptContainerElement> ForAnyElementType;
};

/** The indirect allocation policy always allocates the elements indirectly. */
template<DWORD Alignment = DEFAULT_ALIGNMENT>
class TAlignedHeapAllocator
{
public:

	enum { NeedsElementType = FALSE };

	class ForAnyElementType
	{
	public:

		/** Default constructor. */
		ForAnyElementType()
			: Data(NULL)
		{}

		/** ENoInit constructor. */
		ForAnyElementType(ENoInit)
		{}

		/** Destructor. */
		~ForAnyElementType()
		{
			if(Data)
			{
				appFree(Data);
				Data = NULL;
			}
		}

		// FContainerAllocatorInterface
		FORCEINLINE FScriptContainerElement* GetAllocation() const
		{
			return Data;
		}
		void ResizeAllocation(
			SIZE_T PreviousNumElements,
			SIZE_T NumElements,
			SIZE_T NumBytesPerElement
			)
		{
			// Avoid calling appRealloc( NULL, 0 ) as ANSI C mandates returning a valid pointer which is not what we want.
			if( Data || NumElements )
			{
				//checkSlow(((QWORD)NumElements*(QWORD)ElementTypeInfo.GetSize() < (QWORD)INT_MAX));
				Data = (FScriptContainerElement*)appRealloc( Data, NumElements*NumBytesPerElement, DEFAULT_ALIGNMENT );
			}
		}
		SIZE_T CalculateSlack(
			SIZE_T NumElements,
			SIZE_T NumAllocatedElements,
			SIZE_T NumBytesPerElement
			) const
		{
			return DefaultCalculateSlack(NumElements,NumAllocatedElements,NumBytesPerElement);
		}

	private:

		/** A pointer to the container's elements. */
		FScriptContainerElement* Data;
	};

	template<typename ElementType>
	class ForElementType : public ForAnyElementType
	{
	public:

		/** Default constructor. */
		ForElementType()
		{}

		/** ENoInit constructor. */
		ForElementType(ENoInit)
		:	ForAnyElementType(E_NoInit)
		{}

		FORCEINLINE ElementType* GetAllocation() const
		{
			return (ElementType*)ForAnyElementType::GetAllocation();
		}
	};
};

/** The indirect allocation policy always allocates the elements indirectly. */
class FHeapAllocator
{
public:

	enum { NeedsElementType = FALSE };

	class ForAnyElementType
	{
	public:

		/** Default constructor. */
		ForAnyElementType()
			: Data(NULL)
		{}

		/** ENoInit constructor. */
		ForAnyElementType(ENoInit)
		{}

		/** Destructor. */
		~ForAnyElementType()
		{
			if(Data)
			{
				appFree(Data);
				Data = NULL;
			}
		}

		// FContainerAllocatorInterface
		FORCEINLINE SIZE_T GetAllocatedSize(INT ArrayMax, INT NumBytesPerElement) const { return SIZE_T(ArrayMax) * SIZE_T(NumBytesPerElement); }
		FORCEINLINE FScriptContainerElement* GetAllocation() const
		{
			return Data;
		}
		void ResizeAllocation(SIZE_T PreviousNumElements,SIZE_T NumElements,SIZE_T NumBytesPerElement)
		{
			// Avoid calling appRealloc( NULL, 0 ) as ANSI C mandates returning a valid pointer which is not what we want.
			if( Data || NumElements )
			{
				//checkSlow(((QWORD)NumElements*(QWORD)ElementTypeInfo.GetSize() < (QWORD)INT_MAX));
				Data = (FScriptContainerElement*)appRealloc( Data, NumElements*NumBytesPerElement );
			}
		}
		SIZE_T CalculateSlack(SIZE_T NumElements,SIZE_T NumAllocatedElements,SIZE_T NumBytesPerElement) const
		{
			return DefaultCalculateSlack(NumElements,NumAllocatedElements,NumBytesPerElement);
		}

	private:

		/** A pointer to the container's elements. */
		FScriptContainerElement* Data;
	};
	
	template<typename ElementType>
	class ForElementType : public ForAnyElementType
	{
	public:

		/** Default constructor. */
		ForElementType()
		{}

		/** ENoInit constructor. */
		ForElementType(ENoInit)
		:	ForAnyElementType(E_NoInit)
		{}

		FORCEINLINE ElementType* GetAllocation() const
		{
			return (ElementType*)ForAnyElementType::GetAllocation();
		}
	};
};

/** The indirect allocation policy with DEFAULT_ALIGNMENT is used by default. */
class FDefaultAllocator : public FHeapAllocator
{
};

/**
 * The inline allocation policy allocates up to a specified number of bytes in the same allocation as the container.
 * Any allocation needed beyond that causes all data to be moved into an indirect allocation.
 * It always uses DEFAULT_ALIGNMENT.
 */
template<UINT NumInlineElements,typename SecondaryAllocator = FDefaultAllocator >
class TInlineAllocator
{
public:

	enum { NeedsElementType = TRUE };

	template<typename ElementType>
	class ForElementType
	{
	public:

		/** Default constructor. */
		ForElementType()
		{
		}

		/** ENoInit constructor. */
		ForElementType(ENoInit)
		{}

		/** Destructor. */
		~ForElementType()
		{
		}

		// FContainerAllocatorInterface
		FORCEINLINE SIZE_T GetAllocatedSize(INT ArrayMax, INT NumBytesPerElement) const { return SecondaryData.GetAllocation() ? SecondaryData.GetAllocatedSize(ArrayMax, NumBytesPerElement) : 0; }
		FORCEINLINE ElementType* GetAllocation() const
		{
			return IfAThenAElseB<ElementType>(SecondaryData.GetAllocation(),GetInlineElements());
		}
		void ResizeAllocation(SIZE_T PreviousNumElements,SIZE_T NumElements,SIZE_T NumBytesPerElement)
		{
			const INT PreviousNumBytes = PreviousNumElements * NumBytesPerElement;

			// Check if the new allocation will fit in the inline data area.
			if(NumElements <= NumInlineElements)
			{
				// If the old allocation wasn't in the inline data area, move it into the inline data area.
				if(SecondaryData.GetAllocation())
				{
					appMemcpy(GetInlineElements(),SecondaryData.GetAllocation(),PreviousNumBytes);

					// Free the old indirect allocation.
					SecondaryData.ResizeAllocation(0,0,NumBytesPerElement);
				}
			}
			else
			{
				if(!SecondaryData.GetAllocation())
				{
					// Allocate new indirect memory for the data.
					SecondaryData.ResizeAllocation(0,NumElements,NumBytesPerElement);

					// Move the data out of the inline data area into the new allocation.
					appMemcpy(SecondaryData.GetAllocation(),GetInlineElements(),PreviousNumBytes);
				}
				else
				{
					// Reallocate the indirect data for the new size.
					SecondaryData.ResizeAllocation(PreviousNumElements,NumElements,NumBytesPerElement);
				}
			}
		}
		SIZE_T CalculateSlack(SIZE_T NumElements,SIZE_T NumAllocatedElements,SIZE_T NumBytesPerElement) const
		{
			// If the elements use less space than the inline allocation, only use the inline allocation as slack.
			return NumElements <= NumInlineElements ?
				NumInlineElements :
				SecondaryData.CalculateSlack(NumElements,NumAllocatedElements,NumBytesPerElement);
		}

	private:

		/** The data is stored in this array if less than NumInlineBytes is needed. */
		TTypeCompatibleBytes<ElementType> InlineData[NumInlineElements];

		/** The data is allocated through the indirect allocation policy if more than NumInlineBytes is needed. */
		typename SecondaryAllocator::template ForElementType<ElementType> SecondaryData;

		/** @return the base of the aligned inline element data */
		ElementType* GetInlineElements() const
		{
			return (ElementType*)InlineData;
		}
	};

	typedef ForElementType<FScriptContainerElement> ForAnyElementType;
};

/** Bit arrays use a 4 DWORD inline allocation policy by default. */
class FDefaultBitArrayAllocator : public TInlineAllocator<4>
{
};

enum { NumBitsPerDWORD = 32 };
enum { NumBitsPerDWORDLogTwo = 5 };

//
// Sparse array allocation definitions
//

/** Encapsulates the allocators used by a sparse array in a single type. */
template<typename InElementAllocator = FDefaultAllocator,typename InBitArrayAllocator = FDefaultBitArrayAllocator>
class TSparseArrayAllocator
{
public:

	typedef InElementAllocator ElementAllocator;
	typedef InBitArrayAllocator BitArrayAllocator;
};

/** An inline sparse array allocator that allows sizing of the inline allocations for a set number of elements. */
template<
	UINT NumInlineElements,
	typename SecondaryAllocator = TSparseArrayAllocator<FDefaultAllocator,FDefaultAllocator>
	>
class TInlineSparseArrayAllocator
{
private:

	/** The size to allocate inline for the bit array. */
	enum { InlineBitArrayDWORDs = (NumInlineElements + NumBitsPerDWORD - 1) / NumBitsPerDWORD};

public:

	typedef TInlineAllocator<NumInlineElements,typename SecondaryAllocator::ElementAllocator>		ElementAllocator;
	typedef TInlineAllocator<InlineBitArrayDWORDs,typename SecondaryAllocator::BitArrayAllocator>	BitArrayAllocator;
};

class FDefaultSparseArrayAllocator : public TSparseArrayAllocator<>
{
};

//
// Set allocation definitions.
//

#define DEFAULT_NUMBER_OF_ELEMENTS_PER_HASH_BUCKET	2
#define DEFAULT_BASE_NUMBER_OF_HASH_BUCKETS			8
#define DEFAULT_MIN_NUMBER_OF_HASHED_ELEMENTS		4

/** Encapsulates the allocators used by a set in a single type. */
template<
	typename InSparseArrayAllocator = TSparseArrayAllocator<>,
	typename InHashAllocator = TInlineAllocator<1,FDefaultAllocator>,
	UINT AverageNumberOfElementsPerHashBucket = DEFAULT_NUMBER_OF_ELEMENTS_PER_HASH_BUCKET,
	UINT BaseNumberOfHashBuckets = DEFAULT_BASE_NUMBER_OF_HASH_BUCKETS,
	UINT MinNumberOfHashedElements = DEFAULT_MIN_NUMBER_OF_HASHED_ELEMENTS
	>
class TSetAllocator
{
public:

	/** Computes the number of hash buckets to use for a given number of elements. */
	static FORCEINLINE UINT GetNumberOfHashBuckets(UINT NumHashedElements)
	{
		if(NumHashedElements >= MinNumberOfHashedElements)
		{
			return appRoundUpToPowerOfTwo(NumHashedElements / AverageNumberOfElementsPerHashBucket + BaseNumberOfHashBuckets);
		}
		else
		{
			return 1;
		}
	}

	typedef InSparseArrayAllocator SparseArrayAllocator;
	typedef InHashAllocator HashAllocator;
};

/** An inline set allocator that allows sizing of the inline allocations for a set number of elements. */
template<
	UINT NumInlineElements,
	typename SecondaryAllocator = TSetAllocator<TSparseArrayAllocator<FDefaultAllocator,FDefaultAllocator>,FDefaultAllocator>,
	UINT AverageNumberOfElementsPerHashBucket = DEFAULT_NUMBER_OF_ELEMENTS_PER_HASH_BUCKET,
	UINT MinNumberOfHashedElements = DEFAULT_MIN_NUMBER_OF_HASHED_ELEMENTS
	>
class TInlineSetAllocator
{
private:

	enum { NumInlineHashBuckets = (NumInlineElements + AverageNumberOfElementsPerHashBucket - 1) / AverageNumberOfElementsPerHashBucket };

public:

	/** Computes the number of hash buckets to use for a given number of elements. */
	static FORCEINLINE UINT GetNumberOfHashBuckets(UINT NumHashedElements)
	{
		const UINT NumDesiredHashBuckets = appRoundUpToPowerOfTwo(NumHashedElements / AverageNumberOfElementsPerHashBucket);
		if(NumDesiredHashBuckets < NumInlineHashBuckets)
        {
	        return NumInlineHashBuckets;
		}
		else if(NumHashedElements < MinNumberOfHashedElements)
		{
			return NumInlineHashBuckets;
		}
		else
		{
			return NumDesiredHashBuckets;
		}
	}

	typedef TInlineSparseArrayAllocator<NumInlineElements,typename SecondaryAllocator::SparseArrayAllocator> SparseArrayAllocator;
	typedef TInlineAllocator<NumInlineHashBuckets,typename SecondaryAllocator::HashAllocator> HashAllocator;
};

class FDefaultSetAllocator : public TSetAllocator<>
{
};


/*-----------------------------------------------------------------------------
	Simple templates
-----------------------------------------------------------------------------*/

//
// Simple single-linked list template.
//
template <class ElementType> class TList
{
public:

	ElementType			Element;
	TList<ElementType>*	Next;

	// Constructor.

	TList(const ElementType &InElement, TList<ElementType>* InNext = NULL)
	{
		Element = InElement;
		Next = InNext;
	}
};


/**
 * The base class of reference counted objects.
 */
class FRefCountedObject
{
public:
	FRefCountedObject(): NumRefs(0) {}
	virtual ~FRefCountedObject() { check(!NumRefs); }
	DWORD AddRef() const
	{
		return DWORD(++NumRefs);
	}
	DWORD Release() const
	{
		DWORD Refs = DWORD(--NumRefs);
		if(Refs == 0)
		{
			delete this;
		}
		return Refs;
	}
	DWORD GetRefCount() const
	{
		return DWORD(NumRefs);
	}
private:
	mutable INT NumRefs;
};

/**
 * A smart pointer to an object which implements AddRef/Release.
 */
template<typename ReferencedType>
class TRefCountPtr
{
	typedef ReferencedType* ReferenceType;
public:

	TRefCountPtr():
		Reference(NULL)
	{}

	TRefCountPtr(ReferencedType* InReference,UBOOL bAddRef = TRUE)
	{
		Reference = InReference;
		if(Reference && bAddRef)
		{
			Reference->AddRef();
		}
	}

	TRefCountPtr(const TRefCountPtr& Copy)
	{
		Reference = Copy.Reference;
		if(Reference)
		{
			Reference->AddRef();
		}
	}

	~TRefCountPtr()
	{
		if(Reference)
		{
			Reference->Release();
		}
	}

	TRefCountPtr& operator=(ReferencedType* InReference)
	{
		// Call AddRef before Release, in case the new reference is the same as the old reference.
		ReferencedType* OldReference = Reference;
		Reference = InReference;
		if(Reference)
		{
			Reference->AddRef();
		}
		if(OldReference)
		{
			OldReference->Release();
		}
		return *this;
	}
	
	TRefCountPtr& operator=(const TRefCountPtr& InPtr)
	{
		return *this = InPtr.Reference;
	}

	UBOOL operator==(const TRefCountPtr& Other) const
	{
		return Reference == Other.Reference;
	}

	ReferencedType* operator->() const
	{
		return Reference;
	}

	operator ReferenceType() const
	{
		return Reference;
	}

	ReferencedType** GetInitReference()
	{
		*this = NULL;
		return &Reference;
	}

	ReferencedType* GetReference() const
	{
		return Reference;
	}

	friend UBOOL IsValidRef(const TRefCountPtr& Reference)
	{
		return Reference.Reference != NULL;
	}

	void SafeRelease()
	{
		*this = NULL;
	}

	DWORD GetRefCount()
	{
		if(Reference)
		{
			Reference->AddRef();
			return Reference->Release();
		}
		else
		{
			return 0;
		}
	}

private:
	ReferencedType* Reference;
};



} // namespace
