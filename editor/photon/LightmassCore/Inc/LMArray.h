/*=============================================================================
	LMArray.h: Dynamic array (TArray) definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

namespace Lightmass
{


template<typename ElementType,typename Allocator = FDefaultAllocator>
class TBaseArray;

template<typename T,typename Allocator = FDefaultAllocator>
class TArray;

template<typename KeyType,typename ValueType,typename SetAllocator = FDefaultSetAllocator >
class TMap;

template<typename KeyType,typename ValueType,typename SetAllocator = FDefaultSetAllocator >
class TMultiMap;

/**
* Generic non-const iterator which can operate on types that expose the following:
* - A type called ElementType representing the contained type.
* - A method IndexType Num() const that returns the number of items in the container.
* - A method UBOOL IsValidIndex(IndexType index) which returns whether a given index is valid in the container.
* - A method T& operator(IndexType index) which returns a reference to a contained object by index.
*/
template< typename ContainerType, typename IndexType = INT >
class TIndexedContainerIterator
{
public:
	typedef typename ContainerType::ElementType ElementType;

	TIndexedContainerIterator(ContainerType& InContainer)
		:	Container( InContainer )
		,	Index(0)
	{
	}

	/** Advances iterator to the next element in the container. */
	TIndexedContainerIterator& operator++()
	{
		++Index;
		return *this;
	}
	TIndexedContainerIterator operator++(int)
	{
		TIndexedContainerIterator Tmp(*this);
		++Index;
		return Tmp;
	}

	/** Moves iterator to the previous element in the container. */
	TIndexedContainerIterator& operator--()
	{
		--Index;
		return *this;
	}
	TIndexedContainerIterator operator--(int)
	{
		TIndexedContainerIterator Tmp(*this);
		--Index;
		return Tmp;
	}

	/** pointer arithmetic support */
	TIndexedContainerIterator& operator+=(INT Offset)
	{
		Index += Offset;
		return *this;
	}

	TIndexedContainerIterator operator+(INT Offset) const
	{
		TIndexedContainerIterator Tmp(*this);
		return Tmp += Offset;
	}

	TIndexedContainerIterator& operator-=(INT Offset)
	{
		return *this += -Offset;
	}

	TIndexedContainerIterator operator-(INT Offset) const
	{
		TIndexedContainerIterator Tmp(*this);
		return Tmp -= Offset;
	}

	TIndexedContainerIterator operator()(INT Offset) const
	{
		return *this + Offset;
	}

	/** @name Element access */
	//@{
	ElementType& operator* () const
	{
		return Container( Index );
	}

	ElementType* operator-> () const
	{
		return &Container( Index );
	}
	//@}

	/** conversion to "bool" returning TRUE if the iterator has not reached the last element. */
	typedef bool PrivateBooleanType;
	operator PrivateBooleanType() const
	{
		return Container.IsValidIndex(Index) ? &TIndexedContainerIterator::Index : NULL;
	}

	/** inverse of the "bool" operator */
	bool operator !() const
	{
		return !operator PrivateBooleanType();
	}

	/** Returns an index to the current element. */
	IndexType GetIndex() const
	{
		return Index;
	}

	/** Resets the iterator to the first element. */
	void Reset()
	{
		Index = 0;
	}
private:
	ContainerType&	Container;
	IndexType		Index;
};

/** operator + */
template< typename ContainerType, typename IndexType >
TIndexedContainerIterator<ContainerType, IndexType> operator+(
	INT Offset,
	TIndexedContainerIterator<ContainerType, IndexType> RHS)
{
	return RHS + Offset;
}



/**
* Generic const iterator which can operate on types that expose the following:
* - A type called ElementType representing the contained type.
* - A method IndexType Num() const that returns the number of items in the container.
* - A method UBOOL IsValidIndex(IndexType index) which returns whether a given index is valid in the container.
* - A method T& operator(IndexType index) const which returns a reference to a contained object by index.
*/
template< typename ContainerType, typename IndexType = INT >
class TIndexedContainerConstIterator
{
public:
	typedef typename ContainerType::ElementType ElementType;

	TIndexedContainerConstIterator(const ContainerType& InContainer)
		:	Container( InContainer )
		,	Index(0)
	{
	}

	/** Advances iterator to the next element in the container. */
	TIndexedContainerConstIterator& operator++()
	{
		++Index;
		return *this;
	}
	TIndexedContainerConstIterator operator++(int)
	{
		TIndexedContainerConstIterator Tmp(*this);
		++Index;
		return Tmp;
	}

	/** Moves iterator to the previous element in the container. */
	TIndexedContainerConstIterator& operator--()
	{
		--Index;
		return *this;
	}
	TIndexedContainerConstIterator operator--(int)
	{
		TIndexedContainerConstIterator Tmp(*this);
		--Index;
		return Tmp;
	}

	/** iterator arithmetic support */
	TIndexedContainerConstIterator& operator+=(INT Offset)
	{
		Index += Offset;
		return *this;
	}

	TIndexedContainerConstIterator operator+(INT Offset) const
	{
		TIndexedContainerConstIterator Tmp(*this);
		return Tmp += Offset;
	}

	TIndexedContainerConstIterator& operator-=(INT Offset)
	{
		return *this += -Offset;
	}

	TIndexedContainerConstIterator operator-(INT Offset) const
	{
		TIndexedContainerConstIterator Tmp(*this);
		return Tmp -= Offset;
	}

	TIndexedContainerConstIterator operator()(INT Offset) const
	{
		return *this + Offset;
	}

	/** @name Element access */
	//@{
	const ElementType& operator* () const
	{
		return Container( Index );
	}

	const ElementType* operator-> () const
	{
		return &Container( Index );
	}
	//@}

	/** conversion to "bool" returning TRUE if the iterator has not reached the last element. */
	typedef bool PrivateBooleanType;
	operator PrivateBooleanType() const
	{
		return Container.IsValidIndex(Index) ? &TIndexedContainerConstIterator::Index : NULL;
	}

	/** inverse of the "bool" operator */
	bool operator !() const
	{
		return !operator PrivateBooleanType();
	}

	/** Returns an index to the current element. */
	IndexType GetIndex() const
	{
		return Index;
	}

	/** Resets the iterator to the first element. */
	void Reset()
	{
		Index = 0;
	}
private:
	const ContainerType&	Container;
	IndexType				Index;
};

/** operator + */
template< typename ContainerType, typename IndexType >
TIndexedContainerConstIterator<ContainerType, IndexType> operator+(
	INT Offset,
	TIndexedContainerConstIterator<ContainerType, IndexType> RHS)
{
	return RHS + Offset;
}


/**
 * Templated dynamic array
 *
 * A dynamically sized array of typed elements.  Makes the assumption that your elements are relocate-able; 
 * i.e. that they can be transparently moved to new memory without a copy constructor.  The main implication 
 * is that pointers to elements in the TArray may be invalidated by adding or removing other elements to the array. 
 * Removal of elements is O(N) and invalidates the indices of subsequent elements.
 *
 */
template<typename InElementType, typename Allocator>
class TArray
{
public:
	typedef InElementType ElementType;

	TArray()
	:   ArrayNum( 0 )
	,	ArrayMax( 0 )
	{}
	TArray( ENoInit )
	:	AllocatorInstance(E_NoInit)
	{}
	explicit TArray( INT InNum )
	:   ArrayNum( InNum )
	,	ArrayMax( InNum )
	{
		AllocatorInstance.ResizeAllocation(0,ArrayMax,sizeof(ElementType));
	}

	/**
	 * Copy constructor. Use the common routine to perform the copy
	 *
	 * @param Other the source array to copy
	 */
	template<typename OtherAllocator>
	TArray(const TArray<ElementType,OtherAllocator>& Other)
	:   ArrayNum( 0 )
	,	ArrayMax( 0 )
	{
		Copy(Other);
	}

	TArray(const TArray<ElementType,Allocator>& Other)
	:   ArrayNum( 0 )
	,	ArrayMax( 0 )
	{
		Copy(Other);
	}

	~TArray()
	{
		checkSlow(ArrayNum>=0);
		checkSlow(ArrayMax>=ArrayNum);
		DestructItems(0,ArrayNum);
		ArrayNum = ArrayMax = 0;

		#if defined(_MSC_VER)
			// ensure that DebugGet gets instantiated.
			//@todo it would be nice if we had a cleaner solution for DebugGet
			volatile const ElementType* Dummy = &DebugGet(0);
		#endif
	}

	/**
	 * Helper function for returning a typed pointer to the first array entry.
	 *
	 * @return pointer to first array entry or NULL if ArrayMax==0
	 */
	FORCEINLINE ElementType* GetTypedData()
	{
		return (ElementType*)AllocatorInstance.GetAllocation();
	}
	FORCEINLINE ElementType* GetData()
	{
		return (ElementType*)AllocatorInstance.GetAllocation();
	}
	/**
	 * Helper function for returning a typed pointer to the first array entry.
	 *
	 * @return pointer to first array entry or NULL if ArrayMax==0
	 */
	FORCEINLINE const ElementType* GetTypedData() const
	{
		return (ElementType*)AllocatorInstance.GetAllocation();
	}
	FORCEINLINE const ElementType* GetData() const
	{
		return (ElementType*)AllocatorInstance.GetAllocation();
	}
	/** 
	 * Helper function returning the size of the inner type
	 *
	 * @return size in bytes of array type
	 */
	FORCEINLINE DWORD GetTypeSize() const
	{
		return sizeof(ElementType);
	}

	/** 
	 * Helper function to return the amount of memory allocated by this container 
	 *
	 * @return number of bytes allocated by this container
	 */
	FORCEINLINE SIZE_T GetAllocatedSize( void ) const
	{
		return AllocatorInstance.GetAllocatedSize(ArrayMax, sizeof(ElementType));
	}

	/**
	 * Returns the amount of slack in this array in elements.
	 */
	INT GetSlack() const
	{
		return ArrayMax - ArrayNum;
	}

	FORCEINLINE UBOOL IsValidIndex( INT i ) const
	{
		return i>=0 && i<ArrayNum;
	}
	FORCEINLINE INT Num() const
	{
		checkSlow(ArrayNum>=0);
		checkSlow(ArrayMax>=ArrayNum);
		return ArrayNum;
	}

	FORCEINLINE ElementType& operator()( INT i )
	{
		checkSlow(i>=0 && (i<ArrayNum||(i==0 && ArrayNum==0)) ); // (i==0 && ArrayNum==0) is workaround for &MyArray(0) abuse
		checkSlow(ArrayMax>=ArrayNum);
		return GetTypedData()[i];
	}
	FORCEINLINE const ElementType& operator()( INT i ) const
	{
		checkSlow(i>=0 && (i<ArrayNum||(i==0 && ArrayNum==0)) ); // (i==0 && ArrayNum==0) is workaround for &MyArray(0) abuse
		checkSlow(ArrayMax>=ArrayNum);
		return GetTypedData()[i];
	}
	ElementType Pop()
	{
		check(ArrayNum>0);
		checkSlow(ArrayMax>=ArrayNum);
		ElementType Result = GetTypedData()[ArrayNum-1];
		Remove( ArrayNum-1 );
		return Result;
	}
	void Push( const ElementType& Item )
	{
		AddItem(Item);
	}
	ElementType& Top()
	{
		return Last();
	}
	const ElementType& Top() const
	{
		return Last();
	}
	ElementType& Last( INT c=0 )
	{
		check(AllocatorInstance.GetAllocation());
		check(c<ArrayNum);
		checkSlow(ArrayMax>=ArrayNum);
		return GetTypedData()[ArrayNum-c-1];
	}
	const ElementType& Last( INT c=0 ) const
	{
		check(GetTypedData());
		checkSlow(c<ArrayNum);
		checkSlow(ArrayMax>=ArrayNum);
		return GetTypedData()[ArrayNum-c-1];
	}
	void Shrink()
	{
		checkSlow(ArrayNum>=0);
		checkSlow(ArrayMax>=ArrayNum);
		if( ArrayMax != ArrayNum )
		{
			ArrayMax = ArrayNum;
			AllocatorInstance.ResizeAllocation(ArrayNum,ArrayMax,sizeof(ElementType));
		}
	}
	UBOOL FindItem( const ElementType& Item, INT& Index ) const
	{
		const ElementType* const RESTRICT DataEnd = GetTypedData() + ArrayNum;
		for(const ElementType* RESTRICT Data = GetTypedData();
			Data < DataEnd;
			Data++
			)
		{
			if( *Data==Item )
			{
				Index = (INT)(Data - GetTypedData());
				return TRUE;
			}
		}
		return FALSE;
	}
	INT FindItemIndex( const ElementType& Item ) const
	{
		const ElementType* const RESTRICT DataEnd = GetTypedData() + ArrayNum;
		for(const ElementType* RESTRICT Data = GetTypedData();
			Data < DataEnd;
			Data++
			)
		{
			if( *Data==Item )
			{
				return (INT)(Data - GetTypedData());
			}
		}
		return INDEX_NONE;
	}
	UBOOL ContainsItem( const ElementType& Item ) const
	{
		return ( FindItemIndex(Item) != INDEX_NONE );
	}
	UBOOL operator==(const TArray& OtherArray) const
	{
		if(Num() != OtherArray.Num())
			return FALSE;
		for(INT Index = 0;Index < Num();Index++)
		{
			if(!((*this)(Index) == OtherArray(Index)))
				return FALSE;
		}
		return TRUE;
	}
	UBOOL operator!=(const TArray& OtherArray) const
	{
		if(Num() != OtherArray.Num())
			return TRUE;
		for(INT Index = 0;Index < Num();Index++)
		{
			if(!((*this)(Index) == OtherArray(Index)))
				return TRUE;
		}
		return FALSE;
	}

	// Add, Insert, Remove, Empty interface.
	INT Add( INT Count=1 )
	{
		check(Count>=0);
		checkSlow(ArrayNum>=0);
		checkSlow(ArrayMax>=ArrayNum);

		const INT OldNum = ArrayNum;
		if( (ArrayNum+=Count)>ArrayMax )
		{
			ArrayMax = AllocatorInstance.CalculateSlack( ArrayNum, ArrayMax, sizeof(ElementType) );
			AllocatorInstance.ResizeAllocation(OldNum,ArrayMax, sizeof(ElementType));
		}

		return OldNum;
	}
	void Insert( INT Index, INT Count=1 )
	{
		check(Count>=0);
		check(ArrayNum>=0);
		check(ArrayMax>=ArrayNum);
		check(Index>=0);
		check(Index<=ArrayNum);

		const INT OldNum = ArrayNum;
		if( (ArrayNum+=Count)>ArrayMax )
		{
			ArrayMax = AllocatorInstance.CalculateSlack( ArrayNum, ArrayMax, sizeof(ElementType) );
			AllocatorInstance.ResizeAllocation(OldNum,ArrayMax,sizeof(ElementType));
		}
		appMemmove
		(
			(BYTE*)AllocatorInstance.GetAllocation() + (Index+Count )*sizeof(ElementType),
			(BYTE*)AllocatorInstance.GetAllocation() + (Index       )*sizeof(ElementType),
			                                               (OldNum-Index)*sizeof(ElementType)
		);
	}
	void InsertZeroed( INT Index, INT Count=1 )
	{
		Insert( Index, Count );
		appMemzero( (BYTE*)AllocatorInstance.GetAllocation()+Index*sizeof(ElementType), Count*sizeof(ElementType) );
	}
	INT InsertItem( const ElementType& Item, INT Index )
	{
		// construct a copy in place at Index (this new operator will insert at 
		// Index, then construct that memory with Item)
		Insert(Index,1);
		new(GetTypedData() + Index) ElementType(Item);
		return Index;
	}
	void Remove( INT Index, INT Count=1 )
	{
		checkSlow(Count >= 0);
		check(Index>=0);
		check(Index<=ArrayNum);
		check(Index+Count<=ArrayNum);

		DestructItems(Index,Count);

		// Skip memmove in the common case that there is nothing to move.
		INT NumToMove = ArrayNum - Index - Count;
		if( NumToMove )
		{
			appMemmove
			(
				(BYTE*)AllocatorInstance.GetAllocation() + (Index      ) * sizeof(ElementType),
				(BYTE*)AllocatorInstance.GetAllocation() + (Index+Count) * sizeof(ElementType),
				NumToMove * sizeof(ElementType)
			);
		}
		ArrayNum -= Count;
		
		const INT NewArrayMax = AllocatorInstance.CalculateSlack(ArrayNum,ArrayMax,sizeof(ElementType));
		if(NewArrayMax != ArrayMax)
		{
			ArrayMax = NewArrayMax;
			AllocatorInstance.ResizeAllocation(ArrayNum,ArrayMax,sizeof(ElementType));
		}
		checkSlow(ArrayNum >= 0);
		checkSlow(ArrayMax >= ArrayNum);
	}
	// RemoveSwap, this version is much more efficient O(Count) instead of O(ArrayNum), but does not preserve the order
	void RemoveSwap( INT Index, INT Count=1 )
	{
		check(Index>=0);
		check(Index<=ArrayNum);
		check(Index+Count<=ArrayNum);

		DestructItems(Index,Count);
		
		checkSlow(Count >= 0);
		checkSlow(Index >= 0); 
		checkSlow(Index <= ArrayNum);
		checkSlow(Index + Count <= ArrayNum);

		// Replace the elements in the hole created by the removal with elements from the end of the array, so the range of indices used by the array is contiguous.
		const INT NumElementsInHole = Count;
		const INT NumElementsAfterHole = ArrayNum - (Index + Count);
		const INT NumElementsToMoveIntoHole = Min(NumElementsInHole,NumElementsAfterHole);
		if(NumElementsToMoveIntoHole)
		{
			appMemcpy(
				(BYTE*)AllocatorInstance.GetAllocation() + (Index                             ) * sizeof(ElementType),
				(BYTE*)AllocatorInstance.GetAllocation() + (ArrayNum-NumElementsToMoveIntoHole) * sizeof(ElementType),
				NumElementsToMoveIntoHole * sizeof(ElementType)
				);
		}
		ArrayNum -= Count;

		const INT NewArrayMax = AllocatorInstance.CalculateSlack(ArrayNum,ArrayMax,sizeof(ElementType));
		if(NewArrayMax != ArrayMax)
		{
			ArrayMax = NewArrayMax;
			AllocatorInstance.ResizeAllocation(ArrayNum,ArrayMax,sizeof(ElementType));
		}
		checkSlow(ArrayNum >= 0);
		checkSlow(ArrayMax >= ArrayNum);
	}
	void Empty( INT Slack=0 )
	{
		DestructItems(0,ArrayNum);

		checkSlow(Slack>=0);
		ArrayNum = 0;
		// only reallocate if we need to, I don't trust realloc to the same size to work
		if (ArrayMax != Slack)
		{
			ArrayMax = Slack;
			AllocatorInstance.ResizeAllocation(0,ArrayMax,sizeof(ElementType));
		}
	}

	/**
	 * Appends the specified array to this array.
	 * Cannot append to self.
	 */
	FORCEINLINE void Append(const TArray& Source)
	{
		// Do nothing if the source and target match, or the source is empty.
		if ( this != &Source && Source.Num() > 0 )
		{
			// Allocate memory for the new elements.
			Reserve( ArrayNum + Source.ArrayNum );

			if ( TContainerTraits<ElementType>::NeedsConstructor )
			{
				// Construct each element.
				for ( INT Index = 0 ; Index < Source.ArrayNum ; ++Index )
				{
					new(GetTypedData() + ArrayNum + Index) ElementType(Source(Index));
				}
			}
			else
			{
				// Do a bulk copy.
				appMemcpy( (BYTE*)AllocatorInstance.GetAllocation() + ArrayNum * sizeof(ElementType), Source.AllocatorInstance.GetAllocation(), sizeof(ElementType) * Source.ArrayNum );
			}
			ArrayNum += Source.ArrayNum;
		}
	}

	/**
	 * Appends the specified array to this array.
	 * Cannot append to self.
	 */
	TArray& operator+=( const TArray& Other )
	{
		Append( Other );
		return *this;
	}

	/**
	 * Copies the source array into this one. Uses the common copy method
	 *
	 * @param Other the source array to copy
	 */
	template<typename OtherAllocator>
	TArray& operator=( const TArray<ElementType,OtherAllocator>& Other )
	{
		Copy(Other);
		return *this;
	}

	TArray& operator=( const TArray<ElementType,Allocator>& Other )
	{
		Copy(Other);
		return *this;
	}

	INT AddItem( const ElementType& Item )
	{
		const INT Index = Add(1);
		new(GetTypedData() + Index) ElementType(Item);
		return Index;
	}
	INT AddZeroed( INT Count=1 )
	{
		const INT Index = Add( Count );
		appMemzero( (BYTE*)AllocatorInstance.GetAllocation()+Index*sizeof(ElementType), Count*sizeof(ElementType) );
		return Index;
	}
	INT AddUniqueItem( const ElementType& Item )
	{
		for( INT Index=0; Index<ArrayNum; Index++ )
			if( (*this)(Index)==Item )
				return Index;
		return AddItem( Item );
	}

	/**
	 * Reserves memory such that the array can contain at least Number elements.
	 */
	void Reserve(INT Number)
	{
		if (Number > ArrayMax)
		{
			ArrayMax = Number;
			AllocatorInstance.ResizeAllocation(ArrayNum,ArrayMax,sizeof(ElementType));
		}
	}

	/** Removes as many instances of Item as there are in the array, maintaining order but not indices. */
	INT RemoveItem( const ElementType& Item )
	{
		// It isn't valid to specify an Item that is in the array, since removing that item will change Item's value.
		check( ((&Item) < GetTypedData()) || ((&Item) >= GetTypedData()+ArrayMax) );

		INT DestIndex = 0;
		for( INT Index=0; Index<ArrayNum; Index++ )
		{
			if( GetTypedData()[Index] == Item )
			{
				// Destruct items that match the specified Item.
				DestructItems(Index,1);
			}
			else
			{
				// Move items that don't match the specified Item to fill holes left by removed items.
				if(Index != DestIndex)
				{
					appMemmove(&GetTypedData()[DestIndex],&GetTypedData()[Index],sizeof(ElementType));
				}
				DestIndex++;
			}
		}

		// Update the array count.
		const INT OriginalNum = ArrayNum;
		ArrayNum = DestIndex;

		return OriginalNum - ArrayNum;
	}

	/** RemoveItemSwap, this version is much more efficient O(Count) instead of O(ArrayNum), but does not preserve the order */
	INT RemoveItemSwap( const ElementType& Item )
	{
		check( ((&Item) < (ElementType*)AllocatorInstance.GetAllocation()) || ((&Item) >= (ElementType*)AllocatorInstance.GetAllocation()+ArrayMax) );
		const INT OriginalNum=ArrayNum;
		for( INT Index=0; Index<ArrayNum; Index++ )
		{
			if( (*this)(Index)==Item )
			{
				RemoveSwap( Index-- );
			}
		}
		return OriginalNum - ArrayNum;
	}

	void Swap(INT A, INT B)
	{
		appMemswap(
			(BYTE*)AllocatorInstance.GetAllocation()+(sizeof(ElementType)*A),
			(BYTE*)AllocatorInstance.GetAllocation()+(sizeof(ElementType)*B),
			sizeof(ElementType)
			);
	}

	void SwapItems(INT A, INT B)
	{
		check((A >= 0) && (B >= 0));
		check((ArrayNum > A) && (ArrayNum > B));
		if (A != B)
		{
			Swap(A,B);
		}
	}

	/**
	 * Same as empty, but doesn't change memory allocations. It calls the
	 * destructors on held items if needed and then zeros the ArrayNum.
	 */
	void Reset(void)
	{
		DestructItems(0,ArrayNum);
		ArrayNum = 0;
	}

	/**
	 * Searches for the first entry of the specified type, will only work
	 * with TArray<UObject*>.  Optionally return the item's index, and can
	 * specify the start index.
	 */
	template<typename SearchType> UBOOL FindItemByClass(SearchType **Item = NULL, INT *ItemIndex = NULL, INT StartIndex = 0)
	{
		UClass* SearchClass = SearchType::StaticClass();
		for (INT Idx = StartIndex; Idx < ArrayNum; Idx++)
		{
			if ((*this)(Idx) != NULL && (*this)(Idx)->IsA(SearchClass))
			{
				if (Item != NULL)
				{
					*Item = (SearchType*)((*this)(Idx));
				}
				if (ItemIndex != NULL)
				{
					*ItemIndex = Idx;
				}
				return TRUE;
			}
		}
		return FALSE;
	}

	// Iterators
	typedef TIndexedContainerIterator< TArray<ElementType,Allocator> >  TIterator;
	typedef TIndexedContainerConstIterator< TArray<ElementType,Allocator> >  TConstIterator;

#if defined(_MSC_VER)
private:
	/**
	* Helper function that can be used inside the debuggers watch window to debug TArrays. E.g. "*Class->Defaults.DebugGet(5)". 
	*
	* @param	i	Index
	* @return		pointer to type T at Index i
	*/
	FORCENOINLINE const ElementType& DebugGet( INT i ) const
	{
		return GetTypedData()[i];
	}
#endif

protected:

	/**
	 * Copies data from one array into this array. Uses the fast path if the
	 * data in question does not need a constructor.
	 *
	 * @param Source the source array to copy
	 */
	template<typename OtherAllocator>
	void Copy(const TArray<ElementType,OtherAllocator>& Source)
	{
		if ((void*)this != (void*)&Source)
		{
			// Just empty our array if there is nothing to copy
			if (Source.Num() > 0)
			{
				// Presize the array so there are no extra allocs/memcpys
				Empty(Source.Num());
				// Determine whether we need per element construction or bulk
				// copy is fine
				if (TContainerTraits<ElementType>::NeedsConstructor)
				{
					// Use the inplace new to copy the element to an array element
					for (INT Index = 0; Index < Source.Num(); Index++)
					{
						new(GetTypedData() + Index) ElementType(Source(Index));
					}
				}
				else
				{
					// Use the much faster path for types that allow it
					appMemcpy(AllocatorInstance.GetAllocation(),&Source(0),sizeof(ElementType) * Source.Num());
				}
				ArrayNum = Source.Num();
			}
			else
			{
				Empty(0);
			}
		}
	}
	
	/** Destructs a range of items in the array. */
	FORCEINLINE void DestructItems(INT Index,INT Count)
	{
		if( TContainerTraits<ElementType>::NeedsDestructor )
		{
			for( INT i=Index; i<Index+Count; i++ )
			{
				(&(*this)(i))->~ElementType();
			}
		}
	}

	enum { AllocatorNeedsElementType = Allocator::NeedsElementType };

	typedef typename TChooseClass<
		AllocatorNeedsElementType,
		typename Allocator::template ForElementType<ElementType>,
		typename Allocator::ForAnyElementType
		>::Result ElementAllocatorType;

	ElementAllocatorType AllocatorInstance;
	INT	  ArrayNum;
	INT	  ArrayMax;
};

/** A specialization of the exchange macro that avoids reallocating when exchanging two arrays. */
template <typename T> inline void Exchange( TArray<T>& A, TArray<T>& B )
{
	appMemswap( &A, &B, sizeof(TArray<T>) );
}

/** A specialization of the exchange macro that avoids reallocating when exchanging two arrays. */
template<typename ElementType,typename Allocator>
inline void Exchange( TArray<ElementType,Allocator>& A, TArray<ElementType,Allocator>& B )
{
	appMemswap( &A, &B, sizeof(TArray<ElementType,Allocator>) );
}


/*-----------------------------------------------------------------------------
	TBitArray
-----------------------------------------------------------------------------*/



// Forward declaration.
template<typename Allocator = FDefaultBitArrayAllocator>
class TBitArray;

template<typename Allocator = FDefaultBitArrayAllocator>
class TConstSetBitIterator;

#if LM_BITARRAY
template<typename Allocator = FDefaultBitArrayAllocator,typename OtherAllocator = FDefaultBitArrayAllocator>
class TConstDualSetBitIterator;

#endif


/** Used to read/write a bit in the array as a UBOOL. */
class FBitReference
{
public:

	FBitReference(DWORD& InData,DWORD InMask)
	:	Data(InData)
	,	Mask(InMask)
	{}

	FORCEINLINE operator UBOOL() const
	{
		 return (Data & Mask) != 0;
	}
	FORCEINLINE void operator=(const UBOOL NewValue)
	{
		if(NewValue)
		{
			Data |= Mask;
		}
		else
		{
			Data &= ~Mask;
		}
	}

private:
	DWORD& Data;
	DWORD Mask;
};

/** Used to read a bit in the array as a UBOOL. */
class FConstBitReference
{
public:

	FConstBitReference(const DWORD& InData,DWORD InMask)
	:	Data(InData)
	,	Mask(InMask)
	{}

	FORCEINLINE operator UBOOL() const
	{
		 return (Data & Mask) != 0;
	}

private:
	const DWORD& Data;
	DWORD Mask;
};

/** Used to reference a bit in an unspecified bit array. */
class FRelativeBitReference
{
	template<typename>
	friend class TBitArray;
	template<typename>
	friend class TConstSetBitIterator;
	template<typename,typename>
	friend class TConstDualSetBitIterator;
public:

	FRelativeBitReference(INT BitIndex)
	:	DWORDIndex(BitIndex >> NumBitsPerDWORDLogTwo)
	,	Mask(1 << (BitIndex & (NumBitsPerDWORD - 1)))
	{}

protected:
	INT DWORDIndex;
	DWORD Mask;
};

/**
 * A dynamically sized bit array.
 * An array of Booleans.  They stored in one bit/Boolean.  There are iterators that efficiently iterate over only set bits.
 */
template<typename Allocator /*= FDefaultBitArrayAllocator*/>
class TBitArray : protected Allocator::template ForElementType<DWORD>
{
public:

	template<typename>
	friend class TConstSetBitIterator;

	template<typename,typename>
	friend class TConstDualSetBitIterator;

	/**
	 * Minimal initialization constructor.
	 * @param Value - The value to initial the bits to.
	 * @param InNumBits - The initial number of bits in the array.
	 */
	TBitArray( const UBOOL Value = FALSE, const INT InNumBits = 0 )
	:	NumBits(0)
	,	MaxBits(0)
	{
		Init(Value,InNumBits);
	}

	/**
	 * Copy constructor.
	 */
	TBitArray(const TBitArray& Copy)
	:	NumBits(0)
	,	MaxBits(0)
	{
		*this = Copy;
	}

	/**
	 * Assignment operator.
	 */
	TBitArray& operator=(const TBitArray& Copy)
	{
		// check for self assignment since we don't use swamp() mechanic
		if( this == &Copy )
		{
			return *this;
		}

		Empty(Copy.Num());
		NumBits = MaxBits = Copy.NumBits;
		if(NumBits)
		{
			const INT NumDWORDs = (MaxBits + NumBitsPerDWORD - 1) / NumBitsPerDWORD;
			Realloc(0);
			appMemcpy(GetData(),Copy.GetData(),NumDWORDs * sizeof(DWORD));
		}
		return *this;
	}

	/**
	 * Adds a bit to the array with the given value.
	 * @return The index of the added bit.
	 */
	INT AddItem(const UBOOL Value)
	{
		const INT Index = NumBits;
		const UBOOL bReallocate = (NumBits + 1) > MaxBits;

		NumBits++;

		if(bReallocate)
		{
			// Allocate memory for the new bits.
			const UINT MaxDWORDs = this->CalculateSlack(
				(NumBits + NumBitsPerDWORD - 1) / NumBitsPerDWORD,
				(MaxBits + NumBitsPerDWORD - 1) / NumBitsPerDWORD,
				sizeof(DWORD)
				);
			MaxBits = MaxDWORDs * NumBitsPerDWORD;
			Realloc(NumBits - 1);
		}

		(*this)(Index) = Value;

		return Index;
	}

	/**
	 * Removes all bits from the array, potentially leaving space allocated for an expected number of bits about to be added.
	 * @param ExpectedNumBits - The expected number of bits about to be added.
	 */
	void Empty(INT ExpectedNumBits = 0)
	{
		NumBits = 0;

		// If the expected number of bits doesn't match the allocated number of bits, reallocate.
		if(MaxBits != ExpectedNumBits)
		{
			MaxBits = ExpectedNumBits;
			Realloc(0);
		}
	}

	/**
	 * Resets the array's contents.
	 * @param Value - The value to initial the bits to.
	 * @param NumBits - The number of bits in the array.
	 */
	void Init(UBOOL Value,INT InNumBits)
	{
		Empty(InNumBits);
		if(InNumBits)
		{
			NumBits = InNumBits;
			appMemset(GetData(),Value ? 0xff : 0,(NumBits + NumBitsPerDWORD - 1) / NumBitsPerDWORD * sizeof(DWORD));
		}
	}

	/**
	 * Removes bits from the array.
	 * @param BaseIndex - The index of the first bit to remove.
	 * @param NumBitsToRemove - The number of consecutive bits to remove.
	 */
	void Remove(INT BaseIndex,INT NumBitsToRemove = 1)
	{
		check(BaseIndex >= 0 && BaseIndex + NumBitsToRemove <= NumBits);

		// Until otherwise necessary, this is an obviously correct implementation rather than an efficient implementation.
		FIterator WriteIt(*this);
		for(FConstIterator ReadIt(*this);ReadIt;++ReadIt)
		{
			// If this bit isn't being removed, write it back to the array at its potentially new index.
			if(ReadIt.GetIndex() < BaseIndex || ReadIt.GetIndex() >= BaseIndex + NumBitsToRemove)
			{
				if(WriteIt.GetIndex() != ReadIt.GetIndex())
				{
					WriteIt.GetValue() = (UBOOL)ReadIt.GetValue();
				}
				++WriteIt;
			}
		}
		NumBits -= NumBitsToRemove;
	}

	/** 
	 * Helper function to return the amount of memory allocated by this container 
	 * @return number of bytes allocated by this container
	 */
	SIZE_T GetAllocatedSize( void ) const
	{
		return (MaxBits / NumBitsPerDWORD) * sizeof(DWORD);
	}

	// Accessors.
	FORCEINLINE INT Num() const { return NumBits; }
	FORCEINLINE FBitReference operator()(INT Index)
	{
		checkSlow(Index>=0 && Index<NumBits);
		return FBitReference(
			GetData()[Index / NumBitsPerDWORD],
			1 << (Index & (NumBitsPerDWORD - 1))
			);
	}
	FORCEINLINE const FConstBitReference operator()(INT Index) const
	{
		checkSlow(Index>=0 && Index<NumBits);
		return FConstBitReference(
			GetData()[Index / NumBitsPerDWORD],
			1 << (Index & (NumBitsPerDWORD - 1))
			);
	}
	FORCEINLINE FBitReference AccessCorrespondingBit(const FRelativeBitReference& RelativeReference)
	{
		checkSlow(RelativeReference.Mask);
		checkSlow(RelativeReference.DWORDIndex >= 0);
		checkSlow(((UINT)RelativeReference.DWORDIndex + 1) * NumBitsPerDWORD - 1 - appCountLeadingZeros(RelativeReference.Mask) < (UINT)NumBits);
		return FBitReference(
			GetData()[RelativeReference.DWORDIndex],
			RelativeReference.Mask
			);
	}
	FORCEINLINE const FConstBitReference AccessCorrespondingBit(const FRelativeBitReference& RelativeReference) const
	{
		checkSlow(RelativeReference.Mask);
		checkSlow(RelativeReference.DWORDIndex >= 0);
		checkSlow(((UINT)RelativeReference.DWORDIndex + 1) * NumBitsPerDWORD - 1 - appCountLeadingZeros(RelativeReference.Mask) < (UINT)NumBits);
		return FConstBitReference(
			GetData()[RelativeReference.DWORDIndex],
			RelativeReference.Mask
			);
	}

	/** BitArray iterator. */
	class FIterator : public FRelativeBitReference
	{
	public:
		FIterator(TBitArray<Allocator>& InArray,INT StartIndex = 0)
		:	FRelativeBitReference(StartIndex)
		,	Array(InArray)
		,	Index(StartIndex)
		{
		}
		FORCEINLINE FIterator& operator++()
		{
			++Index;
			this->Mask <<= 1;
			if(!this->Mask)
			{
				// Advance to the next DWORD.
				this->Mask = 1;
				++this->DWORDIndex;
			}
			return *this;
		}
		/** conversion to "bool" returning TRUE if the iterator is valid. */
		typedef bool PrivateBooleanType;
		FORCEINLINE operator PrivateBooleanType() const { return Index < Array.Num() ? &FIterator::Index : NULL; }
		FORCEINLINE bool operator !() const { return !operator PrivateBooleanType(); }

		FORCEINLINE FBitReference GetValue() const { return FBitReference(Array.GetData()[this->DWORDIndex],this->Mask); }
		FORCEINLINE INT GetIndex() const { return Index; }
	private:
		TBitArray<Allocator>& Array;
		INT Index;
	};

	/** Const BitArray iterator. */
	class FConstIterator : public FRelativeBitReference
	{
	public:
		FConstIterator(const TBitArray<Allocator>& InArray,INT StartIndex = 0)
		:	FRelativeBitReference(StartIndex)
		,	Array(InArray)
		,	Index(StartIndex)
		{
		}
		FORCEINLINE FConstIterator& operator++()
		{
			++Index;
			this->Mask <<= 1;
			if(!this->Mask)
			{
				// Advance to the next DWORD.
				this->Mask = 1;
				++this->DWORDIndex;
			}
			return *this;
		}
		/** conversion to "bool" returning TRUE if the iterator is valid. */
		typedef bool PrivateBooleanType;
		FORCEINLINE operator PrivateBooleanType() const { return Index < Array.Num() ? &FConstIterator::Index : NULL; }
		FORCEINLINE bool operator !() const { return !operator PrivateBooleanType(); }

		FORCEINLINE FConstBitReference GetValue() const { return FConstBitReference(Array.GetData()[this->DWORDIndex],this->Mask); }
		FORCEINLINE INT GetIndex() const { return Index; }
	private:
		const TBitArray<Allocator>& Array;
		INT Index;
	};

private:
	INT NumBits;
	INT MaxBits;

	FORCEINLINE const DWORD* GetData() const
	{
		return (DWORD*)this->GetAllocation();
	}

	FORCEINLINE DWORD* GetData()
	{
		return (DWORD*)this->GetAllocation();
	}

	void Realloc(INT PreviousNumBits)
	{
		const INT PreviousNumDWORDs = (PreviousNumBits + NumBitsPerDWORD - 1) / NumBitsPerDWORD;
		const INT MaxDWORDs = (MaxBits + NumBitsPerDWORD - 1) / NumBitsPerDWORD;

		this->ResizeAllocation(PreviousNumDWORDs,MaxDWORDs,sizeof(DWORD));

		if(MaxDWORDs)
		{
			// Reset the newly allocated slack DWORDs.
			appMemzero((DWORD*)this->GetAllocation() + PreviousNumDWORDs,(MaxDWORDs - PreviousNumDWORDs) * sizeof(DWORD));
		}
	}
};


/** An iterator which only iterates over set bits. */
template<typename Allocator>
class TConstSetBitIterator : public FRelativeBitReference
{
public:

	/** Constructor. */
	TConstSetBitIterator(const TBitArray<Allocator>& InArray,INT StartIndex = 0)
	:	FRelativeBitReference(StartIndex)
	,	Array(InArray)
	,	UnvisitedBitMask((~0) << (StartIndex & (NumBitsPerDWORD - 1)))
	,	CurrentBitIndex(StartIndex)
	,	BaseBitIndex(StartIndex & ~(NumBitsPerDWORD - 1))
	{
		FindFirstSetBit();
	}

	/** Advancement operator. */
	FORCEINLINE TConstSetBitIterator& operator++()
	{
		// Mark the current bit as visited.
		UnvisitedBitMask &= ~this->Mask;

		// Find the first set bit that hasn't been visited yet.
		FindFirstSetBit();

		return *this;
	}

	/** conversion to "bool" returning TRUE if the iterator is valid. */
	typedef bool PrivateBooleanType;
	FORCEINLINE operator PrivateBooleanType() const { return CurrentBitIndex < Array.Num() ? &TConstSetBitIterator::CurrentBitIndex : NULL; }
	FORCEINLINE bool operator !() const { return !operator PrivateBooleanType(); }

	/** Index accessor. */
	FORCEINLINE INT GetIndex() const
	{
		return CurrentBitIndex;
	}

private:

	const TBitArray<Allocator>& Array;

	DWORD UnvisitedBitMask;
	INT CurrentBitIndex;
	INT BaseBitIndex;

	/** Find the first set bit starting with the current bit, inclusive. */
	void FindFirstSetBit()
	{
		static const DWORD EmptyArrayData = 0;
		const DWORD* ArrayData = IfAThenAElseB(Array.GetData(),&EmptyArrayData);

		// Advance to the next non-zero DWORD.
		DWORD RemainingBitMask = ArrayData[this->DWORDIndex] & UnvisitedBitMask;
		while(!RemainingBitMask)
		{
			this->DWORDIndex++;
			BaseBitIndex += NumBitsPerDWORD;
			const INT LastDWORDIndex = (Array.Num() - 1) / NumBitsPerDWORD;
			if(this->DWORDIndex <= LastDWORDIndex)
			{
				RemainingBitMask = ArrayData[this->DWORDIndex];
				UnvisitedBitMask = ~0;
			}
			else
			{
				// We've advanced past the end of the array.
				CurrentBitIndex = Array.Num();
				return;
			}
		};

		// We can assume that RemainingBitMask!=0 here.
		checkSlow(RemainingBitMask);

		// This operation has the effect of unsetting the lowest set bit of BitMask
		const DWORD NewRemainingBitMask = RemainingBitMask & (RemainingBitMask - 1);

		// This operation XORs the above mask with the original mask, which has the effect
		// of returning only the bits which differ; specifically, the lowest bit
		this->Mask = NewRemainingBitMask ^ RemainingBitMask;

		// If the Nth bit was the lowest set bit of BitMask, then this gives us N
		CurrentBitIndex = BaseBitIndex + NumBitsPerDWORD - 1 - appCountLeadingZeros(this->Mask);
	}
};


/*-----------------------------------------------------------------------------
	TSparseArray
-----------------------------------------------------------------------------*/





// Forward declarations.
template<typename ElementType,typename Allocator = FDefaultSparseArrayAllocator >
class TSparseArray;

/**
 * The result of a sparse array allocation.
 */
struct FSparseArrayAllocationInfo
{
	INT Index;
	void* Pointer;
};

/** Allocated elements are overlapped with free element info in the element list. */
template<typename ElementType>
union TSparseArrayElementOrFreeListLink
{
	/** If the element is allocated, its value is stored here. */
	ElementType ElementData;

	/** If the element isn't allocated, this is a link in the free element list. */
	INT NextFreeIndex;
};






/**
 * A dynamically sized array where element indices aren't necessarily contiguous.  Memory is allocated for all 
 * elements in the array's index range, so it doesn't save memory; but it does allow O(1) element removal that 
 * doesn't invalidate the indices of subsequent elements.  It uses TArray to store the elements, and a TBitArray
 * to store whether each element index is allocated (for fast iteration over allocated elements).
 *
 */
template<typename ElementType,typename Allocator /*= FDefaultSparseArrayAllocator */>
class TSparseArray
{
public:

	typedef TBitArray<typename Allocator::BitArrayAllocator> AllocationBitArrayType;

	/** Destructor. */
	~TSparseArray()
	{
		// Destruct the elements in the array.
		Empty();
	}

	/**
	 * Allocates space for an element in the array.  The element is not initialized, and you must use the corresponding placement new operator
	 * to construct the element in the allocated memory.
	 */
	FSparseArrayAllocationInfo Add()
	{
		FSparseArrayAllocationInfo Result;

		if(NumFreeIndices > 0)
		{
			// Remove and use the first index from the list of free elements.
			Result.Index = FirstFreeIndex;
			FirstFreeIndex = GetData(FirstFreeIndex).NextFreeIndex;
			--NumFreeIndices;
		}
		else
		{
			// Add a new element.
			Result.Index = Data.Add(1);
			AllocationFlags.AddItem(TRUE);
		}

		// Compute the pointer to the new element's data.
		Result.Pointer = &GetData(Result.Index).ElementData;

		// Flag the element as allocated.
		AllocationFlags(Result.Index) = TRUE;

		return Result;
	}

	/**
	 * Adds an element to the array.
	 */
	INT AddItem(typename TContainerTraits<ElementType>::ConstInitType Element)
	{
		FSparseArrayAllocationInfo Allocation = Add();
		new(Allocation) ElementType(Element);
		return Allocation.Index;
	}

	/**
	 * Removes an element from the array.
	 */
	void Remove(INT BaseIndex,INT Count = 1)
	{
		for(INT Index = 0;Index < Count;Index++)
		{
			check(AllocationFlags(BaseIndex + Index));

			// Destruct the element being removed.
			if(TContainerTraits<ElementType>::NeedsDestructor)
			{
				(*this)(BaseIndex + Index).~ElementType();
			}

			// Mark the element as free and add it to the free element list.
			GetData(BaseIndex + Index).NextFreeIndex = NumFreeIndices > 0 ? FirstFreeIndex : INDEX_NONE;
			FirstFreeIndex = BaseIndex + Index;
			++NumFreeIndices;
			AllocationFlags(BaseIndex + Index) = FALSE;
		}
	}

	/**
	 * Removes all elements from the array, potentially leaving space allocated for an expected number of elements about to be added.
	 * @param ExpectedNumElements - The expected number of elements about to be added.
	 */
	void Empty(INT ExpectedNumElements = 0)
	{
		// Destruct the allocated elements.
		if( TContainerTraits<ElementType>::NeedsDestructor )
		{
			for(TIterator It(*this);It;++It)
			{
				ElementType& Element = *It;
				Element.~ElementType();
			}
		}

		// Free the allocated elements.
		Data.Empty(ExpectedNumElements);
		FirstFreeIndex = 0;
		NumFreeIndices = 0;
		AllocationFlags.Empty(ExpectedNumElements);
	}

	/**
	 * Preallocates enough memory to contain the specified number of elements.
	 *
	 * @param	ExpectedNumElements		the total number of elements that the array will have
	 */
	void Reserve(INT ExpectedNumElements)
	{
		if ( ExpectedNumElements > Data.Num() )
		{
			const INT ElementsToAdd = ExpectedNumElements - Data.Num();
			if ( ElementsToAdd > 0 )
			{
				// allocate memory in the array itself
				INT ElementIndex = Data.Add(ElementsToAdd);

				// now mark the new elements as free
				for ( INT FreeIndex = ElementIndex; FreeIndex < ElementsToAdd; FreeIndex++ )
				{
					GetData(FreeIndex).NextFreeIndex = NumFreeIndices > 0 ? FirstFreeIndex : INDEX_NONE;
					FirstFreeIndex = FreeIndex;
					++NumFreeIndices;
				}
				//@fixme - this will have to do until TBitArray has a Reserve method....
				for ( INT i = 0; i < ElementsToAdd; i++ )
				{
					AllocationFlags.AddItem(FALSE);
				}
			}
		}
	}

	/** Shrinks the array's storage to avoid slack. */
	void Shrink()
	{
		// Determine the highest allocated index in the data array.
		INT MaxAllocatedIndex = INDEX_NONE;
		for(TConstSetBitIterator<typename Allocator::BitArrayAllocator> AllocatedIndexIt(AllocationFlags);AllocatedIndexIt;++AllocatedIndexIt)
		{
			MaxAllocatedIndex = Max(MaxAllocatedIndex,AllocatedIndexIt.GetIndex());
		}

		const INT FirstIndexToRemove = MaxAllocatedIndex + 1;
		if(FirstIndexToRemove < Data.Num())
		{
			if(NumFreeIndices > 0)
			{
				// Look for elements in the free list that are in the memory to be freed.
				INT* PreviousNextFreeIndex = &FirstFreeIndex;
				for(INT FreeIndex = FirstFreeIndex;
					FreeIndex != INDEX_NONE;
					FreeIndex = *PreviousNextFreeIndex)
				{
					if(FreeIndex >= FirstIndexToRemove)
					{
						*PreviousNextFreeIndex = GetData(FreeIndex).NextFreeIndex;
						--NumFreeIndices;
					}
					else
					{
						PreviousNextFreeIndex = &GetData(FreeIndex).NextFreeIndex;
					}
				}
			}

			// Truncate unallocated elements at the end of the data array.
			Data.Remove(FirstIndexToRemove,Data.Num() - FirstIndexToRemove);
			AllocationFlags.Remove(FirstIndexToRemove,AllocationFlags.Num() - FirstIndexToRemove);
		}

		// Shrink the data array.
		Data.Shrink();
	}

	/** Compacts the allocated elements into a contiguous index range. */
	void Compact()
	{
		// Copy the existing elements to a new array.
		TSparseArray<ElementType,Allocator> CompactedArray;
		CompactedArray.Empty(Num());
		for(TConstIterator It(*this);It;++It)
		{
			new(CompactedArray.Add()) ElementType(*It);
		}

		// Replace this array with the compacted array.
		Exchange(*this,CompactedArray);
	}

	/** Sorts the elements using the provided comparison class. */
	template<typename CompareClass>
	void Sort()
	{
		if(Num() > 0)
		{
			// Compact the elements array so all the elements are contiguous.
			Compact();

			// Sort the elements according to the provided comparison class.
			Lightmass::Sort<FElementOrFreeListLink,ElementCompareClass<CompareClass> >(&GetData(0),Num());
		}
	}

	/** 
	 * Helper function to return the amount of memory allocated by this container 
	 * @return number of bytes allocated by this container
	 */
	SIZE_T GetAllocatedSize( void ) const
	{
		return	(Data.Num() + Data.GetSlack()) * sizeof(FElementOrFreeListLink) +
				AllocationFlags.GetAllocatedSize();
	}

	/**
	 * Equality comparison operator.
	 * Checks that both arrays have the same elements and element indices; that means that unallocated elements are signifigant!
	 */
	friend UBOOL operator==(const TSparseArray& A,const TSparseArray& B)
	{
		if(A.GetMaxIndex() != B.GetMaxIndex())
		{
			return FALSE;
		}

		for(INT ElementIndex = 0;ElementIndex < A.GetMaxIndex();ElementIndex++)
		{
			const UBOOL bIsAllocatedA = A.IsAllocated(ElementIndex);
			const UBOOL bIsAllocatedB = B.IsAllocated(ElementIndex);
			if(bIsAllocatedA != bIsAllocatedB)
			{
				return FALSE;
			}
			else if(bIsAllocatedA)
			{
				if(A(ElementIndex) != B(ElementIndex))
				{
					return FALSE;
				}
			}
		}

		return TRUE;
	}

	/**
	 * Inequality comparison operator.
	 * Checks that both arrays have the same elements and element indices; that means that unallocated elements are signifigant!
	 */
	friend UBOOL operator!=(const TSparseArray& A,const TSparseArray& B)
	{
		return !(A == B);
	}

	/** Default constructor. */
	TSparseArray()
	:	FirstFreeIndex(0)
	,	NumFreeIndices(0)
	{}

	/** Copy constructor. */
	TSparseArray(const TSparseArray& InCopy)
	:	FirstFreeIndex(0)
	,	NumFreeIndices(0)
	{
		*this = InCopy;
	}

	/** Assignment operator. */
	TSparseArray& operator=(const TSparseArray& InCopy)
	{
		if(this != &InCopy)
		{
			// Reallocate the array.
			Empty(InCopy.GetMaxIndex());
			Data.Add(InCopy.GetMaxIndex());

			// Copy the other array's element allocation state.
			FirstFreeIndex = InCopy.FirstFreeIndex;
			NumFreeIndices = InCopy.NumFreeIndices;
			AllocationFlags = InCopy.AllocationFlags;

			// Determine whether we need per element construction or bulk copy is fine
			if (TContainerTraits<ElementType>::NeedsConstructor)
			{
				// Use the inplace new to copy the element to an array element
				for(INT Index = 0;Index < InCopy.GetMaxIndex();Index++)
				{
					const FElementOrFreeListLink& SourceElement = InCopy.GetData(Index);
					FElementOrFreeListLink& DestElement = GetData(Index);
					if(InCopy.IsAllocated(Index))
					{
						::new((BYTE*)&DestElement.ElementData) ElementType(*(ElementType*)&SourceElement.ElementData);
					}
					else
					{
						DestElement.NextFreeIndex = SourceElement.NextFreeIndex;
					}
				}
			}
			else
			{
				// Use the much faster path for types that allow it
				appMemcpy(Data.GetData(),InCopy.Data.GetData(),sizeof(FElementOrFreeListLink) * InCopy.GetMaxIndex());
			}
		}
		return *this;
	}

	// Accessors.
	ElementType& operator()(INT Index)
	{
		checkSlow(Index >= 0);
		checkSlow(Index < Data.Num());
		checkSlow(Index < AllocationFlags.Num());
		checkSlow(AllocationFlags(Index));
		return *(ElementType*)&GetData(Index).ElementData;
	}
	const ElementType& operator()(INT Index) const
	{
		checkSlow(Index >= 0);
		checkSlow(Index < Data.Num());
		checkSlow(AllocationFlags(Index));
		return *(ElementType*)&GetData(Index).ElementData;
	}
	UBOOL IsAllocated(INT Index) const { return AllocationFlags(Index); }
	INT GetMaxIndex() const { return Data.Num(); }
	INT Num() const { return Data.Num() - NumFreeIndices; }

private:

	/** The base class of sparse array iterators. */
	template<bool bConst>
	class TBaseIterator
	{
	private:

		typedef typename TChooseClass<bConst,const TSparseArray,TSparseArray>::Result ArrayType;
		typedef typename TChooseClass<bConst,const ElementType,ElementType>::Result ItElementType;

		// private class for safe bool conversion
		struct PrivateBooleanHelper { INT Value; };

	public:
		TBaseIterator(ArrayType& InArray,INT StartIndex = 0):
			Array(InArray),
			BitArrayIt(InArray.AllocationFlags,StartIndex)
		{}
		FORCEINLINE TBaseIterator& operator++()
		{
			// Iterate to the next set allocation flag.
			++BitArrayIt;
			return *this;
		}
		FORCEINLINE INT GetIndex() const { return BitArrayIt.GetIndex(); }

		/** conversion to "bool" returning TRUE if the iterator is valid. */
		typedef bool PrivateBooleanType;
		FORCEINLINE operator PrivateBooleanType() const { return BitArrayIt ? &PrivateBooleanHelper::Value : NULL; }
		FORCEINLINE bool operator !() const { return !operator PrivateBooleanType(); }

		FORCEINLINE ItElementType& operator*() const { return Array(GetIndex()); }
		FORCEINLINE ItElementType* operator->() const { return &Array(GetIndex()); }
		FORCEINLINE const FRelativeBitReference& GetRelativeBitReference() const { return BitArrayIt; }

		/** Safely removes the current element from the array. */
		void RemoveCurrent()
		{
			Array.Remove(GetIndex());
		}
	private:
		ArrayType& Array;
		TConstSetBitIterator<typename Allocator::BitArrayAllocator> BitArrayIt;
	};

public:

	/** Iterates over all allocated elements in a sparse array. */
	class TIterator : public TBaseIterator<false>
	{
	public:
		TIterator(TSparseArray& InArray,INT StartIndex = 0):
			TBaseIterator<false>(InArray,StartIndex)
		{}
	};

	/** Iterates over all allocated elements in a const sparse array. */
	class TConstIterator : public TBaseIterator<true>
	{
	public:
		TConstIterator(const TSparseArray& InArray,INT StartIndex = 0):
			TBaseIterator<true>(InArray,StartIndex)
		{}
	};

	/** An iterator which only iterates over the elements of the array which correspond to set bits in a separate bit array. */
	template<typename SubsetAllocator = FDefaultBitArrayAllocator>
	class TConstSubsetIterator
	{
	private:
		// private class for safe bool conversion
		struct PrivateBooleanHelper { INT Value; };

	public:
		TConstSubsetIterator( const TSparseArray& InArray, const TBitArray<SubsetAllocator>& InBitArray ):
			Array(InArray),
			BitArrayIt(InArray.AllocationFlags,InBitArray)
		{}
		FORCEINLINE TConstSubsetIterator& operator++()
		{
			// Iterate to the next element which is both allocated and has its bit set in the other bit array.
			++BitArrayIt;
			return *this;
		}
		FORCEINLINE INT GetIndex() const { return BitArrayIt.GetIndex(); }
		
		/** conversion to "bool" returning TRUE if the iterator is valid. */
		typedef bool PrivateBooleanType;
		FORCEINLINE operator PrivateBooleanType() const { return BitArrayIt ? &PrivateBooleanHelper::Value : NULL; }
		FORCEINLINE bool operator !() const { return !operator PrivateBooleanType(); }

		FORCEINLINE const ElementType& operator*() const { return Array(GetIndex()); }
		FORCEINLINE const ElementType* operator->() const { return &Array(GetIndex()); }
		FORCEINLINE const FRelativeBitReference& GetRelativeBitReference() const { return BitArrayIt; }
	private:
		const TSparseArray& Array;
		TConstDualSetBitIterator<typename Allocator::BitArrayAllocator,SubsetAllocator> BitArrayIt;
	};

	/** Concatenation operators */
	TSparseArray& operator+=( const TSparseArray& OtherArray )
	{
		this->Reserve(OtherArray.Num());
		for ( typename TSparseArray::TConstIterator It(OtherArray); It; ++It )
		{
			this->AddItem(*It);
		}
		return *this;
	}
	TSparseArray& operator+=( const TArray<ElementType>& OtherArray )
	{
		this->Reserve(OtherArray.Num());
		for ( INT Idx = 0; Idx < OtherArray.Num(); Idx++ )
		{
			this->AddItem(OtherArray(Idx));
		}
		return *this;
	}

private:

	/**
	 * The element type stored is only indirectly related to the element type requested, to avoid instantiating TArray redundantly for
	 * compatible types.
	 */
	typedef TSparseArrayElementOrFreeListLink<
		TAlignedBytes<sizeof(ElementType),TElementAlignmentCalculator<ElementType>::Alignment>
		> FElementOrFreeListLink;

	/** Extracts the element value from the array's element structure and passes it to the user provided comparison class. */
	template<typename CompareClass>
	class ElementCompareClass
	{
	public:
		static INT Compare(const FElementOrFreeListLink& A,const FElementOrFreeListLink& B)
		{
			return CompareClass::Compare(*(ElementType*)&A.ElementData,*(ElementType*)&B.ElementData);
		}
	};

	TArray<FElementOrFreeListLink,typename Allocator::ElementAllocator> Data;
	AllocationBitArrayType AllocationFlags;

	/** The index of an unallocated element in the array that currently contains the head of the linked list of free elements. */
	INT FirstFreeIndex;

	/** The number of elements in the free list. */
	INT NumFreeIndices;

	/** Accessor for the element or free list data. */
	FElementOrFreeListLink& GetData(INT Index)
	{
		return ((FElementOrFreeListLink*)Data.GetData())[Index];
	}

	/** Accessor for the element or free list data. */
	const FElementOrFreeListLink& GetData(INT Index) const
	{
		return ((FElementOrFreeListLink*)Data.GetData())[Index];
	}
};


/** A specialization of the exchange macro that avoids reallocating when exchanging two arrays. */
template <typename ElementType,typename Allocator>
inline void Exchange(
	TSparseArray<ElementType,Allocator>& A,
	TSparseArray<ElementType,Allocator>& B
	)
{
	appMemswap( &A, &B, sizeof(A) );
}





/*-----------------------------------------------------------------------------
	Indirect array.
	Same as a TArray above, but stores pointers to the elements, to allow
	resizing the array index without relocating the actual elements.
-----------------------------------------------------------------------------*/

template<typename T,typename Allocator = FDefaultAllocator>
class TIndirectArray : public TArray<void*,Allocator>
{
public:
	typedef TArray<void*,Allocator> Super;

	typedef T ElementType;
	TIndirectArray()
	:	Super()
	{}
	TIndirectArray( INT InNum )
	:	Super( InNum )
	{}
	TIndirectArray( const TIndirectArray& Other )
	:	Super( Other.ArrayNum )
	{
		this->ArrayNum=0;
		for( INT i=0; i<Other.ArrayNum; i++ )
		{
			new(*this)T(Other(i));
		}
	}
	TIndirectArray( ENoInit )
	:	Super( E_NoInit )
	{}
	~TIndirectArray()
	{
		checkSlow(this->ArrayNum>=0);
		checkSlow(this->ArrayMax>=this->ArrayNum);
		Remove( 0, this->ArrayNum );
	}
	/**
	 * Helper function for returning a typed pointer to the first array entry.
	 *
	 * @return pointer to first array entry or NULL if this->ArrayMax==0
	 */
	T** GetTypedData()
	{
		return (T**)this->AllocatorInstance.GetAllocation();
	}
	/**
	 * Helper function for returning a typed pointer to the first array entry.
	 *
	 * @return pointer to first array entry or NULL if this->ArrayMax==0
	 */
	const T** GetTypedData() const
	{
		return (T**)this->AllocatorInstance.GetAllocation();
	}
	/** 
	 * Helper function returning the size of the inner type
	 *
	 * @return size in bytes of array type
	 */
	DWORD GetTypeSize() const
	{
		return sizeof(T*);
	}
	/**
	 * Copies the source array into this one.
	 *
	 * @param Other the source array to copy
	 */
	TIndirectArray& operator=( const TIndirectArray& Other )
	{
		Empty( Other.Num() );
		for( INT i=0; i<Other.Num(); i++ )
		{
			new(*this)T(Other(i));
		}	
		return *this;
	}
    T& operator()( INT i )
	{
		checkSlow(i>=0 && i<this->ArrayNum);
		checkSlow(this->ArrayMax>=this->ArrayNum);
		return *((T**)this->AllocatorInstance.GetAllocation())[i];
	}
	const T& operator()( INT i ) const
	{
		checkSlow(i>=0 && i<this->ArrayNum);
		checkSlow(this->ArrayMax>=this->ArrayNum);
		return *((T**)this->AllocatorInstance.GetAllocation())[i];
	}
	void Shrink()
	{
		Super::Shrink( );
	}
	UBOOL FindItem( const T*& Item, INT& Index ) const
	{
		for( Index=0; Index<this->ArrayNum; Index++ )
		{
			if( (*this)(Index)==*Item )
			{
				return TRUE;
			}
		}
		return FALSE;
	}
	INT FindItemIndex( const T*& Item ) const
	{
		for( INT Index=0; Index<this->ArrayNum; Index++ )
		{
			if( (*this)(Index)==*Item )
			{
				return Index;
			}
		}
		return INDEX_NONE;
	}

	INT Add( INT Count=1 )
	{
		// use AddZeroed for indirect arrays so that the destructor/Remove doesn't try to free a garbage pointer
		return Super::AddZeroed(Count);
	}

	void Remove( INT Index, INT Count=1 )
	{
		check(Index>=0);
		check(Index<=this->ArrayNum);
		check(Index+Count<=this->ArrayNum);
		for( INT i=Index; i<Index+Count; i++ )
		{
			delete ((T**)this->AllocatorInstance.GetAllocation())[i];
		}
		Super::Remove( Index, Count );
	}
	void Empty( INT Slack=0 )
	{
		for( INT i=0; i<this->ArrayNum; i++ )
		{
			delete ((T**)this->AllocatorInstance.GetAllocation())[i];
		}
		Super::Empty( Slack );
	}
	INT AddRawItem(T* Item)
	{
		const INT	Index = Super::Add(1);
		((T**)this->AllocatorInstance.GetAllocation())[Index] = Item;
		return Index;
	}
	/** 
	* Helper function to return the amount of memory allocated by this container 
	*
	* @return number of bytes allocated by this container
	*/
	SIZE_T GetAllocatedSize( void ) const
	{
		return( this->ArrayMax * (sizeof(T) + sizeof(T*)) );
	}
};


/** An array that uses multiple allocations to avoid allocation failure due to fragmentation. */
template<typename ElementType>
class TChunkedArray
{
public:

	TChunkedArray() :
		NumElements(0)
	{}

	/** Initialization constructor. */
	TChunkedArray(INT InNumElements):
		NumElements(InNumElements)
	{
		// Compute the number of chunks needed.
		const INT NumChunks = (NumElements + NumElementsPerChunk - 1) / NumElementsPerChunk;

		// Allocate the chunks.
		Chunks.Empty(NumChunks);
		for(INT ChunkIndex = 0;ChunkIndex < NumChunks;ChunkIndex++)
		{
			new(Chunks) FChunk;
		}
	}

	// Accessors.
	ElementType& operator()(INT ElementIndex)
	{
		const UINT ChunkIndex = ElementIndex / NumElementsPerChunk;
		const UINT ChunkElementIndex = ElementIndex % NumElementsPerChunk;
		return Chunks(ChunkIndex).Elements[ChunkElementIndex];
	}
	const ElementType& operator()(INT ElementIndex) const
	{
		const INT ChunkIndex = ElementIndex / NumElementsPerChunk;
		const INT ChunkElementIndex = ElementIndex % NumElementsPerChunk;
		return Chunks(ChunkIndex).Elements[ChunkElementIndex];
	}

	SIZE_T GetAllocatedSize( void ) const
	{
		return Chunks.GetAllocatedSize();
	}

	INT Add( INT Count=1 )
	{
		check(Count>=0);
		checkSlow(NumElements>=0);

		const INT OldNum = NumElements;
		for (INT i = 0; i < Count; i++)
		{
			if (NumElements % NumElementsPerChunk == 0)
			{
				new(Chunks) FChunk;
			}
			NumElements++;
		}
		return OldNum;
	}

	void Empty( INT Slack=0 ) 
	{
		Chunks.Empty(Slack % NumElementsPerChunk + 1);
		NumElements = 0;
	}

	void Shrink()
	{
		Chunks.Shrink();
	}

	INT Num() const { return NumElements; }

private:

	enum { TargetBytesPerChunk = 1024 * 1024 };
	enum { NumElementsPerChunk = TargetBytesPerChunk / sizeof(ElementType) };

	/** A chunk of the array's elements. */
	struct FChunk
	{
		/** The elements in the chunk. */
		ElementType Elements[NumElementsPerChunk];
	};

	/** The chunks of the array's elements. */
	TIndirectArray<FChunk> Chunks;

	/** The number of elements in the array. */
	INT NumElements;
};



} // namespace



/**
 * A placement new operator which constructs an element in a sparse array allocation.
 * Must be outside the namespace
 */
inline void* operator new(size_t Size,const Lightmass::FSparseArrayAllocationInfo& Allocation)
{
	return Allocation.Pointer;
}

/**
 * A placement new operator which constructs an element in an indirect array
 * Must be outside the namespace
 */
template<typename T> void* operator new( size_t Size, Lightmass::TIndirectArray<T>& Array )
{
	check(Size == sizeof(T));
	const INT Index = Array.AddRawItem((T*)Lightmass::appMalloc(Size));
	return &Array(Index);
}

/**
 * A placement new operator which constructs an element in an TArray
 * Must be outside the namespace
 */
template <typename T,typename Allocator> void* operator new( size_t Size, Lightmass::TArray<T,Allocator>& Array )
{
	check(Size == sizeof(T));
	const Lightmass::INT Index = Array.Add(1);
	return &Array(Index);
}

/**
 * A placement new operator which constructs an element in an TArray at an index
 * Must be outside the namespace
 */
template <typename T,typename Allocator> void* operator new( size_t Size, Lightmass::TArray<T,Allocator>& Array, Lightmass::INT Index )
{
	check(Size == sizeof(T));
	Array.Insert(Index,1);
	return &Array(Index);
}

