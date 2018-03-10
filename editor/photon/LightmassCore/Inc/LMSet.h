/*=============================================================================
	LMSet.h: Set definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

#include "LMArray.h"

namespace Lightmass
{


/**
 * The base KeyFuncs type with some useful definitions for all KeyFuncs; meant to be derived from instead of used directly.
 */
template<typename ElementType,typename InKeyType,UBOOL bInAllowDuplicateKeys = FALSE>
struct BaseKeyFuncs
{
	typedef InKeyType KeyType;
	typedef typename TCallTraits<InKeyType>::ParamType KeyInitType;
	typedef typename TCallTraits<ElementType>::ParamType ElementInitType;

	enum { bAllowDuplicateKeys = bInAllowDuplicateKeys };
};

/**
 * A default implementation of the KeyFuncs used by TSet which uses the element as a key.
 */
template<typename ElementType,UBOOL bInAllowDuplicateKeys = FALSE>
struct DefaultKeyFuncs : BaseKeyFuncs<ElementType,ElementType,bInAllowDuplicateKeys>
{
	typedef typename TCallTraits<ElementType>::ParamType KeyInitType;
	typedef typename TCallTraits<ElementType>::ParamType ElementInitType;

	/**
	 * @return The key used to index the given element.
	 */
	static KeyInitType GetSetKey(ElementInitType Element)
	{
		return Element;
	}

	/**
	 * @return True if the keys match.
	 */
	static UBOOL Matches(KeyInitType A,KeyInitType B)
	{
		return A == B;
	}

	/** Calculates a hash index for a key. */
	static DWORD GetKeyHash(KeyInitType Key)
	{
		return GetTypeHash(Key);
	}
};

// Forward declaration.
template<
	typename ElementType,
	typename KeyFuncs = DefaultKeyFuncs<ElementType>,
	typename Allocator = FDefaultSetAllocator
	>
class TSet;

/** Either NULL or an identifier for an element of a set. */
class FSetElementId
{
public:

	template<typename,typename,typename>
	friend class TSet;

	/** Default constructor. */
	FSetElementId():
		Index(INDEX_NONE)
	{}

	/** @return a boolean value representing whether the id is NULL. */
	UBOOL IsValidId() const
	{
		return Index != INDEX_NONE;
	}

	/** Comparison operator. */
	friend UBOOL operator==(const FSetElementId& A,const FSetElementId& B)
	{
		return A.Index == B.Index;
	}

private:

	/** The index of the element in the set's element array. */
	INT Index;

	/** Initialization constructor. */
	FSetElementId(INT InIndex):
		Index(InIndex)
	{}

	/** Implicit conversion to the element index. */
	operator INT() const
	{
		return Index;
	}
};

/**
 * A set with an optional KeyFuncs parameters for customizing how the elements are compared and searched.  
 * E.g. You can specify a mapping from elements to keys if you want to find elements by specifying a subset of 
 * the element type.  It uses a TSparseArray of the elements, and also links the elements into a hash with a 
 * number of buckets proportional to the number of elements.  Addition, removal, and finding are O(1).
 *
 */
template<
	typename ElementType,
	typename KeyFuncs /*= DefaultKeyFuncs<ElementType>*/,
	typename Allocator /*= FDefaultSetAllocator*/
	>
class TSet
{
	typedef typename KeyFuncs::KeyInitType KeyInitType;
	typedef typename KeyFuncs::ElementInitType ElementInitType;

public:

	/** Initialization constructor. */
	TSet()
	:	HashSize(0)
	{}

	/** Copy constructor. */
	TSet(const TSet& Copy)
	:	HashSize(0)
	{
		*this = Copy;
	}

	/** Destructor. */
	~TSet()
	{
		HashSize = 0;
	}

	/** Assignment operator. */
	TSet& operator=(const TSet& Copy)
	{
		if(this != &Copy)
		{
			Empty(Copy.Num());
			for(TConstIterator CopyIt(Copy);CopyIt;++CopyIt)
			{
				Add(*CopyIt);
			}
		}
		return *this;
	}

	/**
	 * Removes all elements from the set, potentially leaving space allocated for an expected number of elements about to be added.
	 * @param ExpectedNumElements - The number of elements about to be added to the set.
	 */
	void Empty(INT ExpectedNumElements = 0)
	{
		// Empty the elements array, and reallocate it for the expected number of elements.
		Elements.Empty(ExpectedNumElements);

		// Resize the hash to the desired size for the expected number of elements.
		if(!ConditionalRehash(ExpectedNumElements,TRUE))
		{
			// If the hash was already the desired size, clear the references to the elements that have now been removed.
			if(HashSize)
			{
				for(INT HashIndex = 0;HashIndex < HashSize;HashIndex++)
				{
					GetTypedHash(HashIndex) = FSetElementId();
				}
			}
		}
	}

	/** Shrinks the set's element storage to avoid slack. */
	void Shrink()
	{
		Elements.Shrink();
		Relax();
	}

	/** Relaxes the set's hash to a size strictly bounded by the number of elements in the set. */
	void Relax()
	{
		ConditionalRehash(Elements.Num(),TRUE);
	}

	/** 
	 * Helper function to return the amount of memory allocated by this container 
	 * @return number of bytes allocated by this container
	 */
	SIZE_T GetAllocatedSize( void ) const
	{
		return Elements.GetAllocatedSize() + (HashSize * sizeof(FSetElementId));
	}

	/** @return the number of elements. */
	INT Num() const
	{
		return Elements.Num();
	}

	/**
	 * Checks whether an element id is valid.
	 * @param Id - The element id to check.
	 * @return TRUE if the element identifier refers to a valid element in this set.
	 */
	UBOOL IsValidId(FSetElementId Id) const
	{
		return	Id.IsValidId() && 
				Id >= 0 &&
				Id < Elements.GetMaxIndex() &&
				Elements.IsAllocated(Id);
	}

	/** Accesses the identified element's value. */
	const ElementType& operator()(FSetElementId Id) const
	{
		checkSlow(IsValidId(Id));
		return Elements(Id).Value;
	}

	/** Accesses the identified element's value. */
	ElementType& operator()(FSetElementId Id)
	{
		checkSlow(IsValidId(Id));
		return Elements(Id).Value;
	}

	/**
	 * Adds an element to the set.
	 *
	 * @param	InElement					Element to add to set
	 * @param	bIsAlreadyInSetPtr	[out]	Optional pointer to bool that will be set depending on whether element is already in set
	 * @return	A pointer to the element stored in the set.  The pointer remains valid as long as the element isn't removed from the set.
	 */
	FSetElementId Add(ElementInitType InElement,UBOOL* bIsAlreadyInSetPtr = NULL)
	{
		FSetElementId ElementId;
		if(!KeyFuncs::bAllowDuplicateKeys)
		{
			// If the set doesn't allow duplicate keys, check for an existing element with the same key as the element being added.
			ElementId = FindId(KeyFuncs::GetSetKey(InElement));
		}

		// If optional pointer was passed in, report whether the element was already present in the case of not allowing duplicate keys.
		if( bIsAlreadyInSetPtr != NULL )
		{
			// Element is already in set
			if( ElementId.IsValidId() )
			{
				*bIsAlreadyInSetPtr = TRUE;
			}
			// Doesn't exist yet and will be added below.
			else
			{
				*bIsAlreadyInSetPtr = FALSE;
			}
		}

		if(KeyFuncs::bAllowDuplicateKeys || !ElementId.IsValidId())
		{
			// Create a new element.
			FSparseArrayAllocationInfo ElementAllocation = Elements.Add();
			ElementId = FSetElementId(ElementAllocation.Index);
			FElement& Element = *new(ElementAllocation) FElement(InElement);

			// Check if the hash needs to be resized.
			if(!ConditionalRehash(Elements.Num()))
			{
				// If the rehash didn't add the new element to the hash, add it.
				HashElement(ElementId,Element);
			}
		}
		else
		{
			// If there's an existing element with the same key as the new element, replace the existing element with the new element.
			Move(Elements(ElementId).Value,InElement);
		}

		return ElementId;
	}

	/**
	 * Removes an element from the set.
	 * @param Element - A pointer to the element in the set, as returned by Add or Find.
	 */
	void Remove(FSetElementId ElementId)
	{
		if(HashSize)
		{
			const FElement& ElementBeingRemoved = Elements(ElementId);

			// Remove the element from the hash.
			for(FSetElementId* NextElementId = &GetTypedHash(ElementBeingRemoved.HashIndex);
				NextElementId->IsValidId();
				NextElementId = &Elements(*NextElementId).HashNextId)
			{
				if(*NextElementId == ElementId)
				{
					*NextElementId = ElementBeingRemoved.HashNextId;
					break;
				}
			}
		}

		// Remove the element from the elements array.
		Elements.Remove(ElementId);
	}

	/**
	 * Finds an element with the given key in the set.
	 * @param Key - The key to search for.
	 * @return The id of the set element matching the given key, or the NULL id if none matches.
	 */
	FSetElementId FindId(KeyInitType Key) const
	{
		if(HashSize)
		{
			for(FSetElementId ElementId = GetTypedHash(KeyFuncs::GetKeyHash(Key));
				ElementId.IsValidId();
				ElementId = Elements(ElementId).HashNextId)
			{
				if(KeyFuncs::Matches(KeyFuncs::GetSetKey(Elements(ElementId).Value),Key))
				{
					// Return the first match, regardless of whether the set has multiple matches for the key or not.
					return ElementId;
				}
			}
		}
		return FSetElementId();
	}

	/**
	 * Finds an element with the given key in the set.
	 * @param Key - The key to search for.
	 * @return A pointer to an element with the given key.  If no element in the set has the given key, this will return NULL.
	 */
	ElementType* Find(KeyInitType Key)
	{
		FSetElementId ElementId = FindId(Key);
		if(ElementId.IsValidId())
		{
			return &Elements(ElementId).Value;
		}
		else
		{
			return NULL;
		}
	}
	
	/**
	 * Finds an element with the given key in the set.
	 * @param Key - The key to search for.
	 * @return A const pointer to an element with the given key.  If no element in the set has the given key, this will return NULL.
	 */
	const ElementType* Find(KeyInitType Key) const
	{
		FSetElementId ElementId = FindId(Key);
		if(ElementId.IsValidId())
		{
			return &Elements(ElementId).Value;
		}
		else
		{
			return NULL;
		}
	}

	/**
	 * Removes all elements from the set matching the specified key.
	 * @param Key - The key to match elements against.
	 * @return The number of elements removed.
	 */
	INT RemoveKey(KeyInitType Key)
	{
		INT NumRemovedElements = 0;

		if(HashSize)
		{
			FSetElementId* NextElementId = &GetTypedHash(KeyFuncs::GetKeyHash(Key));
			while(NextElementId->IsValidId())
			{
				FElement& Element = Elements(*NextElementId);
				if(KeyFuncs::Matches(KeyFuncs::GetSetKey(Element.Value),Key))
				{
					// This element matches the key, remove it from the set.  Note that Remove sets *NextElementId to point to the next
					// element after the removed element in the hash bucket.
					Remove(*NextElementId);
					NumRemovedElements++;

					if(!KeyFuncs::bAllowDuplicateKeys)
					{
						// If the hash disallows duplicate keys, we're done removing after the first matched key.
						break;
					}
				}
				else
				{
					NextElementId = &Element.HashNextId;
				}
			}
		}

		return NumRemovedElements;
	}

	/**
	 * Checks if the element contains an element with the given key.
	 * @param Key - The key to check for.
	 * @return TRUE if the set contains an element with the given key.
	 */
	UBOOL Contains(KeyInitType Key) const
	{
		return FindId(Key).IsValidId();
	}

	/**
	 * Sorts the set's elements using the provided comparison class.
	 */
	template<typename CompareClass>
	void Sort()
	{
		// Sort the elements according to the provided comparison class.
		Elements.Sort<ElementCompareClass<CompareClass> >();

		// Rehash.
		Rehash();
	}

	UBOOL VerifyHashElementsKey(KeyInitType Key)
	{
		UBOOL bResult=TRUE;
		if(HashSize)
		{
			// iterate over all elements for the hash entry of the given key 
			// and verify that the ids are valid
			FSetElementId ElementId = GetTypedHash(KeyFuncs::GetKeyHash(Key));
			while( ElementId.IsValidId() )
			{
				if( !IsValidId(ElementId) )
				{
					bResult=FALSE;
					break;
				}
				ElementId = Elements(ElementId).HashNextId;
			}
		}
		return bResult;
	}

	// Legacy comparison operators.  Note that these also test whether the set's elements were added in the same order!
	friend UBOOL LegacyCompareEqual(const TSet& A,const TSet& B)
	{
		return A.Elements == B.Elements;
	}
	friend UBOOL LegacyCompareNotEqual(const TSet& A,const TSet& B)
	{
		return A.Elements != B.Elements;
	}

private:

	/** An element in the set. */
	class FElement
	{
	public:

		/** The element's value. */
		ElementType Value;

		/** The id of the next element in the same hash bucket. */
		mutable FSetElementId HashNextId;

		/** The hash bucket that the element is currently linked to. */
		mutable INT HashIndex;

		/** Default constructor. */
		FElement()
		{}

		/** Initialization constructor. */
		FElement(ElementInitType InValue):
			Value(InValue)
		{}

		// Comparison operators
		inline UBOOL operator==(const FElement& Other) const
		{
			return Value == Other.Value;
		}
		inline UBOOL operator!=(const FElement& Other) const
		{
			return Value != Other.Value;
		}
	};

	/** Extracts the element value from the set's element structure and passes it to the user provided comparison class. */
	template<typename CompareClass>
	class ElementCompareClass
	{
	public:
		static INT Compare(const FElement& A,const FElement& B)
		{
			return CompareClass::Compare(A.Value,B.Value);
		}
	};

	typedef TSparseArray<FElement,typename Allocator::SparseArrayAllocator> ElementArrayType;

	ElementArrayType Elements;

	mutable typename Allocator::HashAllocator::template ForElementType<FSetElementId> Hash;
	mutable INT HashSize;

	FSetElementId& GetTypedHash(INT HashIndex) const
	{
		return ((FSetElementId*)Hash.GetAllocation())[HashIndex & (HashSize - 1)];
	}

	/**
	 * Accesses an element in the set.
	 * This is needed because the iterator classes aren't friends of FSetElementId and so can't access the element index.
	 */
	const FElement& GetInternalElement(FSetElementId Id) const
	{
		return Elements(Id);
	}
	FElement& GetInternalElement(FSetElementId Id)
	{
		return Elements(Id);
	}

	/**
	 * Translates an element index into an element ID.
	 * This is needed because the iterator classes aren't friends of FSetElementId and so can't access the FSetElementId private constructor.
	 */
	static FSetElementId IndexToId(INT Index)
	{
		return FSetElementId(Index);
	}

	/** Adds an element to the hash. */
	void HashElement(FSetElementId ElementId,const FElement& Element) const
	{
		// Compute the hash bucket the element goes in.
		Element.HashIndex = KeyFuncs::GetKeyHash(KeyFuncs::GetSetKey(Element.Value)) & (HashSize-1);

		// Link the element into the hash bucket.
		Element.HashNextId = GetTypedHash(Element.HashIndex);
		GetTypedHash(Element.HashIndex) = ElementId;
	}

	/**
	 * Checks if the hash has an appropriate number of buckets, and if not resizes it.
	 * @param NumHashedElements - The number of elements to size the hash for.
	 * @param bAllowShrinking - TRUE if the hash is allowed to shrink.
	 * @return TRUE if the set was rehashed.
	 */
	UBOOL ConditionalRehash(INT NumHashedElements,UBOOL bAllowShrinking = FALSE) const
	{
		// Calculate the desired hash size for the specified number of elements.
		const INT DesiredHashSize = Allocator::GetNumberOfHashBuckets(NumHashedElements);

		// If the hash hasn't been created yet, or is smaller than the desired hash size, rehash.
		if(NumHashedElements > 0 &&
			(!HashSize ||
			HashSize < DesiredHashSize ||
			(HashSize > DesiredHashSize && bAllowShrinking)))
		{
			HashSize = DesiredHashSize;
			Rehash();
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	/** Resizes the hash. */
	void Rehash() const
	{
		// Free the old hash.
		Hash.ResizeAllocation(0,0,sizeof(FSetElementId));

		if(HashSize)
		{
			// Allocate the new hash.
			checkSlow(!(HashSize&(HashSize-1)));
			Hash.ResizeAllocation(0,HashSize,sizeof(FSetElementId));
			for(INT HashIndex = 0;HashIndex < HashSize;HashIndex++)
			{
				GetTypedHash(HashIndex) = FSetElementId();
			}

			// Add the existing elements to the new hash.
			for(typename ElementArrayType::TConstIterator ElementIt(Elements);ElementIt;++ElementIt)
			{
				HashElement(FSetElementId(ElementIt.GetIndex()),*ElementIt);
			}
		}
	}

	/** The base type of whole set iterators. */
	template<bool bConst>
	class TBaseIterator
	{
	private:

		typedef typename TChooseClass<bConst,const ElementType,ElementType>::Result ItElementType;
		typedef typename TChooseClass<bConst,typename ElementArrayType::TConstIterator,typename ElementArrayType::TIterator>::Result ElementItType;

		// private class for safe bool conversion
		struct PrivateBooleanHelper { INT Value; };

	public:

		/** Initialization constructor. */
		TBaseIterator(const TSet& InSet,INT StartIndex)
		:	ElementIt(InSet.Elements,StartIndex)
		{}

		/** Initialization constructor. */
		TBaseIterator(TSet& InSet,INT StartIndex)
		:	ElementIt(InSet.Elements,StartIndex)
		{}

		/** Advances the iterator to the next element. */
		TBaseIterator& operator++()
		{
			++ElementIt;
			return *this;
		}

		/** conversion to "bool" returning TRUE if the iterator is valid. */
		typedef bool PrivateBooleanType;
		operator PrivateBooleanType() const { return ElementIt ? &PrivateBooleanHelper::Value : NULL; }
		bool operator !() const { return !operator PrivateBooleanType(); }

		// Accessors.
		FSetElementId GetId() const
		{
			return TSet::IndexToId(ElementIt.GetIndex());
		}
		ItElementType* operator->() const
		{
			return &ElementIt->Value;
		}
		ItElementType& operator*() const
		{
			return ElementIt->Value;
		}

	protected:
		ElementItType ElementIt;
	};

	/** The base type of whole set iterators. */
	template<bool bConst>
	class TBaseKeyIterator
	{
	private:

		typedef typename TChooseClass<bConst,const TSet,TSet>::Result SetType;
		typedef typename TChooseClass<bConst,const ElementType,ElementType>::Result ItElementType;

		// private class for safe bool conversion
		struct PrivateBooleanHelper { INT Value; };

	public:
		/** Initialization constructor. */
		TBaseKeyIterator(SetType& InSet,KeyInitType InKey)
		:	Set(InSet)
		,	Key(InKey)
		,	Id()
		{
			// The set's hash needs to be initialized to find the elements with the specified key.
			Set.ConditionalRehash(Set.Elements.Num());
			if(Set.HashSize)
			{
				NextId = Set.GetTypedHash(KeyFuncs::GetKeyHash(Key));
				++(*this);
			}
		}

		/** Advances the iterator to the next element. */
		TBaseKeyIterator& operator++()
		{
			Id = NextId;

			while(Id.IsValidId())
			{
				NextId = Set.GetInternalElement(Id).HashNextId;

				if(KeyFuncs::Matches(KeyFuncs::GetSetKey(Set(Id)),Key))
				{
					break;
				}

				Id = NextId;
			}
			return *this;
		}

		/** conversion to "bool" returning TRUE if the iterator is valid. */
		typedef bool PrivateBooleanType;
		operator PrivateBooleanType() const { return Id.IsValidId() ? &PrivateBooleanHelper::Value : NULL; }
		bool operator !() const { return !operator PrivateBooleanType(); }

		// Accessors.
		ItElementType* operator->() const
		{
			return &Set(Id);
		}
		ItElementType& operator*() const
		{
			return Set(Id);
		}

	protected:
		SetType& Set;
		typename TContainerTraits<typename KeyFuncs::KeyType>::ConstPointerType Key;
		FSetElementId Id;
		FSetElementId NextId;
	};

public:

	/** Used to iterate over the elements of a const TSet. */
	class TConstIterator : public TBaseIterator<true>
	{
	public:
		TConstIterator(const TSet& InSet,INT StartIndex = 0):
			TBaseIterator<true>(InSet,StartIndex)
		{}
	};

	/** Used to iterate over the elements of a TSet. */
	class TIterator : public TBaseIterator<false>
	{
	public:
		TIterator(TSet& InSet,INT StartIndex = 0)
		:	TBaseIterator<false>(InSet,StartIndex)
		,	Set(InSet)
		{}

		/** Removes the current element from the set. */
		void RemoveCurrent()
		{
			Set.Remove(TBaseIterator<false>::GetId());
		}
	private:
		TSet& Set;
	};
	
	/** Used to iterate over the elements of a const TSet. */
	class TConstKeyIterator : public TBaseKeyIterator<true>
	{
	public:
		TConstKeyIterator(const TSet& InSet,KeyInitType InKey):
			TBaseKeyIterator<true>(InSet,InKey)
		{}
	};

	/** Used to iterate over the elements of a TSet. */
	class TKeyIterator : public TBaseKeyIterator<false>
	{
	public:
		TKeyIterator(TSet& InSet,KeyInitType InKey)
		:	TBaseKeyIterator<false>(InSet,InKey)
		,	Set(InSet)
		{}

		/** Removes the current element from the set. */
		void RemoveCurrent()
		{
			Set.Remove(TBaseKeyIterator<false>::Id);
			TBaseKeyIterator<false>::Id = FSetElementId();
		}
	private:
		TSet& Set;
	};
};

}
