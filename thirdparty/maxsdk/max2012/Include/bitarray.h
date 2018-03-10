/**********************************************************************
*<
FILE: bitarray.h

DESCRIPTION:

CREATED BY: Dan Silva

HISTORY:

*>   Copyright (c) 1994, All Rights Reserved.
**********************************************************************/

#pragma once

#include "GeomExport.h"
#include "maxheap.h"
#include <wtypes.h>
#include <limits.h>
#include "maxtypes.h"
#include "assert1.h"

// forward declarations
class ILoad;
class ISave;

/*! \sa  Class BitArray.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This is the callback object for the method <b>BitArray::EnumSet()</b>. The proc
method is called for each "1" in the BitArray.  */
class BitArrayCallback: public MaxHeapOperators
{
public:
	/*! \remarks This method is called for each "1" in the BitArray.
	\par Parameters:
	<b>int n</b>\n\n
	This is the zero based index into the BitArray of the element which is "1".
	*/
	virtual void proc(int n)=0;
};

// Direction indicators for BitArray::Rotate and BitArray::Shift
enum
{
	LEFT_BITSHIFT  = 0,
	RIGHT_BITSHIFT = 1,
};

// Generate statistics on bitarrays created.  Requires a complete rebuild to toggle
// on and off.
// #define DL_BITARRAY_STATS

/*! \sa  Template Class Tab, Class BitArrayCallback.\n\n
\par Description:
This class allows the developer to define a set of bit flags that may be
treated as a virtual array and are stored in an efficient manner. The class has
methods to set, clear and return the i-th bit, resize the BitArray, etc. All
methods are implemented by the system.  */
class BitArray: public MaxHeapOperators {
	enum
	{
		kMAX_LOCALBITS = CHAR_BIT * sizeof(DWORD_PTR),
	};

	union
	{
		// numBits cannot be put into a DWORD_PTR so memory had to be allocated.
		DWORD_PTR*  bits;

		// bits fit into a DWORD_PTR object; no memory allocation needed.
		DWORD_PTR   localBits;
	};

	long numBits;

public:
	class NumberSetProxy : public MaxHeapOperators
	{
	public:
		inline operator bool() const           // if( array.NumberSet() )
		{
			return !mArray.IsEmpty();
		}

		inline bool operator !() const         // if( !array.NumberSet() )
		{
			return mArray.IsEmpty();
		}

		inline operator int() const            // int n = array.NumberSet();
		{
			return mArray.NumberSetImpl();
		}

		inline operator DWORD_PTR() const
		{
			return mArray.NumberSetImpl();
		}

		inline operator float() const
		{
			return (float)mArray.NumberSetImpl();
		}

#ifdef WIN64
		inline operator DWORD() const
		{
			return mArray.NumberSetImpl();
		}
#endif

		inline bool operator <(int n) const    // if( array.NumberSet() < 3 )
		{
			// if( NumberSet() < 0 ) or a negative, always returns false.
			// if( NumberSet() < 1 ), basically mean "IsEmpty()".
			// if( NumberSet() < n ), we use !(NumberSet() >= n)
			return (n <= 0) ? false : ((n == 1) ? mArray.IsEmpty() : !mArray.NumberSetAtLeastImpl(n));
		}

		inline bool operator <=(int n) const   // if( array.NumberSet() <= 3 )
		{
			// if( x <= n ) ==> if( !(x >= (n+1)) )
			return !mArray.NumberSetAtLeastImpl(n+1);
		}

		inline bool operator >(int n) const    // if( array.NumberSet() > 3 )
		{
			// if( x > 0 ) ==> !IsEmpty()
			// if( x > n ) ==> if( x >= (n+1) )
			return n ? mArray.NumberSetAtLeastImpl(n+1) : !mArray.IsEmpty();
		}

		inline bool operator >=(int n) const   // if( array.NumberSet() >= 3 )
		{
			return mArray.NumberSetAtLeastImpl(n);
		}

		inline bool operator ==(int n) const   // if( array.NumberSet() == 3 )
		{
			return mArray.NumberSetEqualImpl(n);
		}

		inline bool operator !=(int n) const   // if( array.NumberSet() != 3 )
		{
			return !mArray.NumberSetEqualImpl(n);
		}

		inline int operator +(int n) const         // int n = array.NumberSet() + 3;
		{
			return mArray.NumberSetImpl() + n;
		}

		inline int operator -(int n) const         // int n = array.NumberSet() + 3;
		{
			return mArray.NumberSetImpl() - n;
		}

		inline int operator *(int n) const         // int n = array.NumberSet() * 3;
		{
			return mArray.NumberSetImpl() * n;
		}

		inline int operator /(int n) const         // int n = array.NumberSet() / 3;
		{
			return mArray.NumberSetImpl() / n;
		}

		inline int operator %(int n) const         // int n = array.NumberSet() % 3;
		{
			return mArray.NumberSetImpl() % n;
		}

		inline int operator +(const NumberSetProxy& proxy) const
		{
			return mArray.NumberSetImpl() + int(proxy);
		}

		inline int operator -(const NumberSetProxy& proxy) const
		{
			return mArray.NumberSetImpl() - int(proxy);
		}

		inline int operator *(const NumberSetProxy& proxy) const
		{
			return mArray.NumberSetImpl() * int(proxy);
		}

	private:
		const BitArray& mArray;

		friend class BitArray;
		// Can only be created by the BitArray itself.
		inline NumberSetProxy(const BitArray& a) : mArray(a) {}
		NumberSetProxy& operator = (const NumberSetProxy& rhs);
	};

	friend class NumberSetProxy;

public:
	/*! \remarks Default constructor. Sets the number of bits to 0. */
	inline BitArray() { bits = NULL; numBits = 0; BitArrayAllocated(); }
	/*! \remarks Constructor.
	\par Parameters:
	<b>int i</b>\n\n
	The size of the BitArray in bits. */
	inline BitArray(int n)
	{
		DbgAssert( n >= 0 );
		if (n < 0)
		{
			n = 0;
		}
		if( UseLocalBits(n) )
		{
			numBits     = n;
			localBits   = 0;

			BitArrayAllocated();
		}
		else
		{
			CreateBitArrayImpl(n);
		}
	}
	/*! \remarks Constructor. Duplicates the BitArray passed.
	\par Parameters:
	<b>const BitArray\& b</b>\n\n
	The BitArray to duplicate. */
	inline BitArray(const BitArray& b)
	{
		if( b.UseLocalBits() )
		{
			localBits   = b.localBits;
			numBits     = b.numBits;

			BitArrayAllocated();
		}
		else
		{
			SetBitsFromImpl(b);
		}
	}

	inline ~BitArray()
	{ 
		if( !UseLocalBits() ) 
			FreeBitsImpl(); 
		else 
			BitArrayDeallocated();
	}

	/*! \remarks Sets the number of bits used.
	\param n - The number of bits in to be in the array. If this value is a negative number, or
				equal to the current size of the BitArray then nothing will happen.
	\param save=0 - If passed as 1, the old bit values will be preserved when the array is resized. */
	GEOMEXPORT void SetSize(int n, int save=0);  // save=1:preserve old bit values

	/*! \remarks Returns the size of the bit array in bits. */
	inline int  GetSize() const { return numBits; }

	/*! \remarks Clears all the bits in the array (sets them to 0). */
	inline void ClearAll()
	{
		UseLocalBits() ? localBits = 0 : ClearAllImpl();
	}

	/*! \remarks Sets all the bits in the array to 1. */
	inline void SetAll() // Only set used bits; leave the others at zero.
	{
		UseLocalBits() ? localBits = BitMask(numBits) - 1 : SetAllImpl();
	}

	/*! \remarks Set the i-th bit to 1.
	\param i - The array index of the bit to set. */
	inline void Set(int i)
	{
		DbgAssert(i>-1 && i<numBits);
		if ((i > -1) && (i < numBits))
		{
			UseLocalBits() ? localBits |= BitMask(i) : SetImpl(i);
		}
	}

	/*! \remarks Sets the i-th bit to 0.
	\par Parameters:
	<b>int i</b>\n\n
	The array index of the bit to clear. */
	inline void Clear(int i)
	{
		DbgAssert(i>-1 && i<numBits);
		if ((i > -1) && (i < numBits))
		{
			UseLocalBits() ? localBits &= ~BitMask(i) : ClearImpl(i);
		}
	}

	/*! \remarks Set the i-th bit to b.
	\param i - The index of the bit to set.
	\param b - The value to set, either 1 or 0. */
	inline void Set(int i, int b) { b ? Set(i) : Clear(i); }

	/*! \remarks Gets the i-th bit.
	\param i - The index of the bit. If the index is a negative or bigger than the array size, it returns 0 */
	inline int operator[](int i) const
	{
		DbgAssert (i>-1);
		DbgAssert (i<numBits);
		if ((i > -1) && (i < numBits))
			return UseLocalBits() ? (localBits & BitMask(i) ? 1 : 0) : GetNthBitImpl(i);
		else
			return 0;
	}

	/*! \remarks		Returns true if no bits are set; otherwise false. This method is much faster
	than checking if <b>NumberSet()</b> returns 0. */
	inline bool IsEmpty() const   { return UseLocalBits() ? !localBits : IsEmptyImpl(); }
	inline bool AnyBitSet() const { return !IsEmpty(); }

	/*! \remarks how many bits are 1's? use IsEmpty() for faster checks
	\return Returns a proxy object which can optimize client code depending on the type
	of access required (ie: != 0 would call IsEmpty(), etc)*/
	inline NumberSetProxy NumberSet() const
	{
		return NumberSetProxy(*this);
	}



	/*! \remarks This is not currently implemented and is reserved for
	future use. */
	GEOMEXPORT void Compress();
	/*! \remarks This is not currently implemented and is reserved for
	future use. */
	GEOMEXPORT void Expand();
	/*! \remarks Reverses the bits in the BitArray.
	\par Parameters:
	<b>BOOL keepZero = FALSE</b>\n\n
	If TRUE the zero bit is kept where it is. */
	GEOMEXPORT void Reverse(BOOL keepZero = FALSE);  // keepZero=TRUE keeps zero bit where it is
	/*! \remarks Rotates the bits in the BitArray (with wraparound).
	\par Parameters:
	<b>int direction</b>\n\n
	The direction to rotate.\n\n
	<b>int count</b>\n\n
	The number of bits to rotate. */
	GEOMEXPORT void Rotate(int direction, int count);            // With wraparound
	/*! \remarks Shifts the bits in the BitArray (without wraparound).
	\par Parameters:
	<b>int direction</b>\n\n
	One of the following values:\n\n
	<b>LEFT_BITSHIFT</b>\n\n
	<b>RIGHT_BITSHIFT</b>\n\n
	<b>int count</b>\n\n
	The number of bits to shift.\n\n
	<b>int where=0</b>\n\n
	This indicates where the shift will begin. For example, if you have a
	<b>BitArray</b> containing: <b>10101010</b>\n\n
	and you <b>Shift(LEFT_BITSHIFT, 1, 4)</b> you'll get: <b>10100100</b>\n\n
	All the bits from 4 to 8 are shifted one bit left, with zeroes shifted in
	from the right. The first bit affected is the <b>where</b> bit. If you
	leave off the <b>where</b> parameter you'd get the usual:
	<b>01010100</b>\n\n
	The <b>RIGHT_BITSHIFT</b> starts at that bit; it is unaffected because
	the operation proceeds to the right: <b>10101010</b>.\n\n
	<b>Shift(RIGHT_BITSHIFT, 1, 4)</b> results in: <b>10101101</b>. */
	GEOMEXPORT void Shift(int direction, int count, int where=0);   // Without wraparound
	/*! \remarks This method is used to enumerate all the elements that have a "1" value,
	and call the callback <b>proc()</b> with the index of the element.
	\par Parameters:
	<b>BitArrayCallback \&cb</b>\n\n
	The callback object whose <b>proc()</b> method is called. */
	GEOMEXPORT void EnumSet(BitArrayCallback &cb);  // enumerates elements that are 1's
	/*! \remarks This method allows you to delete a selection of elements from this
	BitArray. This is useful, for instance, if you're deleting a set of
	vertices from a mesh and wish to keep the vertSel and vertHide arrays up
	to date.
	\par Parameters:
	<b>BitArray \& dset</b>\n\n
	This is a bit array which represents which elements should be deleted.
	Typically (if mult==1) dset will have the same size as (this).\n\n
	<b>int mult=1</b>\n\n
	This is a multiplier which indicates how many elements in (*this) are
	deleted for each entry in dset. For instance, when deleting faces in a
	mesh, you also need to delete the corresponding edge selection data.
	Since edgeSel[f*3], edgeSel[f*3+1], and edgeSel[f*3+2] correspond to face
	f, you'd use mult=3:\n\n
	<b>faceSel.DeleteSet (fdel);</b>\n\n
	<b>edgeSel.DeleteSet (fdel, 3);</b> */
	GEOMEXPORT void DeleteSet (BitArray & dset, int mult=1);
	/*! \remarks Saves the BitArray to the 3ds Max file. */
	GEOMEXPORT IOResult Save(ISave* isave);
	/*! \remarks Loads the BitArray from the 3ds Max file.
	\par Operators:
	*/
	GEOMEXPORT IOResult Load(ILoad* iload);

	/*! \remarks This operator is available in release 3.0 and later only.\n\n
	Comparison operator.
	\par Parameters:
	<b>const BitArray\& b</b>\n\n
	The BitArray to compare with this one.
	\return  true if the BitArrays are 'equal' (same size and same bits set);
	otherwise false. */
	inline bool operator==(const BitArray& b) const
	{
		return (numBits == b.numBits) && (UseLocalBits() ? (localBits == b.localBits) : CompareBitsImpl(b));
	}

	// Assignment operators
	/*! \remarks Assignment operator. */
	GEOMEXPORT BitArray& operator=(const BitArray& b);

	// Assignment operators: These require arrays of the same size!
	/*! \remarks AND= this BitArray with the specified BitArray. */
	GEOMEXPORT BitArray& operator&=(const BitArray& b);  // AND=
	/*! \remarks OR= this BitArray with the specified BitArray. */
	GEOMEXPORT BitArray& operator|=(const BitArray& b);  // OR=
	/*! \remarks XOR= this BitArray with the specified BitArray. */
	GEOMEXPORT BitArray& operator^=(const BitArray& b);  // XOR=

	// Binary operators: These require arrays of the same size!
	/*! \remarks AND two BitArrays */
	GEOMEXPORT BitArray operator&(const BitArray&) const; // AND
	/*! \remarks OR two BitArrays */
	GEOMEXPORT BitArray operator|(const BitArray&) const; // OR
	/*! \remarks XOR two BitArrays */
	GEOMEXPORT BitArray operator^(const BitArray&) const; // XOR

	// Unary operators
	/*! \remarks Unary NOT function */
	inline BitArray operator~() const
	{
		return UseLocalBits() ? BitArray(~localBits, numBits, true) : OperatorNotImpl();
	}

	//! \brief Swap the contents of two bitarrays.
	/*! This is an efficient way of transfering the contents of a temporary bitarray
	object into a more permanent instance, such as a data member.  For instance:
	\code
	{ 
	BitArray	tmp(size);
	// do something with tmp...
	m_MyBitArray.Swap(tmp);
	}
	\endcode
	would be more efficient than using operator= in this case.
	\param[in, out] other The contents of 'other' will be swaped with the contents of 'this' */
	GEOMEXPORT void Swap(BitArray& other);
private:
	inline BitArray(DWORD_PTR localBits_, long numBits_, bool zeroHighBits = false) :
	localBits(localBits_), numBits(numBits_)
	{
		DbgAssert( UseLocalBits() );

		if( zeroHighBits )
			ZeroUnusedBitsImpl();

		BitArrayAllocated();
	}

	inline bool UseLocalBits() const { return numBits <= kMAX_LOCALBITS; }
	inline bool UseLocalBits(int n) const { return n <= kMAX_LOCALBITS; }
	inline DWORD_PTR BitMask(int i) const
		// NOTE: Shifting by kMAX_LOCALBITS will give an undefined behavior; the
		// chip actually limits the shift from 0 to kMAX_LOCALBITS-1, so most likely
		// you simply return '1' when what you wanted was zero.
	{ return (i < kMAX_LOCALBITS) ? (DWORD_PTR(1) << i) : DWORD_PTR(0); }

	// Used internally to treat the bit array the same way whether it's new'ed or
	// simply local.
	inline const DWORD_PTR* GetBitPtr() const
	{
		return UseLocalBits() ? &localBits : bits;
	}

	inline DWORD_PTR* GetBitPtr()
	{
		return UseLocalBits() ? &localBits : bits;
	}

	// Called from the ctor only; initializes an array filled with zeroes.
	GEOMEXPORT void CreateBitArrayImpl(int n);
	// Called from the ctor only; initializes from an array of bits.
	GEOMEXPORT void SetBitsFromImpl(const BitArray&);

	GEOMEXPORT void ClearAllImpl();
	GEOMEXPORT void SetAllImpl();

	GEOMEXPORT void SetImpl(int i);
	GEOMEXPORT void ClearImpl(int i);

	GEOMEXPORT void SetImpl(int i, int b);
	GEOMEXPORT int  GetNthBitImpl(int i) const;
	GEOMEXPORT int  NumberSetImpl() const;
	GEOMEXPORT bool IsEmptyImpl() const;

	GEOMEXPORT BitArray OperatorNotImpl() const;
	GEOMEXPORT bool CompareBitsImpl(const BitArray&) const;

	GEOMEXPORT void FreeBitsImpl();

#ifndef DL_BITARRAY_STATS
	inline void BitArrayAllocated() {}
	inline void BitArrayDeallocated() {}
#else
	class BitArrayStats;
	friend class BitArrayStats;

	GEOMEXPORT void BitArrayAllocated();
	GEOMEXPORT void BitArrayDeallocated();
#endif

	GEOMEXPORT bool NumberSetImpl(int n) const;         // Exhaustive count; can be dead slow
	GEOMEXPORT bool NumberSetEqualImpl(int n) const;    // Stops as soon as count will be higher
	GEOMEXPORT bool NumberSetAtLeastImpl(int n) const;  // Stops as soon as count reaches limit.

	// Zeroes out bits over numBits so we can always use fast comparisons (memcmp,
	// ==, etc) without having to mask out the last chunk.
	GEOMEXPORT void ZeroUnusedBitsImpl();
};

// Help the compiler out when the array.NumberSet is on the right-hand of the equation
template <typename T> inline T operator +(T n, const BitArray::NumberSetProxy& proxy)
{
	return n + proxy.operator int();
}

template <typename T> inline T operator -(T n, const BitArray::NumberSetProxy& proxy)
{
	return n - proxy.operator int();
}

template <typename T> inline T operator *(T n, const BitArray::NumberSetProxy& proxy)
{
	return proxy.operator *(n);
}

template <typename T> inline T operator /(T n, const BitArray::NumberSetProxy& proxy)
{
	return n / proxy.operator int();
}

template <typename T> inline T operator %(T n, const BitArray::NumberSetProxy& proxy)
{
	return n % proxy.operator int();
}

template <typename T> inline bool operator <=(T n, const BitArray::NumberSetProxy& proxy)
{
	return proxy.operator >=(n);
}

template <typename T> inline bool operator <(T n, const BitArray::NumberSetProxy& proxy)
{
	return proxy.operator >(n);
}

template <typename T> inline bool operator >(T n, const BitArray::NumberSetProxy& proxy)
{
	return proxy.operator <(n);
}

template <typename T> inline bool operator >=(T n, const BitArray::NumberSetProxy& proxy)
{
	return proxy.operator <=(n);
}

template <typename T> inline bool operator ==(T n, const BitArray::NumberSetProxy& proxy)
{
	return proxy.operator ==(n);
}

template <typename T> inline bool operator !=(T n, const BitArray::NumberSetProxy& proxy)
{
	return proxy.operator !=(n);
}

template <typename T> inline void operator +=(T& n, const BitArray::NumberSetProxy& proxy)
{
	n += proxy.operator int();
}

template <typename T> inline void operator -=(T& n, const BitArray::NumberSetProxy& proxy)
{
	n -= proxy.operator int();
}

