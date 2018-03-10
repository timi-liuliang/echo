/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef STRINGDICT_H
#define STRINGDICT_H

#include "PsShare.h"
#include "PsUserAllocated.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "stable.h"

namespace physx
{
	namespace general_string_parsing2
	{

extern const char *emptystring;

class StringRef : public UserAllocated
{
public:
	StringRef(void)
	{
		mString = "";
	}

	inline StringRef(const StringRef &str);

	operator const char *() const
	{
		return mString;
	}

	const char * Get(void) const { return mString; };

	void Set(const char *str)
	{
		mString = str;
	}

	const StringRef &operator= (const StringRef& rhs )
	{
		mString = rhs.Get();
		return *this;
	}

	bool operator== ( const StringRef& rhs ) const
	{
		return rhs.mString == mString;
	}

	bool operator< ( const StringRef& rhs ) const
	{
		return rhs.mString < mString;
	}

	bool operator!= ( const StringRef& rhs ) const
	{
		return rhs.mString != mString;
	}

	bool operator> ( const StringRef& rhs ) const
	{
		return rhs.mString > mString;
	}

	bool operator<= ( const StringRef& rhs ) const
	{
		return rhs.mString <= mString;
	}

	bool operator>= ( const StringRef& rhs ) const
	{
		return rhs.mString >= mString;
	}

	bool SamePrefix(const char *prefix) const
	{
		PxU32 len = (PxU32)strlen(prefix);
		if ( len && strncmp(mString,prefix,len) == 0 ) return true;
		return false;
	}

	bool SameSuffix(const StringRef &suf) const
	{
		const char *source = mString;
		const char *suffix = suf.mString;
		PxU32 len1 = (PxU32)strlen(source);
		PxU32 len2 = (PxU32)strlen(suffix);
		if ( len1 < len2 ) return false;
		const char *compare = &source[(len1-len2)];
		if ( strcmp(compare,suffix) == 0 ) return true;
		return false;
	}

#ifdef PX_X64
  PxU64 getHash(void) const
  {
    return (PxU64) mString;
  }
#else
#pragma warning(push)
#pragma warning(disable:4311)
#pragma warning(disable:4302)
  PxU32 getHash(void) const
  {
    return (PxU32)(size_t)(mString);
  }
#pragma warning(pop)
#endif

private:
	const char *mString; // the actual char ptr
};


class StringDict : public UserAllocated
{
public:
	StringDict(void)
	{
	}

	~StringDict(void)
	{
	}

	StringRef Get(const char *text)
	{
		StringRef ref;
		if ( text )
		{
			if ( text[0] == '\0')
			{
				ref.Set(emptystring);
			}
			else
			{
				bool first;
				const char *foo = mStringTable.Get(text,first);
				ref.Set(foo);
			}
		}
		return ref;
	}

	StringRef Get(const char *text,bool &first)
	{
		StringRef ref;
		const char *foo = mStringTable.Get(text,first);
		ref.Set(foo);
		return ref;
	}

  void setCaseSensitive(bool state)
  {
    mStringTable.setCaseSensitive(state);
  }

private:
	StringTable mStringTable;
};

typedef Array< StringRef  >  StringRefVector;

inline StringRef::StringRef(const StringRef &str)
{
	mString = str.Get();
}

// This is a helper class so you can easily do an alphabetical sort on an STL vector of StringRefs.
// Usage: USER_STL::sort( list.begin(), list.end(), StringSortRef() );
class StringSortRef : public UserAllocated
{
	public:

	 bool operator()(const StringRef &a,const StringRef &b) const
	 {
		 const char *str1 = a.Get();
		 const char *str2 = b.Get();
		 PxI32 r = physx::string::stricmp(str1,str2);
		 return r < 0;
	 }
};

extern StringDict *gStringDict;

};
using namespace general_string_parsing2;
};


static PX_INLINE physx::StringDict * getGlobalStringDict(void)
{
	if ( physx::gStringDict == 0 )
		physx::gStringDict = PX_NEW(physx::StringDict);
	return physx::gStringDict;
}

#define SGET(x) getGlobalStringDict()->Get(x)

#endif // STRINGDICT_H
