/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef STABLE_H
#define STABLE_H

#include "PsString.h"
#include "foundation/PxAssert.h"
#include <string.h>

#include "PsShare.h"
#include "PsUserAllocated.h"
#include "PsHashMap.h"

#if defined(LINUX)
#define stricmp strcasecmp
#endif

#pragma warning(disable:4786)
#pragma warning(disable:4995)
#pragma warning(disable:4996)

namespace physx
{
	namespace general_string_parsing2
	{

class CRC32
{
public:
  CRC32(void)
  {
    const PxU32 QUOTIENT=0x04c11db7;
    for (PxU32 i = 0; i < 256; i++)
    {
      PxU32 crc = i << 24;
      for (PxU32 j = 0; j < 8; j++)
      {
        if (crc & 0x80000000)
          crc = (crc << 1) ^ QUOTIENT;
        else
          crc = crc << 1;
      }
      mCRCTable[i] = myhtonl(crc);
    }
    mIsLittleEndian = !isBigEndian();
  }


  inline PxU32 myhtonl(PxU32 n_ecx)
  {
    PxU32 n_eax = n_ecx;           //mov         eax,ecx
    PxU32 n_edx = n_ecx;           //mov         edx,ecx
    n_edx = n_edx << 16;           //shl         edx,10h
    n_eax = n_eax & 0x0FF00;       //and         eax,0FF00h
    n_eax = n_eax | n_edx;         //or          eax,edx
    n_edx = n_ecx;                 // mov         edx,ecx
    n_edx = n_edx & 0x0FF0000;     //and edx,0FF0000h
    n_ecx = n_ecx >> 16;           //shr         ecx,10h
    n_edx = n_edx | n_ecx;         //or          edx,ecx
    n_eax = n_eax << 8;            //shl         eax,8
    n_edx = n_edx >> 8;            //shr         edx,8
    n_eax|=n_edx;                  //  71AB2BE9  or          eax,edx
    return n_eax;
  }

  inline void getRand(PxU32 &current) const
  {
    current = (current * 214013L + 2531011L) & 0x7fffffff;
  };

  PxU32 crc32(const PxU8 *data, PxU32 len) const
  {
    PxU32 ret;

    ret =  crc32Internal(data,len);

    return ret;
  }

  PxU32 crc32(const char *data) const
  {
    PxU32 len = (PxU32)strlen(data);
    return crc32( (const PxU8 *)data,len);
  }

private:

  bool inline isBigEndian() { int i = 1; return *((char*)&i)==0; }

  PxU32 crc32Internal(const PxU8 *data,PxU32 len) const
  {
    PxU32        dlen = (len/4)*4;
    PxU32        result=len;
    PxU32        *p = (PxU32 *)data;
    PxU32        *e = (PxU32 *)(data + dlen);
    PxU32         current = len;


	if ( dlen >= 4 )
	{
		result = ~*p++;
		result = result ^ len;

		const PxU32 *tmp = (const PxU32 *) data;
		current = *tmp & len;

		if ( mIsLittleEndian )
		{
		  while( p<e )
		  {
			getRand(current);
			result = mCRCTable[result & 0xff] ^ result >> 8;
			result = mCRCTable[result & 0xff] ^ result >> 8;
			result = mCRCTable[result & 0xff] ^ result >> 8;
			result = mCRCTable[result & 0xff] ^ result >> 8;
			result ^= *p++;
			result ^= current;
			current &= result; // feed the result back into the random number seed, this forces the random sequence to drift with the input charcter stream.
		  }
		}
		else
		{
		  while( p<e )
		  {
			getRand(current);
			result = mCRCTable[result >> 24] ^ result << 8;
			result = mCRCTable[result >> 24] ^ result << 8;
			result = mCRCTable[result >> 24] ^ result << 8;
			result = mCRCTable[result >> 24] ^ result << 8;
			result ^= *p++;
			result ^= current;
			current &= result; // feed the result back into the random number seed, this forces the random sequence to drift with the input charcter stream.
		  }
	  }


    }
    PxU32 partial = len&3;
    if ( partial )
    {
      for (PxU32 i=0; i<partial; i++)
      {
          getRand(current);
          PxU8 v = data[dlen+i];
          result = mCRCTable[v]^result;
          result^=current;
          current &= result;
      }
    }
    return ~result;
  }

  bool  mIsLittleEndian;
  PxU32 mCRCTable[256];
};



class SStringHash : public UserAllocated
{
public:
  SStringHash(const char *str)
  {
    mNextHash = 0;
    PxU32 len = (PxU32)strlen(str);
    mString = (char *)PX_ALLOC(len+1, PX_DEBUG_EXP("SStringHash"));
    strcpy(mString,str);
  }

  ~SStringHash(void)
  {
    PX_FREE(mString);
  }

  inline const char * getString(void) const { return mString; };
  inline SStringHash *getNextHash(void) const { return mNextHash; };
  inline void         setNextHash(SStringHash *sh) { mNextHash = sh; };
  inline bool         match(const char *str,bool caseSensitive) const 
  { 
	  return caseSensitive ? (strcmp(mString,str) == 0) : (physx::string::stricmp(mString,str) == 0);
  };

private:
  SStringHash *mNextHash;
  char       *mString;
};



class StringTable : public UserAllocated, public CRC32
{
public:

    typedef HashMap< PxU32, SStringHash *> StringHashMap;

	StringTable(void)
	{
        mCaseSensitive = true;
	};

	~StringTable(void)
	{
    	for (StringHashMap::Iterator i=mStrings.getIterator(); !i.done(); ++i)
        {
            SStringHash *sh = (*i).second;
            while ( sh )
            {
                SStringHash *next = sh->getNextHash();
                delete sh;
                sh = next;
            }
        }
	}

	const char * Get(const char *str,bool &first)
	{
		first = false;
        const char *ret=0;

        PxU32 hash;

        if ( !mCaseSensitive )
        {
            char temp_string[8192];
            strncpy(temp_string,str,8192);
			physx::string::strlwr(temp_string);
            hash = crc32(temp_string);
        }
        else
        {
          hash = crc32(str);
        }

        const StringHashMap::Entry *found = mStrings.find(hash);

        if ( found == NULL )
        {
            SStringHash *sh = PX_NEW(SStringHash)(str);
            mStrings[hash] = sh;
            ret = sh->getString();
			first = true;
        }
        else
        {
            SStringHash *sh = (*found).second;
            while ( sh )
            {
                if ( sh->match(str,mCaseSensitive) )
                {
                    ret = sh->getString();
                    break;
                }
                sh = sh->getNextHash();
            }
            if ( !ret )
            {
                SStringHash *nh = PX_NEW(SStringHash)(str);
                sh = (*found).second;
                nh->setNextHash(sh);
				mStrings.erase(hash); // erase the old hash.
				mStrings[hash] = nh;  // assign the new hash
				ret = sh->getString();
				first = true;
            }
        }
        return ret;
	};


    void setCaseSensitive(bool state)
    {
        mCaseSensitive = state;
    }

private:
    bool                mCaseSensitive;
    StringHashMap       mStrings;
};

class StringIntHash : public UserAllocated
{
public:
  StringIntHash(const char *str,PxU32 id)
  {
    mNextHash = 0;
    mId       = id;
    PxU32 len = (PxU32)strlen(str);
    mString = (char *)PX_ALLOC(len+1, PX_DEBUG_EXP("StringIntHash"));
    strcpy(mString,str);
  }

  ~StringIntHash(void)
  {
    PX_FREE(mString);
  }

  inline const char *    getString(void) const { return mString; };
  inline StringIntHash * getNextHash(void) const { return mNextHash; };
  inline void            setNextHash(StringIntHash *sh) { mNextHash = sh; };

  inline bool         match(const char *str,bool caseSensitive) const
  { 
	  return caseSensitive ? (strcmp(mString,str) == 0) : (physx::string::stricmp(mString,str) == 0);
  };

  inline PxU32           getId(void) const { return mId; };

private:
  StringIntHash *mNextHash;
  PxU32          mId;
  char          *mString;
};


class StringTableInt : public UserAllocated, public CRC32
{
public:

    typedef HashMap< PxU32, StringIntHash *>  StringIntHashMap;
    typedef HashMap< PxU32, StringIntHash * > IntCharHashMap;

	StringTableInt(void)
	{
		mCaseSensitive = false;
  }
  ~StringTableInt(void)
  {
		for (StringIntHashMap::Iterator i=mStrings.getIterator(); !i.done(); ++i)
        {
            StringIntHash *sh = (*i).second;
            while ( sh )
            {
                StringIntHash *next = sh->getNextHash();
                delete sh;
                sh = next;
            }
        }
	}

	bool Get(const char *str,PxU32 &id) const
	{
		id = Get(str);
		return id != 0;
	}

	PxU32 Get(const char *str) const
	{
		PxU32 ret = 0;

        if ( str )
        {
          PxU32 hash;

          if ( !mCaseSensitive )
          {
              char temp_string[8192];
              strncpy(temp_string,str,8192);
			  physx::string::strlwr(temp_string);
              hash = crc32(temp_string);
          }
          else
          {
              hash = crc32(str); // compute the hash value.
          }
          const StringIntHashMap::Entry *found = mStrings.find(hash);
          if ( found != NULL )
          {
              const StringIntHash *sh = (*found).second;
              while ( sh )
              {
                  if ( sh->match(str,mCaseSensitive) )
                  {
                      ret = sh->getId();
                      break;
                  }
                  sh = sh->getNextHash();
              }
          }
        }


		return ret;
	};

	void Add(const char *str,PxU32 id)
	{
        StringIntHash *sh;
        char temp_string[8192];

        PxU32 hash;
        if ( !mCaseSensitive )
        {
            strncpy(temp_string,str,8192);
			physx::string::strlwr(temp_string);
            hash = crc32(temp_string);
        }
        else
        {
          hash = crc32(str);
        }
        const StringIntHashMap::Entry *found = mStrings.find(hash);
        if ( found == NULL )
        {
            sh = PX_NEW(StringIntHash)(str,id);
            mStrings[hash] = sh;
        }
        else
        {
            sh = (*found).second;
            while ( sh )
            {
                if ( sh->match(str,mCaseSensitive) )
                {
                    PX_ALWAYS_ASSERT(); // same string mapped to multiple ids!!
                    break;
                }
                sh = sh->getNextHash();
            }
            if ( !sh )
            {
                StringIntHash *nh = PX_NEW(StringIntHash)(str,id);
                sh = (*found).second;
                nh->setNextHash(sh);
				mStrings.erase(hash);
				mStrings[hash] = nh;
                sh = nh;
            }
        }
        mIds[id] = sh;
	}

	const char * Get(PxU32 id) const
	{
		const char *ret = 0;

        const IntCharHashMap::Entry *found = mIds.find(id);
        if ( found != NULL )
            ret = (*found).second->getString();

		return ret;
	}

	void SetCaseSensitive(bool s)
	{
		mCaseSensitive = s;
	}


    IntCharHashMap      mIds;
private:
    bool                mCaseSensitive;
    StringIntHashMap    mStrings;
};

}; // end of namespace
using namespace general_string_parsing2;
};

#endif // STABLE_H
