/*=============================================================================
	LMMap.h: Dynamic map (TMap) definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

#include "LMArray.h"
#include "LMSet.h"

namespace Lightmass
{

// Forward declarations.
template<typename ElementType,typename SetAllocator = FDefaultSetAllocator >
class TLookupMap;


#define ExchangeB(A,B) {UBOOL T=A; A=B; B=T;}

/**
 * A hashing function that works well for pointers.
 */
template<class T>
inline DWORD PointerHash(const T* Key,DWORD C = 0)
{
#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

	DWORD A;
	DWORD B;
	A = B = 0x9e3779b9;
	// Avoid LHS stalls on PS3 and Xbox 360
	A += reinterpret_cast<DWORD>(Key);
	mix(A,B,C);
	return C;

#undef mix
}

/** 
 * The base class of maps from keys to values.  Implemented using a TSet of key-value pairs with a custom KeyFuncs, 
 * with the same O(1) addition, removal, and finding. 
 */
template<typename KeyType,typename ValueType,UBOOL bInAllowDuplicateKeys,typename SetAllocator = FDefaultSetAllocator >
class TMapBase
{
public:

	typedef typename TContainerTraits<KeyType>::ConstPointerType KeyConstPointerType;
	typedef typename TContainerTraits<KeyType>::ConstInitType KeyInitType;
	typedef typename TContainerTraits<ValueType>::ConstInitType ValueInitType;

	/** Default constructor. */
	TMapBase() {}

	/** Copy constructor. */
	TMapBase(const TMapBase& Other)
	:	Pairs( Other.Pairs )
	{}

	/** Assignment operator. */
	TMapBase& operator=(const TMapBase& Other)
	{
		Pairs = Other.Pairs;
		return *this;
	}

	// Legacy comparison operators.  Note that these also test whether the map's key-value pairs were added in the same order!
	friend UBOOL LegacyCompareEqual(const TMapBase& A,const TMapBase& B)
	{
		return LegacyCompareEqual(A.Pairs,B.Pairs);
	}
	friend UBOOL LegacyCompareNotEqual(const TMapBase& A,const TMapBase& B)
	{
		return LegacyCompareNotEqual(A.Pairs,B.Pairs);
	}

	/**
	 * Removes all elements from the map, potentially leaving space allocated for an expected number of elements about to be added.
	 * @param ExpectedNumElements - The number of elements about to be added to the set.
	 */
	void Empty(INT ExpectedNumElements = 0)
	{
		Pairs.Empty(ExpectedNumElements);
	}

    /** Efficiently empties out the map but preserves all allocations and capacities */
    void Reset()
    {
        Empty(Num());
    }

	/** Shrinks the pair set to avoid slack. */
	void Shrink()
	{
		Pairs.Shrink();
	}

	/** @return The number of elements in the map. */
	INT Num() const
	{
		return Pairs.Num();
	}

#if LM_MAP // this takes a lot of supporting code, prolly not needed
	/**
	 * Returns the unique keys contained within this map
	 * @param	OutKeys	- Upon return, contains the set of unique keys in this map.
	 * @return The number of unique keys in the map.
	 */
	INT GetKeys(TLookupMap<KeyType>& OutKeys) const;
#endif

	/** 
	 * Helper function to return the amount of memory allocated by this container 
	 * @return number of bytes allocated by this container
	 */
	SIZE_T GetAllocatedSize( void ) const
	{
		return Pairs.GetAllocatedSize();
	}

	/**
	 * Sets the value associated with a key.
	 * If the key is already associated with any values, the existing values are replaced by the new value.
	 * @param InKey - The key to associate the value with.
	 * @param InValue - The value to associate with the key.
	 * @return A reference to the value as stored in the map.  The reference is only valid until the next change to any key in the map.
	 */
	ValueType& Set(KeyInitType InKey,ValueInitType InValue)
	{
		// Remove existing values associated with the specified key.
		// This is only necessary if the TSet allows duplicate keys; otherwise TSet::Add replaces the existing key-value pair.
		if(KeyFuncs::bAllowDuplicateKeys)
		{
			for(typename PairSetType::TKeyIterator It(Pairs,InKey);It;++It)
			{
				It.RemoveCurrent();
			}
		}

		// Add the key-value pair to the set.  TSet::Add will replace any existing key-value pair that has the same key.
		const FSetElementId PairId = Pairs.Add(FPairInitializer(InKey,InValue));

		return Pairs(PairId).Value;
	}

	/**
	 * Removes all value associations for a key.
	 * @param InKey - The key to remove associated values for.
	 * @return The number of values that were associated with the key.
	 */
	INT Remove(KeyConstPointerType InKey)
	{
		const INT NumRemovedPairs = Pairs.RemoveKey(InKey);
		return NumRemovedPairs;
	}
	INT RemoveKey(KeyConstPointerType InKey)
	{
		return Remove(InKey);
	}

	/**
	 * Returns the key associated with the specified value.  The time taken is O(N) in the number of pairs.
	 * @param	Value - The value to search for
	 * @return	A pointer to the key associated with the specified value, or NULL if the value isn't contained in this map.  The pointer
	 *			is only valid until the next change to any key in the map.
	 */
	const KeyType* FindKey(ValueInitType Value) const
	{
		for(typename PairSetType::TConstIterator PairIt(Pairs);PairIt;++PairIt)
		{
			if(PairIt->Value == Value)
			{
				return &PairIt->Key;
			}
		}
		return NULL;
	}

	/**
	 * Returns the value associated with a specified key.
	 * @param	Key - The key to search for.
	 * @return	A pointer to the value associated with the specified key, or NULL if the key isn't contained in this map.  The pointer
	 *			is only valid until the next change to any key in the map.
	 */
	ValueType* Find(KeyConstPointerType Key)
	{
		FPair* Pair = Pairs.Find(Key);
		return Pair ? &Pair->Value : NULL;
	}
	const ValueType* Find(KeyConstPointerType Key) const
	{
		const FPair* Pair = Pairs.Find(Key);
		return Pair ? &Pair->Value : NULL;
	}

	/**
	 * Returns the value associated with a specified key.
	 * @param	Key - The key to search for.
	 * @return	The value associated with the specified key, or the default value for the ValueType if the key isn't contained in this map.
	 */
	ValueType FindRef(KeyConstPointerType Key) const
	{
		const FPair* Pair = Pairs.Find(Key);
		return Pair ? Pair->Value : (ValueType)0;
	}

	/**
	 * Checks if map contains the specified key.
	 * @return Key - The key to check for.
	 * @return TRUE if the map contains the key.
	 */
	UBOOL HasKey(KeyConstPointerType Key) const
	{
		return Pairs.Contains(Key);
	}

	/**
	 * Generates an array from the keys in this map.
	 */
	void GenerateKeyArray(TArray<KeyType>& OutArray) const
	{
		OutArray.Empty(Pairs.Num());
		for(typename PairSetType::TConstIterator PairIt(Pairs);PairIt;++PairIt)
		{
			new(OutArray) KeyType(PairIt->Key);
		}
	}

	/**
	 * Generates an array from the values in this map.
	 */
	void GenerateValueArray(TArray<ValueType>& OutArray) const
	{
		OutArray.Empty(Pairs.Num());
		for(typename PairSetType::TConstIterator PairIt(Pairs);PairIt;++PairIt)
		{
			new(OutArray) ValueType(PairIt->Value);
		}
	}

protected:

	/** An initializer type for pairs that's passed to the pair set when adding a new pair. */
	class FPairInitializer
	{
	public:

		KeyInitType Key;
		ValueInitType Value;

		/** Initialization constructor. */
		FPairInitializer(KeyInitType InKey,ValueInitType InValue):
			Key(InKey),
			Value(InValue)
		{}
	};

	/** A key-value pair in the map. */
	class FPair
	{
	public:

		KeyType Key;
		ValueType Value;

		/** Default constructor. */
		FPair()
		{}

		/** Initialization constructor. */
		FPair(const FPairInitializer& InInitializer)
		:	Key(InInitializer.Key)
		,	Value(InInitializer.Value)
		{}

		/** Copy constructor. */
		FPair(const FPair& InCopy)
		:	Key(InCopy.Key)
		,	Value(InCopy.Value)
		{}

		// Comparison operators
		inline UBOOL operator==(const FPair& Other) const
		{
			return Key == Other.Key && Value == Other.Value;
		}
		inline UBOOL operator!=(const FPair& Other) const
		{
			return Key != Other.Key || Value != Other.Value;
		}

		/** Implicit conversion to pair initializer. */
		operator FPairInitializer() const
		{
			return FPairInitializer(Key,Value);
		}
	};

	/** Defines how the map's pairs are hashed. */
	struct KeyFuncs : BaseKeyFuncs<FPair,KeyType,bInAllowDuplicateKeys>
	{
		typedef typename TContainerTraits<KeyType>::ConstPointerType	KeyInitType;
		typedef const FPairInitializer&									ElementInitType;

		static KeyInitType GetSetKey(const FPairInitializer& Element)
		{
			return Element.Key;
		}
		static UBOOL Matches(KeyInitType A,KeyInitType B)
		{
			return A == B;
		}
		static DWORD GetKeyHash(KeyInitType Key)
		{
			return GetTypeHash(Key);
		}
	};

	typedef TSet<FPair,KeyFuncs,SetAllocator> PairSetType;

	/** The base of TMapBase iterators. */
	template<bool bConst>
	class TBaseIterator
	{
	private:

		typedef typename TChooseClass<bConst,typename PairSetType::TConstIterator,typename PairSetType::TIterator>::Result PairItType;
		typedef typename TChooseClass<bConst,const TMapBase,TMapBase>::Result MapType;
		typedef typename TChooseClass<bConst,const KeyType,KeyType>::Result ItKeyType;
		typedef typename TChooseClass<bConst,const ValueType,ValueType>::Result ItValueType;

		// private class for safe bool conversion
		struct PrivateBooleanHelper { INT Value; };

	public:

		/** Initialization constructor. */
		TBaseIterator(MapType& InMap,INT StartIndex = 0)
		:	PairIt(InMap.Pairs,StartIndex)
		{}

		TBaseIterator& operator++()			{ ++PairIt; return *this; }
		/** conversion to "bool" returning TRUE if the iterator is valid. */
		typedef bool PrivateBooleanType;
		operator PrivateBooleanType() const { return PairIt ? &PrivateBooleanHelper::Value : NULL; }
		bool operator !() const { return !operator PrivateBooleanType(); }
		ItKeyType& Key() const		{ return PairIt->Key; }
		ItValueType& Value() const	{ return PairIt->Value; }

	protected:
		PairItType PairIt;
	};

	/** The base type of iterators that iterate over the values associated with a specified key. */
	template<bool bConst>
	class TBaseKeyIterator
	{
	private:

		typedef typename TChooseClass<bConst,typename PairSetType::TConstKeyIterator,typename PairSetType::TKeyIterator>::Result SetItType;
		typedef typename TChooseClass<bConst,const KeyType,KeyType>::Result ItKeyType;
		typedef typename TChooseClass<bConst,const ValueType,ValueType>::Result ItValueType;

		// private class for safe bool conversion
		struct PrivateBooleanHelper { INT Value; };

	public:
		/** Initialization constructor. */
		TBaseKeyIterator(const SetItType& InSetIt)
		:	SetIt(InSetIt)
		{}

		TBaseKeyIterator& operator++()			{ ++SetIt; return *this; }
		/** conversion to "bool" returning TRUE if the iterator is valid. */
		typedef bool PrivateBooleanType;
		operator PrivateBooleanType() const { return SetIt ? &PrivateBooleanHelper::Value : NULL; }
		bool operator !() const { return !operator PrivateBooleanType(); }
		ItKeyType& Key() const		{ return SetIt->Key; }
		ItValueType& Value() const	{ return SetIt->Value; }

	protected:
		SetItType SetIt;
	};

	/** A set of the key-value pairs in the map. */
	PairSetType Pairs;

public:

	/** Map iterator. */
	class TIterator : public TBaseIterator<false>
	{
	public:

		/** Initialization constructor. */
		TIterator(TMapBase& InMap,UBOOL bInRequiresRehashOnRemoval = FALSE,INT StartIndex = 0) 
		:	TBaseIterator<false>(InMap,StartIndex)
		,	Map(InMap)
		,	bElementsHaveBeenRemoved(FALSE)
		,	bRequiresRehashOnRemoval(bInRequiresRehashOnRemoval)
		{}

		/** Destructor. */
		~TIterator()
		{
			if(bElementsHaveBeenRemoved && bRequiresRehashOnRemoval)
			{
				Map.Pairs.Relax();
			}
		}

		/** Removes the current pair from the map. */
		void RemoveCurrent()
		{
			TBaseIterator<false>::PairIt.RemoveCurrent();
			bElementsHaveBeenRemoved = TRUE;
		}

	private:
		TMapBase& Map;
		UBOOL bElementsHaveBeenRemoved;
		UBOOL bRequiresRehashOnRemoval;
	};

	/** Const map iterator. */
	class TConstIterator : public TBaseIterator<true>
	{
	public:
		TConstIterator(const TMapBase& InMap,INT StartIndex = 0)
		:	TBaseIterator<true>(InMap,StartIndex)
		{}
	};

	/** Iterates over values associated with a specified key in a const map. */
	class TConstKeyIterator : public TBaseKeyIterator<true>
	{
	public:
		TConstKeyIterator(const TMapBase& InMap,KeyInitType InKey)
		:	TBaseKeyIterator<true>(typename PairSetType::TConstKeyIterator(InMap.Pairs,InKey))
		{}
	};

	/** Iterates over values associated with a specified key in a map. */
	class TKeyIterator : public TBaseKeyIterator<false>
	{
	public:
		TKeyIterator(TMapBase& InMap,KeyInitType InKey)
		:	TBaseKeyIterator<false>(typename PairSetType::TKeyIterator(InMap.Pairs,InKey))
		{}

		/** Removes the current key-value pair from the map. */
		void RemoveCurrent()
		{
			TBaseKeyIterator<false>::SetIt.RemoveCurrent();
		}
	};
};

/** The base type of sortable maps. */
template<typename KeyType,typename ValueType,UBOOL bInAllowDuplicateKeys,typename SetAllocator = FDefaultSetAllocator >
class TSortableMapBase : public TMapBase<KeyType,ValueType,bInAllowDuplicateKeys,SetAllocator>
{
public:
	typedef TMapBase<KeyType,ValueType,bInAllowDuplicateKeys,SetAllocator> Super;

	/** Default constructor. */
	TSortableMapBase() {}

	/** Copy constructor. */
	TSortableMapBase(const TSortableMapBase& InCopy)
	:	TMapBase<KeyType,ValueType,bInAllowDuplicateKeys>(InCopy)
	{}

	/** Assignment operator. */
	TSortableMapBase& operator=( const TSortableMapBase& Other )
	{
		Super::operator=( Other );
		return *this;
	}

	/**
	 * Sorts the pairs array using each pair's Key as the sort criteria, then rebuilds the map's hash.
	 * Invoked using "MyMapVar.KeySort<COMPARE_CONSTREF_CLASS(KeyType,Filename)>();"
	 */
	template<typename CompareClass>
	void KeySort()
	{
		Super::Pairs.Sort<KeyComparisonClass<CompareClass> >();
	}

	/**
	 * Sorts the pairs array using each pair's Value as the sort criteria, then rebuilds the map's hash.
	 * Invoked using "MyMapVar.ValueSort<COMPARE_CONSTREF_CLASS(ValueType,Filename)>();"
	 */
	template<typename CompareClass>
	void ValueSort()
	{
		Super::Pairs.Sort<ValueComparisonClass<CompareClass> >();
	}

private:

	/** Extracts the pair's key from the map's pair structure and passes it to the user provided comparison class. */
	template<typename CompareClass>
	class KeyComparisonClass
	{
	public:
		static INT Compare(
			const typename Super::FPair& A,
			const typename Super::FPair& B
			)
		{
			return CompareClass::Compare(A.Key,B.Key);
		}
	};

	/** Extracts the pair's value from the map's pair structure and passes it to the user provided comparison class. */
	template<typename CompareClass>
	class ValueComparisonClass
	{
	public:
		static INT Compare(
			const typename Super::FPair& A,
			const typename Super::FPair& B
			)
		{
			return CompareClass::Compare(A.Value, B.Value);
		}
	};
};

/** A TMapBase specialization that only allows a single value associated with each key.*/
template<typename KeyType,typename ValueType,typename SetAllocator /*= FDefaultSetAllocator*/>
class TMap : public TSortableMapBase<KeyType,ValueType,FALSE,SetAllocator>
{
public:

	typedef TSortableMapBase<KeyType,ValueType,FALSE,SetAllocator> Super;
	typedef typename Super::KeyInitType KeyInitType;

	/** Default constructor. */
	TMap() {}

	/** Copy constructor. */
	TMap(const TMap& InCopy)
	:	Super(InCopy)
	{}

	/** Assignment operator. */
	TMap& operator=( const TMap& Other )
	{
		Super::operator=( Other );
		return *this;
	}

	/**
	 * Removes the pair with the specified key and copies the value that was removed to the ref parameter
	 * @param Key - the key to search for
	 * @param OutRemovedValue - if found, the value that was removed (not modified if the key was not found)
	 * @return whether or not the key was found
	 */
	UBOOL RemoveAndCopyValue(KeyInitType Key,ValueType& OutRemovedValue)
	{
		const FSetElementId PairId = Super::Pairs.FindId(Key);
		if(PairId.IsValidId())
		{
			OutRemovedValue = Super::Pairs(PairId).Value;
			Super::Pairs.Remove(PairId);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
};

/** A TMapBase specialization that allows multiple values to be associated with each key. */
template<typename KeyType,typename ValueType,typename SetAllocator /* = FDefaultSetAllocator */>
class TMultiMap : public TSortableMapBase<KeyType,ValueType,TRUE,SetAllocator>
{
public:

	typedef TSortableMapBase<KeyType,ValueType,TRUE,SetAllocator> Super;
	typedef typename Super::KeyInitType KeyInitType;
	typedef typename Super::ValueInitType ValueInitType;

	/** Default constructor. */
	TMultiMap() {}

	/** Copy constructor. */
	TMultiMap(const TMultiMap& InCopy)
	:	Super(InCopy)
	{}

	/** Assignment operator. */
	TMultiMap& operator=(const TMultiMap& Other)
	{
		Super::operator=( Other );
		return *this;
	}

	/**
	 * Finds all values associated with the specified key.
	 * @param Key - The key to find associated values for.
	 * @param OutValues - Upon return, contains the values associated with the key.
	 * @param bMaintainOrder - TRUE if the Values array should be in the same order as the map's pairs.
	 */
	void MultiFind(KeyInitType Key,TArray<ValueType>& OutValues,UBOOL bMaintainOrder = FALSE) const
	{
		for(typename Super::PairSetType::TConstKeyIterator It(Super::Pairs,Key);It;++It)
		{
			new(OutValues) ValueType(It->Value);
		}

		if(bMaintainOrder)
		{
			// Create an array with the values in reverse enumerated order; i.e. the order they were inserted in the map.
			TArray<ValueType> OrderedValues;
			OrderedValues.Empty(OutValues.Num());
			for(INT Index = OutValues.Num() - 1;Index >= 0;Index--)
			{
				new(OrderedValues) ValueType(OutValues(Index));
			}

			// Swap the ordered array into the output array.
			Exchange(OrderedValues,OutValues);
		}
	}

	/**
	 * Adds a key-value association to the map.  The association doesn't replace any of the key's existing associations.
	 * @param InKey - The key to associate.
	 * @param InValue - The value to associate.
	 * @return A reference to the value as stored in the map; the reference is only valid until the next change to any key in the map.
	 */
	ValueType& Add(KeyInitType InKey,ValueInitType InValue)
	{
		const FSetElementId PairId = Super::Pairs.Add(typename Super::FPairInitializer(InKey,InValue));
		return Super::Pairs(PairId).Value;
	}

	/**
	 * Adds a key-value association to the map.  The association doesn't replace any of the key's existing associations.
	 * However, if both the key and value match an existing association in the map, no new association is made and the existing association's
	 * value is returned.
	 * @param InKey - The key to associate.
	 * @param InValue - The value to associate.
	 * @return A reference to the value as stored in the map; the reference is only valid until the next change to any key in the map.
	 */
	ValueType& AddUnique(KeyInitType InKey,ValueInitType InValue)
	{
		// Check for an existing association between the same key and value.
		for(typename Super::PairSetType::TKeyIterator It(Super::Pairs,InKey);It;++It)
		{
			if(It->Value == InValue)
			{
				return It->Value;
			}
		}

		// If there's no existing association with the same key and value, create one.
		return Add(InKey,InValue);
	}

	/**
	 * Removes associations between the specified key and value from the map.
	 * @param InKey - The key part of the pair to remove.
	 * @param InValue - The value part of the pair to remove.
	 * @return The number of associations removed.
	 */
	INT RemovePair(KeyInitType InKey,ValueInitType InValue)
	{
		// Iterate over pairs with a matching key.
		INT NumRemovedPairs = 0;
		for(typename Super::PairSetType::TKeyIterator It(Super::Pairs,InKey);It;++It)
		{
			// If this pair has a matching value as well, remove it.
			if(It->Value == InValue)
			{
				It.RemoveCurrent();
				++NumRemovedPairs;
			}
		}
		return NumRemovedPairs;
	}

	/**
	 * Removes the first association between the specified key and value from the map.
	 * @param InKey - The key part of the pair to remove.
	 * @param InValue - The value part of the pair to remove.
	 * @return The number of associations removed.
	 */
	INT RemoveSinglePair(KeyInitType InKey,ValueInitType InValue)
	{
		// Iterate over pairs with a matching key.
		INT NumRemovedPairs = 0;
		for(typename Super::PairSetType::TKeyIterator It(Super::Pairs,InKey);It;++It)
		{
			// If this pair has a matching value as well, remove it.
			if(It->Value == InValue)
			{
				It.RemoveCurrent();
				++NumRemovedPairs;

				// We were asked to remove only the first association, so bail out.
				break;
			}
		}
		return NumRemovedPairs;
	}

	/**
	 * Finds an association between a specified key and value. (const)
	 * @param Key - The key to find.
	 * @param Value - The value to find.
	 * @return If the map contains a matching association, a pointer to the value in the map is returned.  Otherwise NULL is returned.
	 *			The pointer is only valid as long as the map isn't changed.
	 */
	const ValueType* FindPair(KeyInitType Key,ValueInitType Value) const
	{
		// Iterate over pairs with a matching key.
		for(typename Super::PairSetType::TConstKeyIterator It(Super::Pairs,Key);It;++It)
		{
			// If the pair's value matches, return a pointer to it.
			if(It->Value == Value)
			{
				return &It->Value;
			}
		}

		return NULL;
	}

	/**
	 * Finds an association between a specified key and value.
	 * @param Key - The key to find.
	 * @param Value - The value to find.
	 * @return If the map contains a matching association, a pointer to the value in the map is returned.  Otherwise NULL is returned.
	 *			The pointer is only valid as long as the map isn't changed.
	 */
	ValueType* FindPair(KeyInitType Key,ValueInitType Value)
	{
		// Iterate over pairs with a matching key.
		for(typename Super::PairSetType::TKeyIterator It(Super::Pairs,Key);It;++It)
		{
			// If the pair's value matches, return a pointer to it.
			if(It->Value == Value)
			{
				return &It->Value;
			}
		}

		return NULL;
	}

	/** Returns the number of values within this map associated with the specified key */
	INT Num(KeyInitType Key) const
	{
		// Iterate over pairs with a matching key.
		INT NumMatchingPairs = 0;
		for(typename Super::PairSetType::TConstKeyIterator It(Super::Pairs,Key);It;++It)
		{
			++NumMatchingPairs;
		}
		return NumMatchingPairs;
	}

	// Since we implement an overloaded Num() function in TMultiMap, we need to reimplement TMapBase::Num to make it visible.
	INT Num() const
	{
		return Super::Num();
	}
};

#if LM_MAP
// This function must be defined outside of the TMultiMap declaration because it uses TLookupMap, which uses TMultiMap.
template<typename KeyType,typename ValueType,UBOOL bInAllowDuplicateKeys,typename SetAllocator>
INT TMapBase<KeyType,ValueType,bInAllowDuplicateKeys,SetAllocator>::GetKeys(TLookupMap<KeyType>& OutKeys) const
{
	for(typename PairSetType::TConstIterator It(Pairs);It;++It)
	{
		OutKeys.AddItemEx(It->Key);
	}
	return OutKeys.Num();
}
#endif

}