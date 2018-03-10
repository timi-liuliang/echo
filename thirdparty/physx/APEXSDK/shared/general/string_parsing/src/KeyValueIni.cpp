/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "KeyValueIni.h"
#include "PsShare.h"
#include "PsString.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <vector>

#include "foundation/PxAssert.h"

#define PX_NEW new
#define PX_FREE(x) ::free(x)
#define PX_ALLOC(x,y) ::malloc(x)

#pragma warning(disable:4100 4267)


namespace physx
{

	class UserAllocated
	{
	public:
	};

	namespace general_string_parsing2
	{


#pragma warning(disable:4996) // Disabling stupid .NET deprecated warning.

#define DEFAULT_BUFFER_SIZE 1000000
#define DEFAULT_GROW_SIZE   2000000


class KV_FILE_INTERFACE : public physx::UserAllocated
{
public:
	KV_FILE_INTERFACE(const char *fname,const char *spec,void *mem,size_t len)
	{
		mMyAlloc = false;
		mRead = true; // default is read access.
		mFph = 0;
		mData = (char *) mem;
		mLen  = len;
		mLoc  = 0;

		if ( spec && physx::string::stricmp(spec,"wmem") == 0 )
		{
			mRead = false;
			if ( mem == 0 || len == 0 )
			{
				mData = (char *)PX_ALLOC(DEFAULT_BUFFER_SIZE, PX_DEBUG_EXP("KeyValueInterface"));
				mLen  = DEFAULT_BUFFER_SIZE;
				mMyAlloc = true;
			}
		}

		if ( mData == 0 )
		{
			mFph = fopen(fname,spec);
		}

  	strncpy(mName,fname,512);
	}

  ~KV_FILE_INTERFACE(void)
  {
  	if ( mMyAlloc )
  	{
  		PX_FREE(mData);
  	}
  	if ( mFph )
  	{
  		fclose(mFph);
  	}
  }

  size_t read(char *data,size_t size)
  {
  	size_t ret = 0;
  	if ( (mLoc+size) <= mLen )
  	{
  		memcpy(data, &mData[mLoc], size );
  		mLoc+=size;
  		ret = 1;
  	}
    return ret;
  }

  size_t write(const char *data,size_t size)
  {
  	size_t ret = 0;

		if ( (mLoc+size) >= mLen && mMyAlloc ) // grow it
		{
			size_t newLen = mLen+DEFAULT_GROW_SIZE;
			if ( size > newLen ) newLen = size+DEFAULT_GROW_SIZE;

			char *data = (char *)PX_ALLOC(newLen, PX_DEBUG_EXP("KeyValueInterface"));
			memcpy(data,mData,mLoc);
      PX_FREE(mData);
			mData = data;
			mLen  = newLen;
		}

  	if ( (mLoc+size) <= mLen )
  	{
  		memcpy(&mData[mLoc],data,size);
  		mLoc+=size;
  		ret = 1;
  	}
  	return ret;
  }

	size_t read(void *buffer,size_t size,size_t count)
	{
		size_t ret = 0;
		if ( mFph )
		{
			ret = fread(buffer,size,count,mFph);
		}
		else
		{
			char *data = (char *)buffer;
			for (size_t i=0; i<count; i++)
			{
				if ( (mLoc+size) <= mLen )
				{
					read(data,size);
					data+=size;
					ret++;
				}
				else
				{
					break;
				}
			}
		}
		return ret;
	}

  size_t write(const void *buffer,size_t size,size_t count)
  {
  	size_t ret = 0;

  	if ( mFph )
  	{
  		ret = fwrite(buffer,size,count,mFph);
  	}
  	else
  	{
  		const char *data = (const char *)buffer;

  		for (size_t i=0; i<count; i++)
  		{
    		if ( write(data,size) )
				{
    			data+=size;
    			ret++;
    		}
    		else
    		{
    			break;
    		}
  		}
  	}
  	return ret;
  }

  size_t writeString(const char *str)
  {
  	size_t ret = 0;
  	if ( str )
  	{
  		size_t len = strlen(str);
  		ret = write(str,len, 1 );
  	}
  	return ret;
  }


  size_t  flush(void)
  {
  	size_t ret = 0;
  	if ( mFph )
  	{
  		ret = (size_t)fflush(mFph);
  	}
  	return ret;
  }


  size_t seek(size_t loc,size_t mode)
  {
  	size_t ret = 0;
  	if ( mFph )
  	{
  		ret = (size_t)fseek(mFph,(long)loc,(int)mode);
  	}
  	else
  	{
  		if ( mode == SEEK_SET )
  		{
  			if ( loc <= mLen )
  			{
  				mLoc = loc;
  				ret = 1;
  			}
  		}
  		else if ( mode == SEEK_END )
  		{
  			mLoc = mLen;
  		}
  		else
  		{
  			PX_ALWAYS_ASSERT();
  		}
  	}
  	return ret;
  }

  size_t tell(void)
  {
  	size_t ret = 0;
  	if ( mFph )
  	{
  		ret = (size_t)ftell(mFph);
  	}
  	else
  	{
  		ret = mLoc;
  	}
  	return ret;
  }

  size_t myputc(char c)
  {
  	size_t ret = 0;
  	if ( mFph )
  	{
  		ret = (size_t)fputc(c,mFph);
  	}
  	else
  	{
  		ret = write(&c,1);
  	}
  	return ret;
  }

  size_t eof(void)
  {
  	size_t ret = 0;
  	if ( mFph )
  	{
  		ret = (size_t)feof(mFph);
  	}
  	else
  	{
  		if ( mLoc >= mLen )
  			ret = 1;
  	}
  	return ret;
  }

  size_t  error(void)
  {
  	size_t ret = 0;
  	if ( mFph )
  	{
  		ret = (size_t)ferror(mFph);
  	}
  	return ret;
  }


  FILE 	*mFph;
  char  *mData;
  size_t    mLen;
  size_t    mLoc;
  bool   mRead;
	char   mName[512];
	bool   mMyAlloc;

};


KV_FILE_INTERFACE * kvfi_fopen(const char *fname,const char *spec,void *mem=0,size_t len=0)
{
	KV_FILE_INTERFACE *ret = 0;

	ret = PX_NEW(KV_FILE_INTERFACE)(fname,spec,mem,len);

	if ( mem == 0 && ret->mData == 0)
  {
  	if ( ret->mFph == 0 )
  	{
      delete ret;
  		ret = 0;
  	}
  }

	return ret;
}

void       kvfi_fclose(KV_FILE_INTERFACE *file)
{
  delete file;
}

size_t        kvfi_fread(void *buffer,size_t size,size_t count,KV_FILE_INTERFACE *fph)
{
	size_t ret = 0;
	if ( fph )
	{
		ret = fph->read(buffer,size,count);
	}
	return ret;
}

size_t        kvfi_fwrite(const void *buffer,size_t size,size_t count,KV_FILE_INTERFACE *fph)
{
	size_t ret = 0;
	if ( fph )
	{
		ret = fph->write(buffer,size,count);
	}
	return ret;
}

size_t        kvfi_fprintf(KV_FILE_INTERFACE *fph,const char *fmt,...)
{
	size_t ret = 0;

	char buffer[2048];
	va_list arg;
	va_start( arg, fmt );
	buffer[2047] = 0;
	physx::string::_vsnprintf(buffer,2047, fmt, arg);
	va_end(arg);

	if ( fph )
	{
		ret = fph->writeString(buffer);
	}

	return ret;
}


size_t        kvfi_fflush(KV_FILE_INTERFACE *fph)
{
	size_t ret = 0;
	if ( fph )
	{
		ret = fph->flush();
	}
	return ret;
}


size_t        kvfi_fseek(KV_FILE_INTERFACE *fph,size_t loc,size_t mode)
{
	size_t ret = 0;
	if ( fph )
	{
		ret = fph->seek(loc,mode);
	}
	return ret;
}

size_t        kvfi_ftell(KV_FILE_INTERFACE *fph)
{
	size_t ret = 0;
	if ( fph )
	{
		ret = fph->tell();
	}
	return ret;
}

size_t        kvfi_fputc(char c,KV_FILE_INTERFACE *fph)
{
	size_t ret = 0;
	if ( fph )
	{
		ret = fph->myputc(c);
	}
	return ret;
}

size_t        kvfi_fputs(const char *str,KV_FILE_INTERFACE *fph)
{
	size_t ret = 0;
	if ( fph )
	{
		ret = fph->writeString(str);
	}
	return ret;
}

size_t        kvfi_feof(KV_FILE_INTERFACE *fph)
{
	size_t ret = 0;
	if ( fph )
	{
		ret = fph->eof();
	}
	return ret;
}

size_t        kvfi_ferror(KV_FILE_INTERFACE *fph)
{
	size_t ret = 0;
	if ( fph )
	{
		ret = fph->error();
	}
	return ret;
}

void *     kvfi_getMemBuffer(KV_FILE_INTERFACE *fph,size_t &outputLength)
{
	outputLength = 0;
	void * ret = 0;
	if ( fph )
	{
		ret = fph->mData;
		outputLength = fph->mLoc;
	}
	return ret;
}


/*******************************************************************/
/******************** InParser.h  ********************************/
/*******************************************************************/
class KvInPlaceParserInterface
{
public:
	virtual PxI32 ParseLine(PxI32 lineno,PxI32 argc,const char **argv) =0;  // return TRUE to continue parsing, return FALSE to abort parsing process
};

enum SeparatorType
{
	ST_DATA,        // is data
	ST_HARD,        // is a hard separator
	ST_SOFT,        // is a soft separator
	ST_EOS,          // is a comment symbol, and everything past this character should be ignored
  ST_COMMENT
};

class KvInPlaceParser
{
public:
	KvInPlaceParser(void)
	{
		Init();
	}

	KvInPlaceParser(char *data,PxI32 len)
	{
		Init();
		SetSourceData(data,len);
	}

	KvInPlaceParser(const char *fname)
	{
		Init();
		SetFile(fname);
	}

	~KvInPlaceParser(void);

	void Init(void)
	{
		mQuoteChar = 34;
		mData = 0;
		mLen  = 0;
		mMyAlloc = false;
		for (PxI32 i=0; i<256; i++)
		{
			mHard[i] = ST_DATA;
			mHardString[i*2] = (char)i;
			mHardString[i*2+1] = (char)0;
		}
		mHard[0]  = ST_EOS;
		mHard[32] = ST_SOFT;
		mHard[9]  = ST_SOFT;
		mHard[13] = ST_SOFT;
		mHard[10] = ST_SOFT;
	}

	void SetFile(const char *fname); // use this file as source data to parse.

	void SetSourceData(char *data,PxI32 len)
	{
		mData = data;
		mLen  = len;
		mMyAlloc = false;
	};

#ifdef _DEBUG
  void validateMem(const char *data,PxI32 len)
  {
    for (PxI32 i=0; i<len; i++)
    {
      PX_ASSERT( data[i] );
    }
  }
#else
  void validateMem(const char *,PxI32 )
  {

  }
#endif

	void SetSourceDataCopy(const char *data,PxI32 len)
	{
    if ( len )
    {

      //validateMem(data,len);

      PX_FREE(mData);
  		mData = (char *)PX_ALLOC((size_t)len+1, PX_DEBUG_EXP("KeyValueInterface"));
      memcpy(mData,data, (size_t)len);
      mData[len] = 0;

      //validateMem(mData,len);
   		mLen  = len;
  		mMyAlloc = true;
    }
	};

	PxI32  Parse(KvInPlaceParserInterface *callback); // returns true if entire file was parsed, false if it aborted for some reason

	PxI32 ProcessLine(PxI32 lineno,char *line,KvInPlaceParserInterface *callback);

	void SetHardSeparator(PxU8 c) // add a hard separator
	{
		mHard[c] = ST_HARD;
	}

	void SetHard(PxU8 c) // add a hard separator
	{
		mHard[c] = ST_HARD;
	}


	void SetCommentSymbol(PxU8 c) // comment character, treated as 'end of string'
	{
		mHard[c] = ST_COMMENT;
	}

	void ClearHardSeparator(PxU8 c)
	{
		mHard[c] = ST_DATA;
	}


	void DefaultSymbols(void); // set up default symbols for hard seperator and comment symbol of the '#' character.

	bool EOS(PxU8 c)
	{
		if ( mHard[c] == ST_EOS || mHard[c] == ST_COMMENT )
		{
			return true;
		}
		return false;
	}

	void SetQuoteChar(char c)
	{
		mQuoteChar = c;
	}


  inline bool IsComment(PxU8 c) const;

private:


	inline char * AddHard(PxI32 &argc,const char **argv,char *foo);
	inline bool   IsHard(PxU8 c);
	inline char * SkipSpaces(char *foo);
	inline bool   IsWhiteSpace(PxU8 c);
	inline bool   IsNonSeparator(PxU8 c); // non seperator,neither hard nor soft

	bool   mMyAlloc; // whether or not *I* allocated the buffer and am responsible for deleting it.
	char  *mData;  // ascii data to parse.
	PxI32    mLen;   // length of data
	SeparatorType  mHard[256];
	char   mHardString[256*2];
	char           mQuoteChar;
};

/*******************************************************************/
/******************** InParser.cpp  ********************************/
/*******************************************************************/
void KvInPlaceParser::SetFile(const char *fname)
{
	if ( mMyAlloc )
	{
		PX_FREE(mData);
	}
	mData = 0;
	mLen  = 0;
	mMyAlloc = false;

	FILE *fph = fopen(fname,"rb");
	if ( fph )
	{
		fseek(fph,0L,SEEK_END);
		mLen = ftell(fph);
		fseek(fph,0L,SEEK_SET);
		if ( mLen )
		{
			mData = (char *) PX_ALLOC(sizeof(char)*(mLen+1), PX_DEBUG_EXP("KeyValueInterface"));
			PxI32 ok = (PxI32)fread(mData, (size_t)mLen, 1, fph);
			if ( !ok )
			{
				PX_FREE(mData);
				mData = 0;
			}
			else
			{
				mData[mLen] = 0; // zero byte terminate end of file marker.
				mMyAlloc = true;
			}
		}
		fclose(fph);
	}
}

KvInPlaceParser::~KvInPlaceParser(void)
{
	if ( mMyAlloc )
	{
		PX_FREE(mData);
	}
}

#define MAXARGS 512

bool KvInPlaceParser::IsHard(PxU8 c)
{
	return mHard[c] == ST_HARD;
}

char * KvInPlaceParser::AddHard(PxI32 &argc,const char **argv,char *foo)
{
	while ( IsHard((PxU8)*foo) )
	{
    PxU8 c = (PxU8)*foo;
		const char *hard = &mHardString[c*2];
		if ( argc < MAXARGS )
		{
			argv[argc++] = hard;
		}
		foo++;
	}
	return foo;
}

bool   KvInPlaceParser::IsWhiteSpace(PxU8 c)
{
	return mHard[c] == ST_SOFT;
}

char * KvInPlaceParser::SkipSpaces(char *foo)
{
	while ( !EOS((PxU8)*foo) && IsWhiteSpace((PxU8)*foo) ) foo++;
	return foo;
}

bool KvInPlaceParser::IsNonSeparator(PxU8 c)
{
	if ( !IsHard(c) && !IsWhiteSpace(c) && c != 0 ) return true;
	return false;
}


bool KvInPlaceParser::IsComment(PxU8 c) const
{
  if ( mHard[c] == ST_COMMENT ) return true;
  return false;
}

PxI32 KvInPlaceParser::ProcessLine(PxI32 lineno,char *line,KvInPlaceParserInterface *callback)
{
	PxI32 ret = 0;

	const char *argv[MAXARGS];
	PxI32 argc = 0;

  char *foo = SkipSpaces(line); // skip leading spaces...

  if ( IsComment((PxU8)*foo) )  // if the leading character is a comment symbol.
    return 0;


  if ( !EOS((PxU8)*foo) )  // if we are not at the end of string then..
  {
    argv[argc++] = foo;  // this is the key
    foo++;

    while ( !EOS((PxU8)*foo) )  // now scan forward until we hit an equal sign.
    {
      if ( *foo == '=' ) // if this is the equal sign then...
      {
        *foo = 0; // stomp a zero byte on the equal sign to terminate the key we should search for trailing spaces too...
        // look for trailing whitespaces and trash them.
        char *scan = foo-1;
        while ( IsWhiteSpace((PxU8)*scan) )
        {
          *scan = 0;
          scan--;
        }

        foo++;
        foo = SkipSpaces(foo);
        if ( !EOS((PxU8)*foo) )
        {
          argv[argc++] = foo;
          foo++;
          while ( !EOS((PxU8)*foo) )
          {
            foo++;
          }
          *foo = 0;
          char *scan = foo-1;
          while ( IsWhiteSpace((PxU8)*scan) )
          {
            *scan = 0;
            scan--;
          }
          break;
        }
      }
      if ( *foo )
        foo++;
    }
  }

  *foo = 0;

	if ( argc )
	{
		ret = callback->ParseLine(lineno, argc, argv );
	}

	return ret;
}

PxI32  KvInPlaceParser::Parse(KvInPlaceParserInterface *callback) // returns true if entire file was parsed, false if it aborted for some reason
{
	PX_ASSERT( callback );
	if ( !mData ) return 0;

	PxI32 ret = 0;

	PxI32 lineno = 0;

	char *foo   = mData;
	char *begin = foo;


	while ( *foo )
	{
		if ( *foo == 10 || *foo == 13 )
		{
			lineno++;
			*foo = 0;

			if ( *begin ) // if there is any data to parse at all...
			{
				PxI32 v = ProcessLine(lineno,begin,callback);
				if ( v ) ret = v;
			}

			foo++;
			if ( *foo == 10 ) foo++; // skip line feed, if it is in the carraige-return line-feed format...
			begin = foo;
		}
		else
		{
			foo++;
		}
	}

	lineno++; // lasst line.

	PxI32 v = ProcessLine(lineno,begin,callback);
	if ( v ) ret = v;
	return ret;
}


void KvInPlaceParser::DefaultSymbols(void)
{
	SetHardSeparator(',');
	SetHardSeparator('(');
	SetHardSeparator(')');
	SetHardSeparator('=');
	SetHardSeparator('[');
	SetHardSeparator(']');
	SetHardSeparator('{');
	SetHardSeparator('}');
	SetCommentSymbol('#');
}



class KeyValue
{
public:
  KeyValue(const char *key,const char *value,PxU32 lineno)
  {
    mKey = key;
    mValue = value;
    mLineNo = lineno;
  }

  const char * getKey(void) const { return mKey; };
  const char * getValue(void) const { return mValue; };
  PxU32 getLineNo(void) const { return mLineNo; };

  void save(KV_FILE_INTERFACE *fph) const
  {
    kvfi_fprintf(fph,"%-30s = %s\r\n", mKey, mValue );
  }

  void setValue(const char *value)
  {
    mValue = value;
  }

private:
  PxU32 mLineNo;
  const char *mKey;
  const char *mValue;
};

typedef std::vector< KeyValue > KeyValueVector;

class KeyValueSection : public UserAllocated
{
public:
  KeyValueSection(const char *section,PxU32 lineno)
  {
    mSection = section;
    mLineNo  = lineno;
  }

  PxU32 getKeyCount(void) const { return mKeys.size(); };
  const char * getSection(void) const { return mSection; };
  PxU32 getLineNo(void) const { return mLineNo; };

  const char * locateValue(const char *key,PxU32 &lineno) const
  {
    const char *ret = 0;

    for (PxU32 i=0; i<mKeys.size(); i++)
    {
      const KeyValue &v = mKeys[i];
	  if ( physx::string::stricmp(key,v.getKey()) == 0 )
      {
        ret = v.getValue();
        lineno = v.getLineNo();
        break;
      }
    }
    return ret;
  }

  const char *getKey(PxU32 index,PxU32 &lineno) const
  {
    const char * ret  = 0;
    if ( index < mKeys.size() )
    {
      const KeyValue &v = mKeys[index];
      ret = v.getKey();
      lineno = v.getLineNo();
    }
    return ret;
  }

  const char *getValue(PxU32 index,PxU32 &lineno) const
  {
    const char * ret  = 0;
    if ( index < mKeys.size() )
    {
      const KeyValue &v = mKeys[index];
      ret = v.getValue();
      lineno = v.getLineNo();
    }
    return ret;
  }

  void addKeyValue(const char *key,const char *value,PxU32 lineno)
  {
    KeyValue kv(key,value,lineno);
    mKeys.push_back(kv);
  }

  void save(KV_FILE_INTERFACE *fph) const
  {
    if ( strcmp(mSection,"@HEADER") == 0 )
    {
    }
    else
    {
      kvfi_fprintf(fph,"\r\n");
      kvfi_fprintf(fph,"\r\n");
      kvfi_fprintf(fph,"[%s]\r\n", mSection );
    }
    for (PxU32 i=0; i<mKeys.size(); i++)
    {
      mKeys[i].save(fph);
    }
  }


  bool  addKeyValue(const char *key,const char *value) // adds a key-value pair.  These pointers *must* be persistent for the lifetime of the INI file!
  {
    bool ret = false;

    for (PxU32 i=0; i<mKeys.size(); i++)
    {
      KeyValue &kv = mKeys[i];
      if ( strcmp(kv.getKey(),key) == 0 )
      {
        kv.setValue(value);
        ret = true;
        break;
      }
    }

    if ( !ret )
    {
      KeyValue kv(key,value,0);
      mKeys.push_back(kv);
      ret = true;
    }

    return ret;
  }

  void reset(void)
  {
    mKeys.clear();
  }

private:
  PxU32 mLineNo;
  const char *mSection;
  KeyValueVector mKeys;
};

typedef std::vector< KeyValueSection *> KeyValueSectionVector;

class KeyValueIni : public KvInPlaceParserInterface, public UserAllocated
{
public:
  KeyValueIni(const char *fname)
  {
    mData.SetFile(fname);
    mData.SetCommentSymbol('#');
    mData.SetCommentSymbol('!');
    mData.SetCommentSymbol(';');
    mData.SetHard('=');
    mCurrentSection = 0;
    KeyValueSection *kvs = PX_NEW(KeyValueSection)("@HEADER",0);
    mSections.push_back(kvs);
    mData.Parse(this);
  }

  KeyValueIni(const char *mem,PxU32 len)
  {
    if ( len )
    {
      mCurrentSection = 0;
      mData.SetSourceDataCopy(mem,(physx::PxI32)len);

      mData.SetCommentSymbol('#');
      mData.SetCommentSymbol('!');
      mData.SetCommentSymbol(';');
      mData.SetHard('=');
      KeyValueSection *kvs = PX_NEW(KeyValueSection)("@HEADER",0);
      mSections.push_back(kvs);
      mData.Parse(this);
    }
  }

  KeyValueIni(void)
  {
    mCurrentSection = 0;
    KeyValueSection *kvs = PX_NEW(KeyValueSection)("@HEADER",0);
    mSections.push_back(kvs);
  }

  virtual ~KeyValueIni(void)
  {
    reset();
  }

  void reset(void)
  {
    KeyValueSectionVector::iterator i;
    for (i=mSections.begin(); i!=mSections.end(); ++i)
    {
      KeyValueSection *kvs = (*i);
      delete kvs;
    }
    mSections.clear();
    mCurrentSection = 0;
  }

  PxU32 getSectionCount(void) const { return mSections.size(); };

	PxI32 ParseLine(PxI32 lineno,PxI32 argc,const char **argv)  // return TRUE to continue parsing, return FALSE to abort parsing process
  {

    if ( argc )
    {
      const char *key = argv[0];
      if ( key[0] == '[' )
      {
        key++;
        char *scan = (char *) key;
        while ( *scan )
        {
          if ( *scan == ']')
          {
            *scan = 0;
            break;
          }
          scan++;
        }
        mCurrentSection = -1;
        for (PxU32 i=0; i<mSections.size(); i++)
        {
          KeyValueSection &kvs = *mSections[i];
		  if ( physx::string::stricmp(kvs.getSection(),key) == 0 )
          {
            mCurrentSection = (PxI32) i;
            break;
          }
        }
        //...
        if ( mCurrentSection < 0 )
        {
          mCurrentSection = (physx::PxI32)mSections.size();
          KeyValueSection *kvs = PX_NEW(KeyValueSection)(key,(physx::PxU32)lineno);
          mSections.push_back(kvs);
        }
      }
      else
      {
        const char *key = argv[0];
        const char *value = 0;
        if ( argc >= 2 )
          value = argv[1];
        mSections[(physx::PxU32)mCurrentSection]->addKeyValue(key,value,(size_t)lineno);
      }
    }

    return 0;
  }

  KeyValueSection * locateSection(const char *section,PxU32 &keys,PxU32 &lineno) const
  {
    KeyValueSection *ret = 0;
    for (PxU32 i=0; i<mSections.size(); i++)
    {
      KeyValueSection *s = mSections[i];
	  if ( physx::string::stricmp(section,s->getSection()) == 0 )
      {
        ret = s;
        lineno = s->getLineNo();
        keys = s->getKeyCount();
        break;
      }
    }
    return ret;
  }

  const KeyValueSection * getSection(PxU32 index,PxU32 &keys,PxU32 &lineno) const
  {
    const KeyValueSection *ret=0;
    if ( index < mSections.size() )
    {
      const KeyValueSection &s = *mSections[index];
      ret = &s;
      lineno = s.getLineNo();
      keys = s.getKeyCount();
    }
    return ret;
  }

  bool save(const char *fname) const
  {
    bool ret = false;
    KV_FILE_INTERFACE *fph = kvfi_fopen(fname,"wb");
    if ( fph )
    {
      for (PxU32 i=0; i<mSections.size(); i++)
      {
        mSections[i]->save(fph);
      }
      kvfi_fclose(fph);
      ret = true;
    }
    return ret;
  }

  void * saveMem(PxU32 &len) const
  {
    void *ret = 0;
    KV_FILE_INTERFACE *fph = kvfi_fopen("mem","wmem");
    if ( fph )
    {
      for (PxU32 i=0; i<mSections.size(); i++)
      {
        mSections[i]->save(fph);
      }

	  size_t tmpLen;
      void *temp = kvfi_getMemBuffer(fph,tmpLen);
	  len = (PxU32)tmpLen;
      if ( temp )
      {
        ret = PX_ALLOC(len, PX_DEBUG_EXP("KeyValueInterface"));
        memcpy(ret,temp,len);
      }

      kvfi_fclose(fph);
    }
    return ret;
  }


  KeyValueSection  *createKeyValueSection(const char *section_name,bool reset)  // creates, or locates and existing section for editing.  If reset it true, will erase previous contents of the section.
  {
    KeyValueSection *ret = 0;

    for (PxU32 i=0; i<mSections.size(); i++)
    {
      KeyValueSection *kvs = mSections[i];
      if ( strcmp(kvs->getSection(),section_name) == 0 )
      {
        ret = kvs;
        if ( reset )
        {
          ret->reset();
        }
        break;
      }
    }
    if ( ret == 0 )
    {
      ret = PX_NEW(KeyValueSection)(section_name,0);
      mSections.push_back(ret);
    }

    return ret;
  }

private:
  PxI32                   mCurrentSection;
  KeyValueSectionVector mSections;
  KvInPlaceParser         mData;
};

KeyValueIni *loadKeyValueIni(const char *fname,PxU32 &sections)
{
  KeyValueIni *ret = 0;

  ret = PX_NEW(KeyValueIni)(fname);
  sections = ret->getSectionCount();
  if ( sections < 2 )
  {
    delete ret;
    ret = 0;
  }

  return ret;
}

KeyValueIni *     loadKeyValueIni(const char *mem,PxU32 len,PxU32 &sections)
{
  KeyValueIni *ret = 0;

  ret = PX_NEW(KeyValueIni)(mem,len);
  sections = ret->getSectionCount();
  if ( sections < 2 )
  {
    delete ret;
    ret = 0;
  }

  return ret;
}

const KeyValueSection * locateSection(const KeyValueIni *ini,const char *section,PxU32 &keys,PxU32 &lineno)
{
  KeyValueSection *ret = 0;

  if ( ini )
  {
    ret = ini->locateSection(section,keys,lineno);
  }

  return ret;
}

const KeyValueSection * getSection(const KeyValueIni *ini,PxU32 index,PxU32 &keycount,PxU32 &lineno)
{
  const KeyValueSection *ret = 0;

  if ( ini )
  {
    ret = ini->getSection(index,keycount,lineno);
  }

  return ret;
}

const char *      locateValue(const KeyValueSection *section,const char *key,PxU32 &lineno)
{
  const char *ret = 0;

  if ( section )
  {
    ret = section->locateValue(key,lineno);
  }

  return ret;
}

const char *      getKey(const KeyValueSection *section,PxU32 keyindex,PxU32 &lineno)
{
  const char *ret = 0;

  if ( section )
  {
    ret = section->getKey(keyindex,lineno);
  }

  return ret;
}

const char *      getValue(const KeyValueSection *section,PxU32 keyindex,PxU32 &lineno)
{
  const char *ret = 0;

  if ( section )
  {
    ret = section->getValue(keyindex,lineno);
  }

  return ret;
}

void              releaseKeyValueIni(const KeyValueIni *ini)
{
  KeyValueIni *k = (KeyValueIni *)ini;
  delete k;
}


const char *    getSectionName(const KeyValueSection *section)
{
  const char *ret = 0;
  if ( section )
  {
    ret = section->getSection();
  }
  return ret;
}


bool  saveKeyValueIni(const KeyValueIni *ini,const char *fname)
{
  bool ret = false;

  if ( ini )
    ret = ini->save(fname);

  return ret;
}

void *  saveKeyValueIniMem(const KeyValueIni *ini,PxU32 &len)
{
  void *ret = 0;

  if ( ini )
    ret = ini->saveMem(len);

  return ret;
}

KeyValueSection  *createKeyValueSection(KeyValueIni *ini,const char *section_name,bool reset)
{
  KeyValueSection *ret = 0;

  if ( ini )
  {
    ret = ini->createKeyValueSection(section_name,reset);
  }
  return ret;
}

bool  addKeyValue(KeyValueSection *section,const char *key,const char *value)
{
  bool ret = false;

  if ( section )
  {
    ret = section->addKeyValue(key,value);
  }

  return ret;
}


KeyValueIni      *createKeyValueIni(void) // create an empty .INI file in memory for editing.
{
  KeyValueIni *ret = PX_NEW(KeyValueIni);
  return ret;
}

bool              releaseIniMem(void *mem)
{
  bool ret = false;
  if ( mem )
  {
    PX_FREE(mem);
    ret = true;
  }
  return ret;
}



}; // end of namespace
};
