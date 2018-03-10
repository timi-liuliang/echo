#ifndef __ECHO_MEMALLOCDEF_H__
#define __ECHO_MEMALLOCDEF_H__

#include "MemDef.h"
#include "MemAllocObj.h"
#include "MemSTLAlloc.h"

#ifndef ECHO_ALIGN
#define ECHO_ALIGN
namespace Echo
{
	template< class T > inline T Align( const T Ptr, int Alignment )
	{
		return (T)(((size_t)Ptr + Alignment - 1) & ~(Alignment-1));
	}
}
#endif

#if (ECHO_MEMORY_ALLOCATOR == ECHO_MEMORY_ALLOCATOR_BINNED)
#include "MemBinnedAlloc.h"

namespace Echo
{
	class CategorisedAllocPolicy : public BinnedAllocPolicy{};
	template <size_t align = 16> class CategorisedAlignAllocPolicy : public BinnedAlignedAllocPolicy<align>{};
	
	class NoMemTraceAllocPolicy : public BinnedAllocPolicyNoMemTrace{};
	template <size_t align = 16> class AlignNoMemTraceAllocPolicy : public BinnedAlignedAllocPolicyNoMemTrace<align>{};
}
#endif

#if (ECHO_MEMORY_ALLOCATOR == ECHO_MEMORY_ALLOCATOR_DEFAULT)
#include "MemDefaultAlloc.h"
namespace Echo
{
	class CategorisedAllocPolicy : public DefaultPolicy{};
	template <size_t align = 16> class CategorisedAlignAllocPolicy : public DefaultAlignedPolicy<align>{};
	
	class NoMemTraceAllocPolicy : public DefaultNoMemTracePolicy{};
	template <size_t align = 16> class AlignNoMemTraceAllocPolicy : public DefaultAlignedNoMemTracePolicy<align>{};
}
#endif

	namespace Echo
	{
		// Useful shortcuts
		typedef CategorisedAllocPolicy GAP;
		typedef CategorisedAlignAllocPolicy<16> GAAP;

		typedef NoMemTraceAllocPolicy STLAP;
		typedef AlignNoMemTraceAllocPolicy<16> STLAAP;

		// Now define all the base classes for each allocation
		typedef AllocatedObject<GAP> GeneralAllocatedObject;

		// Per-class allocators defined here
		// NOTE: small, non-virtual classes should not subclass an allocator
		// the virtual function table could double their size and make them less efficient
		// use primitive or STL allocators / deallocators for those
		//typedef GeneralAllocatedObject		ObjectAlloc;

		// Containers (by-value only)
		// Will  be of the form:
		// typedef STLAllocator<T, DefaultAllocPolicy, Category> TAlloc;
		// for use in vector<T, TAlloc>::type 

		//////////////////////////////////////////////////////////////////////////

		/** Utility function for constructing an array of objects with placement new,
		without using new[] (which allocates an undocumented amount of extra memory
		and so isn't appropriate for custom allocators).
		*/
		template<typename T>
		T* EchoConstruct(T* basePtr, size_t count)
		{
			for (size_t i = 0; i < count; ++i)
			{
				new ((void*)(basePtr+i)) T();
			}
			return basePtr;
		}
	}


#ifndef ECHO_REPLACE_OPERATOR_NEW_DEL
#define ECHO_REPLACE_OPERATOR_NEW_DEL
#endif

#if (ECHO_MEMORY_TRACKER)

/// Allocate a block of raw memory.
#	define ECHO_MALLOC(bytes) ::Echo::CategorisedAllocPolicy::allocateBytes(bytes, __FILE__, __LINE__, __FUNCTION__)
/// Allocate a block of memory for a primitive type.
#	define ECHO_ALLOC_T(T, count) static_cast<T*>(::Echo::CategorisedAllocPolicy::allocateBytes(sizeof(T)*(count), __FILE__, __LINE__, __FUNCTION__))

#if (ECHO_MEMORY_ALLOCATOR == ECHO_MEMORY_ALLOCATOR_DEFAULT || ECHO_MEMORY_ALLOCATOR == ECHO_MEMORY_ALLOCATOR_BINNED)
#   define ECHO_REALLOC(ptr, newBytes) ::Echo::CategorisedAllocPolicy::reallocBytes(ptr,newBytes, __FILE__, __LINE__, __FUNCTION__)
#endif

/// Free the memory allocated with ECHO_MALLOC or ECHO_ALLOC_T.
#	define ECHO_FREE(ptr) ::Echo::CategorisedAllocPolicy::deallocateBytes((void*)ptr)

/// Allocate space for one primitive type, external type or non-virtual type with constructor parameters
#	define ECHO_NEW_T(T) new (::Echo::CategorisedAllocPolicy::allocateBytes(sizeof(T), __FILE__, __LINE__, __FUNCTION__)) T
/// Allocate a block of memory for 'count' primitive types - do not use for classes that inherit from AllocatedObject
#	define ECHO_NEW_ARRAY_T(T, count) ::Echo::EchoConstruct(static_cast<T*>(::Echo::CategorisedAllocPolicy::allocateBytes(sizeof(T)*(count), __FILE__, __LINE__, __FUNCTION__)), count) 
/// Free the memory allocated with ECHO_NEW_T.
#	define ECHO_DELETE_T(ptr, T) if(ptr){(ptr)->~T(); ::Echo::CategorisedAllocPolicy::deallocateBytes((void*)ptr);}
/// Free the memory allocated with LOR_NEW_ARRAY_T.
#	define ECHO_DELETE_ARRAY_T(ptr, T, count) if(ptr){for (size_t b = 0; b < count; ++b) { (ptr)[b].~T();} ::Echo::CategorisedAllocPolicy::deallocateBytes((void*)ptr);}

// aligned allocation
/// Allocate a block of raw memory aligned to SIMD boundaries.
#	define ECHO_MALLOC_SIMD(bytes) ::Echo::CategorisedAlignAllocPolicy::allocateBytes(bytes, __FILE__, __LINE__, __FUNCTION__)
/// Allocate a block of raw memory aligned to user defined boundaries.
#	define ECHO_MALLOC_ALIGN(bytes, align) ::Echo::CategorisedAlignAllocPolicy<align>::allocateBytes(bytes, __FILE__, __LINE__, __FUNCTION__)
/// Allocate a block of memory for a primitive type aligned to SIMD boundaries.
#	define ECHO_ALLOC_T_SIMD(T, count) static_cast<T*>(::Echo::CategorisedAlignAllocPolicy::allocateBytes(sizeof(T)*(count), __FILE__, __LINE__, __FUNCTION__))
/// Allocate a block of memory for a primitive type aligned to user defined boundaries.
#	define ECHO_ALLOC_T_ALIGN(T, count, align) static_cast<T*>(::Echo::CategorisedAlignAllocPolicy<align>::allocateBytes(sizeof(T)*(count), __FILE__, __LINE__, __FUNCTION__))
/// Free the memory allocated with either ECHO_MALLOC_SIMD or ECHO_ALLOC_T_SIMD.
#	define ECHO_FREE_SIMD(ptr) ::Echo::CategorisedAlignAllocPolicy::deallocateBytes(ptr)
/// Free the memory allocated with either ECHO_MALLOC_ALIGN or ECHO_ALLOC_T_ALIGN.
#	define ECHO_FREE_ALIGN(ptr, align) ::Echo::CategorisedAlignAllocPolicy<align>::deallocateBytes(ptr)

/// Allocate space for one primitive type, external type or non-virtual type aligned to SIMD boundaries
#	define ECHO_NEW_T_SIMD(T) new (::Echo::CategorisedAlignAllocPolicy::allocateBytes(sizeof(T), __FILE__, __LINE__, __FUNCTION__)) T
/// Allocate a block of memory for 'count' primitive types aligned to SIMD boundaries - do not use for classes that inherit from AllocatedObject
#	define ECHO_NEW_ARRAY_T_SIMD(T, count) ::Echo::EchoConstruct(static_cast<T*>(::Echo::CategorisedAlignAllocPolicy::allocateBytes(sizeof(T)*(count), __FILE__, __LINE__, __FUNCTION__)), count) 
/// Free the memory allocated with LOR_NEW_T_SIMD.
#	define ECHO_DELETE_T_SIMD(ptr, T) if(ptr){(ptr)->~T(); ::Echo::CategorisedAlignAllocPolicy::deallocateBytes(ptr);}
/// Free the memory allocated with ECHO_NEW_ARRAY_T_SIMD.
#	define ECHO_DELETE_ARRAY_T_SIMD(ptr, T, count) if(ptr){for (size_t b = 0; b < count; ++b) { (ptr)[b].~T();} ::Echo::CategorisedAlignAllocPolicy::deallocateBytes(ptr);}
/// Allocate space for one primitive type, external type or non-virtual type aligned to user defined boundaries
#	define ECHO_NEW_T_ALIGN(T, align) new (::Echo::CategorisedAlignAllocPolicy<align>::allocateBytes(sizeof(T), __FILE__, __LINE__, __FUNCTION__)) T
/// Allocate a block of memory for 'count' primitive types aligned to user defined boundaries - do not use for classes that inherit from AllocatedObject
#	define ECHO_NEW_ARRAY_T_ALIGN(T, count, align) ::Echo::EchoConstruct(static_cast<T*>(::Echo::CategorisedAlignAllocPolicy<align>::allocateBytes(sizeof(T)*(count), __FILE__, __LINE__, __FUNCTION__)), count) 
/// Free the memory allocated with ECHO_NEW_T_ALIGN.
#	define ECHO_DELETE_T_ALIGN(ptr, T, align) if(ptr){(ptr)->~T(); ::Echo::CategorisedAlignAllocPolicy<align>::deallocateBytes(ptr);}
/// Free the memory allocated with ECHO_NEW_ARRAY_T_ALIGN.
#	define ECHO_DELETE_ARRAY_T_ALIGN(ptr, T, count, align) if(ptr){for (size_t _b = 0; _b < count; ++_b) { (ptr)[_b].~T();} ::Echo::CategorisedAlignAllocPolicy<align>::deallocateBytes(ptr);}


#else // else - #if defined(ECHO_MEMORY_TRACKER)

/// Allocate a block of raw memory.
#	define ECHO_MALLOC(bytes) ::Echo::NoMemTraceAllocPolicy::allocateBytes(bytes)
/// Allocate a block of memory for a primitive type.
#	define ECHO_ALLOC_T(T, count) static_cast<T*>(::Echo::NoMemTraceAllocPolicy::allocateBytes(sizeof(T)*(count)))

#if (ECHO_MEMORY_ALLOCATOR == ECHO_MEMORY_ALLOCATOR_DEFAULT || ECHO_MEMORY_ALLOCATOR == ECHO_MEMORY_ALLOCATOR_BINNED)
#   define ECHO_REALLOC(ptr, newBytes) ::Echo::NoMemTraceAllocPolicy::reallocBytes(ptr,newBytes)
#endif

/// Free the memory allocated with ECHO_MALLOC or ECHO_ALLOC_T. Category is required to be restated to ensure the matching policy is used
#	define ECHO_FREE(ptr) ::Echo::NoMemTraceAllocPolicy::deallocateBytes((void*)ptr)

/// Allocate space for one primitive type, external type or non-virtual type with constructor parameters
#	define ECHO_NEW_T(T) new (::Echo::NoMemTraceAllocPolicy::allocateBytes(sizeof(T))) T
/// Allocate a block of memory for 'count' primitive types - do not use for classes that inherit from AllocatedObject
#	define ECHO_NEW_ARRAY_T(T, count) ::Echo::EchoConstruct(static_cast<T*>(::Echo::NoMemTraceAllocPolicy::allocateBytes(sizeof(T)*(count))), count) 
/// Free the memory allocated with ECHO_NEW_T.
#	define ECHO_DELETE_T(ptr, T) if(ptr){(ptr)->~T(); ::Echo::NoMemTraceAllocPolicy::deallocateBytes((void*)ptr);}
/// Free the memory allocated with ECHO_NEW_ARRAY_T.
#	define ECHO_DELETE_ARRAY_T(ptr, T, count) if(ptr){for (size_t b = 0; b < count; ++b) { (ptr)[b].~T();} ::Echo::NoMemTraceAllocPolicy::deallocateBytes((void*)ptr);}

// aligned allocation
/// Allocate a block of raw memory aligned to SIMD boundaries.
#	define ECHO_MALLOC_SIMD(bytes) ::Echo::AlignNoMemTraceAllocPolicy::allocateBytes(bytes)
/// Allocate a block of raw memory aligned to user defined boundaries.
#	define ECHO_MALLOC_ALIGN(bytes, align) ::Echo::AlignNoMemTraceAllocPolicy<align>::allocateBytes(bytes)
/// Allocate a block of memory for a primitive type aligned to SIMD boundaries.
#	define ECHO_ALLOC_T_SIMD(T, count) static_cast<T*>(::Echo::AlignNoMemTraceAllocPolicy::allocateBytes(sizeof(T)*(count)))
/// Allocate a block of memory for a primitive type aligned to user defined boundaries.
#	define ECHO_ALLOC_T_ALIGN(T, count, align) static_cast<T*>(::Echo::AlignNoMemTraceAllocPolicy<align>::allocateBytes(sizeof(T)*(count)))
/// Free the memory allocated with either ECHO_MALLOC_SIMD or ECHO_ALLOC_T_SIMD.
#	define ECHO_FREE_SIMD(ptr) ::Echo::AlignNoMemTraceAllocPolicy::deallocateBytes((void*)ptr)
/// Free the memory allocated with either ECHO_MALLOC_ALIGN or ECHO_ALLOC_T_ALIGN.
#	define ECHO_FREE_ALIGN(ptr, align) ::Echo::AlignNoMemTraceAllocPolicy<align>::deallocateBytes((void*)ptr)

/// Allocate space for one primitive type, external type or non-virtual type aligned to SIMD boundaries
#	define ECHO_NEW_T_SIMD(T) new (::Echo::AlignNoMemTraceAllocPolicy::allocateBytes(sizeof(T))) T
/// Allocate a block of memory for 'count' primitive types aligned to SIMD boundaries - do not use for classes that inherit from AllocatedObject
#	define ECHO_NEW_ARRAY_T_SIMD(T, count) ::Echo::EchoConstruct(static_cast<T*>(::Echo::AlignNoMemTraceAllocPolicy::allocateBytes(sizeof(T)*(count))), count) 
/// Free the memory allocated with ECHO_NEW_T_SIMD.
#	define ECHO_DELETE_T_SIMD(ptr, T) if(ptr){(ptr)->~T(); ::Echo::AlignNoMemTraceAllocPolicy::deallocateBytes((void*)ptr);}
/// Free the memory allocated with ECHO_NEW_ARRAY_T_SIMD.
#	define ECHO_DELETE_ARRAY_T_SIMD(ptr, T, count) if(ptr){for (size_t b = 0; b < count; ++b) { (ptr)[b].~T();} ::Echo::AlignNoMemTraceAllocPolicy::deallocateBytes((void*)ptr);}
/// Allocate space for one primitive type, external type or non-virtual type aligned to user defined boundaries
#	define ECHO_NEW_T_ALIGN(T, align) new (::Echo::AlignNoMemTraceAllocPolicy<align>::allocateBytes(sizeof(T))) T
/// Allocate a block of memory for 'count' primitive types aligned to user defined boundaries - do not use for classes that inherit from AllocatedObject
#	define ECHO_NEW_ARRAY_T_ALIGN(T, count, align) ::Echo::EchoConstruct(static_cast<T*>(::Echo::AlignNoMemTraceAllocPolicy<align>::allocateBytes(sizeof(T)*(count))), count) 
/// Free the memory allocated with ECHO_NEW_T_ALIGN.
#	define ECHO_DELETE_T_ALIGN(ptr, T, align) if(ptr){(ptr)->~T(); ::Echo::AlignNoMemTraceAllocPolicy<align>::deallocateBytes((void*)ptr);}
/// Free the memory allocated with ECHO_NEW_ARRAY_T_ALIGN.
#	define ECHO_DELETE_ARRAY_T_ALIGN(ptr, T, count, align) if(ptr){for (size_t _b = 0; _b < count; ++_b) { (ptr)[_b].~T();} ::Echo::AlignNoMemTraceAllocPolicy<align>::deallocateBytes((void*)ptr);}


#endif // end - #ifdef ECHO_RUNMODE_DEBUG

#include <deque>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <string>

#if (ECHO_STL_MEMORY_ALLOCATOR == ECHO_STL_MEMORY_ALLOCATOR_CUSTOM)
#else
	#include <sstream>
#endif

namespace Echo
{

	// for stl containers.
	template <typename T, typename A = SA<T, STLAP> >
	struct deque
	{
#if (ECHO_STL_MEMORY_ALLOCATOR == ECHO_STL_MEMORY_ALLOCATOR_CUSTOM)
		typedef typename std::deque<T, A> type;
		typedef typename std::deque<T, A>::iterator iterator;
		typedef typename std::deque<T, A>::const_iterator const_iterator;
#else
		typedef typename std::deque<T> type;
		typedef typename std::deque<T>::iterator iterator;
		typedef typename std::deque<T>::const_iterator const_iterator;
#endif
	};

	template <typename T, typename A = SA<T, STLAP> >
	struct vector
	{
#if (ECHO_STL_MEMORY_ALLOCATOR == ECHO_STL_MEMORY_ALLOCATOR_CUSTOM)
		typedef typename std::vector<T, A> type;
		typedef typename std::vector<T, A>::iterator iterator;
		typedef typename std::vector<T, A>::const_iterator const_iterator;
#else
		typedef typename std::vector<T> type;
		typedef typename std::vector<T>::iterator iterator;
		typedef typename std::vector<T>::const_iterator const_iterator;
#endif
	};

	template <typename T, typename A = SA<T, STLAP> >
	struct list
	{
#if (ECHO_STL_MEMORY_ALLOCATOR == ECHO_STL_MEMORY_ALLOCATOR_CUSTOM)
		typedef typename std::list<T, A> type;
		typedef typename std::list<T, A>::iterator iterator;
		typedef typename std::list<T, A>::const_iterator const_iterator;
#else
		typedef typename std::list<T> type;
		typedef typename std::list<T>::iterator iterator;
		typedef typename std::list<T>::const_iterator const_iterator;
#endif
	};

	template <typename T, typename P = std::less<T>, typename A = SA<T, STLAP> >
	struct set
	{
#if (ECHO_STL_MEMORY_ALLOCATOR == ECHO_STL_MEMORY_ALLOCATOR_CUSTOM)
		typedef typename std::set<T, P, A> type;
		typedef typename std::set<T, P, A>::iterator iterator;
		typedef typename std::set<T, P, A>::const_iterator const_iterator;
#else
		typedef typename std::set<T, P> type;
		typedef typename std::set<T, P>::iterator iterator;
		typedef typename std::set<T, P>::const_iterator const_iterator;
#endif
	};

	template <typename T, typename P = std::less<T>, typename A = SA<T, STLAP> >
	struct multiset
	{
#if (ECHO_STL_MEMORY_ALLOCATOR == ECHO_STL_MEMORY_ALLOCATOR_CUSTOM)
		typedef typename std::multiset<T, P, A> type;
		typedef typename std::multiset<T, P, A>::iterator iterator;
		typedef typename std::multiset<T, P, A>::const_iterator const_iterator;
#else
		typedef typename std::multiset<T, P> type;
		typedef typename std::multiset<T, P>::iterator iterator;
		typedef typename std::multiset<T, P>::const_iterator const_iterator;
#endif
	};

	template <typename K, typename V, typename P = std::less<K>, typename A = SA<std::pair<const K, V>, STLAP> >
	struct map
	{
#if (ECHO_STL_MEMORY_ALLOCATOR == ECHO_STL_MEMORY_ALLOCATOR_CUSTOM)
		typedef typename std::map<K, V, P, A> type;
		typedef typename std::map<K, V, P, A>::iterator iterator;
		typedef typename std::map<K, V, P, A>::const_iterator const_iterator;
#else
		typedef typename std::map<K, V, P> type;
		typedef typename std::map<K, V, P>::iterator iterator;
		typedef typename std::map<K, V, P>::const_iterator const_iterator;
#endif
	};

	template <typename K, typename V, typename P = std::less<K>, typename A = SA<std::pair<const K, V>, STLAP> >
	struct multimap
	{
#if (ECHO_STL_MEMORY_ALLOCATOR == ECHO_STL_MEMORY_ALLOCATOR_CUSTOM)
		typedef typename std::multimap<K, V, P, A> type;
		typedef typename std::multimap<K, V, P, A>::iterator iterator;
		typedef typename std::multimap<K, V, P, A>::const_iterator const_iterator;
#else
		typedef typename std::multimap<K, V, P> type;
		typedef typename std::multimap<K, V, P>::iterator iterator;
		typedef typename std::multimap<K, V, P>::const_iterator const_iterator;
#endif
	};

#define StdStringT(T) std::basic_string<T, std::char_traits<T>, std::allocator<T> >	
#define CustomMemoryStringT(T) std::basic_string<T, std::char_traits<T>, SA<T, STLAP> >

	template<typename T>
	bool operator <(const CustomMemoryStringT(T)& l,const StdStringT(T)& o)
	{
		return l.compare(0,l.length(),o.c_str(),o.length())<0;
	}
	template<typename T>
	bool operator <(const StdStringT(T)& l,const CustomMemoryStringT(T)& o)
	{
		return l.compare(0,l.length(),o.c_str(),o.length())<0;
	}
	template<typename T>
	bool operator <=(const CustomMemoryStringT(T)& l,const StdStringT(T)& o)
	{
		return l.compare(0,l.length(),o.c_str(),o.length())<=0;
	}
	template<typename T>
	bool operator <=(const StdStringT(T)& l,const CustomMemoryStringT(T)& o)
	{
		return l.compare(0,l.length(),o.c_str(),o.length())<=0;
	}
	template<typename T>
	bool operator >(const CustomMemoryStringT(T)& l,const StdStringT(T)& o)
	{
		return l.compare(0,l.length(),o.c_str(),o.length())>0;
	}
	template<typename T>
	bool operator >(const StdStringT(T)& l,const CustomMemoryStringT(T)& o)
	{
		return l.compare(0,l.length(),o.c_str(),o.length())>0;
	}
	template<typename T>
	bool operator >=(const CustomMemoryStringT(T)& l,const StdStringT(T)& o)
	{
		return l.compare(0,l.length(),o.c_str(),o.length())>=0;
	}
	template<typename T>
	bool operator >=(const StdStringT(T)& l,const CustomMemoryStringT(T)& o)
	{
		return l.compare(0,l.length(),o.c_str(),o.length())>=0;
	}

	template<typename T>
	bool operator ==(const CustomMemoryStringT(T)& l,const StdStringT(T)& o)
	{
		return l.compare(0,l.length(),o.c_str(),o.length())==0;
	}
	template<typename T>
	bool operator ==(const StdStringT(T)& l,const CustomMemoryStringT(T)& o)
	{
		return l.compare(0,l.length(),o.c_str(),o.length())==0;
	}

	template<typename T>
	bool operator !=(const CustomMemoryStringT(T)& l,const StdStringT(T)& o)
	{
		return l.compare(0,l.length(),o.c_str(),o.length())!=0;
	}
	template<typename T>
	bool operator !=(const StdStringT(T)& l,const CustomMemoryStringT(T)& o)
	{
		return l.compare(0,l.length(),o.c_str(),o.length())!=0;
	}

	template<typename T>
	CustomMemoryStringT(T) operator +=(const CustomMemoryStringT(T)& l,const StdStringT(T)& o)
	{
		return CustomMemoryStringT(T)(l)+=o.c_str();
	}
	template<typename T>
	CustomMemoryStringT(T) operator +=(const StdStringT(T)& l,const CustomMemoryStringT(T)& o)
	{
		return CustomMemoryStringT(T)(l.c_str())+=o.c_str();
	}

	template<typename T>
	CustomMemoryStringT(T) operator +(const CustomMemoryStringT(T)& l,const StdStringT(T)& o)
	{
		return CustomMemoryStringT(T)(l)+=o.c_str();
	}

	template<typename T>
	CustomMemoryStringT(T) operator +(const StdStringT(T)& l,const CustomMemoryStringT(T)& o)
	{
		return CustomMemoryStringT(T)(l.c_str())+=o.c_str();
	}

	template<typename T>
	CustomMemoryStringT(T) operator +(const T* l,const CustomMemoryStringT(T)& o)
	{
		return CustomMemoryStringT(T)(l)+=o;
	}

#undef StdStringT
#undef CustomMemoryStringT

	typedef ::std::fstream			FStream;
	typedef ::std::ifstream			IFStream;
	typedef ::std::ofstream			OFStream;

	typedef ui8		utf8;
	typedef ui32	utf32;

#if (ECHO_STL_MEMORY_ALLOCATOR == ECHO_STL_MEMORY_ALLOCATOR_CUSTOM)

	typedef std::basic_string<char, std::char_traits<char>, SA<char, STLAP > >	                String;
	typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, SA<wchar_t, STLAP > >	        WString;
	typedef std::basic_stringstream<char, std::char_traits<char>, SA<char, STLAP > >            StringStream;
	typedef std::basic_stringstream<wchar_t, std::char_traits<wchar_t>, SA<wchar_t, STLAP > >   WStringStream;

	typedef std::basic_istringstream<char, std::char_traits<char>, SA<char, STLAP > >           IStringStream;
	typedef std::basic_ostringstream<char, std::char_traits<char>, SA<char, STLAP > >           OStringStream;

	typedef std::basic_string<utf32, std::char_traits<utf32>, SA<utf32, STLAP> >	UTF32String;

#else
	typedef std::string			String;
	typedef std::wstring		WString;
	typedef std::stringstream	StringStream;
	typedef std::wstringstream  WStringStream;
	typedef std::istringstream  IStringStream;
	typedef std::ostringstream  OStringStream;
	typedef std::basic_string<utf32, std::char_traits<utf32> >	UTF32String;

#endif

	typedef String UTF8String;

	typedef vector<String>::type		StringArray;
	typedef set<String>::type			StringSet;
}

// Global Interface Definitions
#define EchoNew(T)								ECHO_NEW_T(T)
#define EchoNewArray(T, count)					ECHO_NEW_ARRAY_T(T, count)
#define EchoSafeDelete(ptr, T)					{ if(ptr) { ECHO_DELETE_T(ptr, T); (ptr) = NULL; } }
#define EchoSafeDeleteArray(ptr, T, count)		{ if(ptr) { ECHO_DELETE_ARRAY_T(ptr, T, count); (ptr) = NULL; } }
#define EchoSafeDeleteContainer(container, T)	{ for (auto& element : container){ ECHO_DELETE_T(element, T); } container.clear();	}

#define EchoMalloc(bytes)						ECHO_MALLOC(bytes)
#define EchoAlloc(T, count)						ECHO_ALLOC_T(T, count)
#define EchoSafeFree(ptr)						{ if(ptr) { ECHO_FREE(ptr); (ptr) = NULL; } }
#define EchoSafeFreeVector(v)					{ for ( size_t i=0; i<v.size(); i++) EchoSafeFree( v[i]); v.clear();}

#define EchoSafeRelease(ptr)					{ if(ptr) { ptr->release(); ptr = NULL; } }

#endif
