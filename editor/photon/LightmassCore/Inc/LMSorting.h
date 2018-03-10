/*=============================================================================
	LMSorting.h: Generic sorting definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

namespace Lightmass
{

/*----------------------------------------------------------------------------
	Sorting template.
----------------------------------------------------------------------------*/
#define IMPLEMENT_COMPARE_POINTER_( RetType, Type, Filename, FunctionBody )			\
class Compare##Filename##Type##Pointer												\
	{																				\
	public:																			\
		static inline RetType Compare( Type* A, Type* B	)							\
			FunctionBody															\
	};


#define IMPLEMENT_COMPARE_CONSTPOINTER_( RetType, Type, Filename, FunctionBody )	\
class Compare##Filename##Type##ConstPointer											\
	{																				\
	public:																			\
		static inline RetType Compare( const Type* A, const Type* B	)				\
			FunctionBody															\
	};


#define IMPLEMENT_COMPARE_CONSTREF_( RetType, Type, Filename, FunctionBody )		\
class Compare##Filename##Type##ConstRef												\
	{																				\
	public:																			\
		static inline RetType Compare( const Type& A, const Type& B	)				\
			FunctionBody															\
	};

#define IMPLEMENT_COMPARE_POINTER( Type, Filename, FunctionBody )		\
	IMPLEMENT_COMPARE_POINTER_(INT, Type, Filename, FunctionBody)

#define IMPLEMENT_COMPARE_CONSTPOINTER( Type, Filename, FunctionBody )	\
	IMPLEMENT_COMPARE_CONSTPOINTER_(INT, Type, Filename, FunctionBody)

#define IMPLEMENT_COMPARE_CONSTREF( Type, Filename, FunctionBody )		\
	IMPLEMENT_COMPARE_CONSTREF_(INT, Type, Filename, FunctionBody)

#define COMPARE_CONSTREF_CLASS( Type, Filename )		Compare##Filename##Type##ConstRef
#define COMPARE_POINTER_CLASS( Type, Filename )			Compare##Filename##Type##Pointer
#define COMPARE_CONSTPOINTER_CLASS( Type, Filename )	Compare##Filename##Type##ConstPointer

#define USE_COMPARE_POINTER( Type, Filename )			Type*,Compare##Filename##Type##Pointer	
#define USE_COMPARE_CONSTPOINTER( Type, Filename )		Type*,Compare##Filename##Type##ConstPointer	
#define USE_COMPARE_CONSTREF( Type, Filename )			Type,Compare##Filename##Type##ConstRef	

/**
 * Sort elements. The sort is unstable, meaning that the ordering of equal items is not necessarily preserved.
 *
 * @param	First	pointer to the first element to sort
 * @param	Num		the number of items to sort
 */
template<class T, class CompareClass> void Sort( T* First, INT Num )
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
		PTRINT Count = Current.Max - Current.Min + 1;
		if( Count <= 8 )
		{
			// Use simple bubble-sort.
			while( Current.Max > Current.Min )
			{
				T *Max, *Item;
				for( Max=Current.Min, Item=Current.Min+1; Item<=Current.Max; Item++ )
				{
					if( CompareClass::Compare(*Item, *Max) > 0 )
					{
						Max = Item;
					}
				}
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
				while( ++Inner.Min<=Current.Max && CompareClass::Compare(*Inner.Min, *Current.Min) <= 0 );
				while( --Inner.Max> Current.Min && CompareClass::Compare(*Inner.Max, *Current.Min) >= 0 );
				if( Inner.Min>Inner.Max )
				{
					break;
				}
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

}

