/*=============================================================================
	LMTypes.h: Basic types used by Lightmass
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/


#pragma once

// get the shared base types (DWORD, INT, etc)
#include "..\..\Public\PublicTypes.h"


namespace Lightmass
{

// SIZE_T is defined in windows.h based on WIN64 or not.
// Plus, it uses ULONG instead of size_t, so we can't use size_t.
// Not defined in PublicTypes because this will change size, so is not safe
// to be shared
#ifdef _WIN64
typedef SQWORD				PTRINT;		// Integer large enough to hold a pointer.
#else
typedef INT					PTRINT;		// Integer large enough to hold a pointer.
#endif



//
// Hash functions for common types.
//

inline DWORD GetTypeHash( const BYTE A )
{
	return A;
}
inline DWORD GetTypeHash( const SBYTE A )
{
	return A;
}
inline DWORD GetTypeHash( const WORD A )
{
	return A;
}
inline DWORD GetTypeHash( const SWORD A )
{
	return A;
}
inline DWORD GetTypeHash( const INT A )
{
	return A;
}
inline DWORD GetTypeHash( const DWORD A )
{
	return A;
}
inline DWORD GetTypeHash( const QWORD A )
{
	return (DWORD)A+((DWORD)(A>>32) * 23);
}
inline DWORD GetTypeHash( const SQWORD A )
{
	return (DWORD)A+((DWORD)(A>>32) * 23);
}

inline DWORD GetTypeHash( const void* A )
{
	return (DWORD)(PTRINT)A;
}

inline DWORD GetTypeHash( void* A )
{
	return (DWORD)(PTRINT)A;
}


/*-----------------------------------------------------------------------------
	From UnTypeTraits.h
-----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
 * Macros to abstract the presence of certain compiler intrinsic type traits 
 -----------------------------------------------------------------------------*/
#if _MSC_VER >= 1400
	#define HAS_TRIVIAL_CONSTRUCTOR(T) __has_trivial_constructor(T)
	#define HAS_TRIVIAL_DESTRUCTOR(T) __has_trivial_destructor(T)
	#define HAS_TRIVIAL_ASSIGN(T) __has_trivial_assign(T)
	#define HAS_TRIVIAL_COPY(T) __has_trivial_copy(T)
	#define IS_POD(T) __is_pod(T)
	#define IS_ENUM(T) __is_enum(T)
	#define IS_EMPTY(T) __is_empty(T)
#else
	#define HAS_TRIVIAL_CONSTRUCTOR(T) false
	#define HAS_TRIVIAL_DESTRUCTOR(T) false
	#define HAS_TRIVIAL_ASSIGN(T) false
	#define HAS_TRIVIAL_COPY(T) false
	#define IS_POD(T) false
	#define IS_ENUM(T) false
	#define IS_EMPTY(T) false
#endif


/*-----------------------------------------------------------------------------
	Type traits similar to TR1 (uses intrinsics supported by VC8)
	Should be updated/revisited/discarded when compiler support for tr1 catches up.
 -----------------------------------------------------------------------------*/

/**
 * TIsFloatType
 */
template<typename T> struct TIsFloatType { enum { Value = false }; };

template<> struct TIsFloatType<float> { enum { Value = true }; };
template<> struct TIsFloatType<double> { enum { Value = true }; };
template<> struct TIsFloatType<long double> { enum { Value = true }; };

/**
 * TIsIntegralType
 */
template<typename T> struct TIsIntegralType { enum { Value = false }; };

template<> struct TIsIntegralType<unsigned char> { enum { Value = true }; };
template<> struct TIsIntegralType<unsigned short> { enum { Value = true }; };
template<> struct TIsIntegralType<unsigned int> { enum { Value = true }; };
template<> struct TIsIntegralType<unsigned long> { enum { Value = true }; };

template<> struct TIsIntegralType<signed char> { enum { Value = true }; };
template<> struct TIsIntegralType<signed short> { enum { Value = true }; };
template<> struct TIsIntegralType<signed int> { enum { Value = true }; };
template<> struct TIsIntegralType<signed long> { enum { Value = true }; };

template<> struct TIsIntegralType<bool> { enum { Value = true }; };
template<> struct TIsIntegralType<char> { enum { Value = true }; };

// compilers we support define wchar_t as a native type
#if !_MSC_VER || defined(_NATIVE_WCHAR_T_DEFINED)
	template<> struct TIsIntegralType<wchar_t> { enum { Value = true }; };
#endif

// C99, but all compilers we use support it
template<> struct TIsIntegralType<unsigned long long> { enum { Value = true }; };
template<> struct TIsIntegralType<signed long long> { enum { Value = true }; };

/**
 * TIsArithmeticType
 */
template<typename T> struct TIsArithmeticType 
{ 
	enum { Value = TIsIntegralType<T>::Value || TIsFloatType<T>::Value } ;
};

/**
 * TIsPointerType
 * @todo - exclude member pointers
 */
template<typename T> struct TIsPointerType						{ enum { Value = false }; };
template<typename T> struct TIsPointerType<T*>					{ enum { Value = true }; };
template<typename T> struct TIsPointerType<const T*>			{ enum { Value = true }; };
template<typename T> struct TIsPointerType<const T* const>		{ enum { Value = true }; };
template<typename T> struct TIsPointerType<T* volatile>			{ enum { Value = true }; };
template<typename T> struct TIsPointerType<T* const volatile>	{ enum { Value = true }; };

/**
 * TIsVoidType
 */
template<typename T> struct TIsVoidType { enum { Value = false }; };
template<> struct TIsVoidType<void> { enum { Value = true }; };
template<> struct TIsVoidType<void const> { enum { Value = true }; };
template<> struct TIsVoidType<void volatile> { enum { Value = true }; };
template<> struct TIsVoidType<void const volatile> { enum { Value = true }; };

/**
 * TIsPODType
 * @todo - POD array and member pointer detection
 */
template<typename T> struct TIsPODType 
{ 
	enum { Value = IS_POD(T) || IS_ENUM(T) || TIsArithmeticType<T>::Value || TIsPointerType<T>::Value }; 
};

/**
 * TIsFundamentalType
 */
template<typename T> 
struct TIsFundamentalType 
{ 
	enum { Value = TIsArithmeticType<T>::Value || TIsVoidType<T>::Value };
};

/*-----------------------------------------------------------------------------
	Call traits - Modeled somewhat after boost's interfaces.
-----------------------------------------------------------------------------*/

/**
 * Call traits helpers
 */
template <typename T, bool TypeIsSmall>
struct TCallTraitsParamTypeHelper
{
	typedef const T& ParamType;
	typedef const T& ConstParamType;
};
template <typename T>
struct TCallTraitsParamTypeHelper<T, true>
{
	typedef const T ParamType;
	typedef const T ConstParamType;
};
template <typename T>
struct TCallTraitsParamTypeHelper<T*, true>
{
	typedef T* ParamType;
	typedef const T* ConstParamType;
};

/*-----------------------------------------------------------------------------
 * TCallTraits
 *
 * Same call traits as boost, though not with as complete a solution.
 *
 * The main member to note is ParamType, which specifies the optimal 
 * form to pass the type as a parameter to a function.
 * 
 * Has a small-value optimization when a type is a POD type and as small as a pointer.
-----------------------------------------------------------------------------*/

/**
 * base class for call traits. Used to more easily refine portions when specializing
 */
template <typename T>
struct TCallTraitsBase
{
private:
	enum { PassByValue = TIsArithmeticType<T>::Value || TIsPointerType<T>::Value || (TIsPODType<T>::Value && sizeof(T) <= sizeof(void*)) };
public:
	typedef T ValueType;
	typedef T& Reference;
	typedef const T& ConstReference;
	typedef typename TCallTraitsParamTypeHelper<T, PassByValue>::ParamType ParamType;
	typedef typename TCallTraitsParamTypeHelper<T, PassByValue>::ConstParamType ConstPointerType;
};

/**
 * TCallTraits
 */
template <typename T>
struct TCallTraits : public TCallTraitsBase<T> {};

// Fix reference-to-reference problems.
template <typename T>
struct TCallTraits<T&>
{
	typedef T& ValueType;
	typedef T& Reference;
	typedef const T& ConstReference;
	typedef T& ParamType;
	typedef T& ConstPointerType;
};

// Array types
template <typename T, size_t N>
struct TCallTraits<T [N]>
{
private:
	typedef T ArrayType[N];
public:
	typedef const T* ValueType;
	typedef ArrayType& Reference;
	typedef const ArrayType& ConstReference;
	typedef const T* const ParamType;
	typedef const T* const ConstPointerType;
};

// const array types
template <typename T, size_t N>
struct TCallTraits<const T [N]>
{
private:
	typedef const T ArrayType[N];
public:
	typedef const T* ValueType;
	typedef ArrayType& Reference;
	typedef const ArrayType& ConstReference;
	typedef const T* const ParamType;
	typedef const T* const ConstPointerType;
};


/*-----------------------------------------------------------------------------
	Traits for our particular container classes
-----------------------------------------------------------------------------*/

/**
 * Helper for array traits. Provides a common base to more easily refine a portion of the traits
 * when specializing. NeedsConstructor/NeedsDestructor is mainly used by the contiguous storage 
 * containers like TArray.
 */
template<typename T> struct TContainerTraitsBase
{
	typedef typename TCallTraits<T>::ParamType ConstInitType;
	typedef typename TCallTraits<T>::ConstPointerType ConstPointerType;
	// WRH - 2007/11/28 - the compilers we care about do not produce equivalently efficient code when manually
	// calling the constructors of trivial classes. In array cases, we can call a single memcpy
	// to initialize all the members, but the compiler will call memcpy for each element individually,
	// which is slower the more elements you have. 
	enum { NeedsConstructor = !HAS_TRIVIAL_CONSTRUCTOR(T) && !TIsPODType<T>::Value };
	// WRH - 2007/11/28 - the compilers we care about correctly elide the destructor code on trivial classes
	// (effectively compiling down to nothing), so it is not strictly necessary that we have NeedsDestructor. 
	// It doesn't hurt, though, and retains for us the ability to skip destructors on classes without trivial ones
	// if we should choose.
	enum { NeedsDestructor = !HAS_TRIVIAL_DESTRUCTOR(T) && !TIsPODType<T>::Value };
};

/**
 * Traits for container classes.
 */
template<typename T> struct TContainerTraits : public TContainerTraitsBase<T> {};


}
