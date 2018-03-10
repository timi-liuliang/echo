/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "Ps.h"
#include "foundation/PxAssert.h"
#include "PsFastXml.h"
#include <stdio.h>
#include <string.h>
#include <new>
#include <ctype.h>

namespace
{
#define MIN_CLOSE_COUNT 2
#define DEFAULT_READ_BUFFER_SIZE (16*1024)
#define NUM_ENTITY  5

struct Entity
{
	const char*     str;
	unsigned int	strLength;
	char		    chr;
};

static const Entity entity[ NUM_ENTITY ] =
{
	{ "&lt;",   4, '<' },
	{ "&amp;",  5, '&' },
	{ "&gt;",   4, '>' },
	{ "&quot;", 6, '\"' },
	{ "&apos;", 6, '\'' }
};

class MyFastXml : public physx::shdfnd::FastXml
{
public:
	
	enum CharType
	{
		CT_DATA,
		CT_EOF,
		CT_SOFT,
		CT_END_OF_ELEMENT, // either a forward slash or a greater than symbol
		CT_END_OF_LINE
	};
	
	MyFastXml(Callback *c)
	{
		mStreamFromMemory = true;
		mCallback = c;
		memset(mTypes, CT_DATA, sizeof(mTypes));
		mTypes[0] = CT_EOF;
		mTypes[(unsigned char)(' ')] = mTypes[(unsigned char) ('\t')] = CT_SOFT;
		mTypes[(unsigned char) ('/')] = mTypes[(unsigned char)('>')] = mTypes[(unsigned char) ('?')] = CT_END_OF_ELEMENT;
		mTypes[(unsigned char) ('\n')] = mTypes[(unsigned char) ('\r')] = CT_END_OF_LINE;
		mError = 0;
		mStackIndex = 0;
		mFileBuf = NULL;
		mReadBufferEnd = NULL;
		mReadBuffer = NULL;
		mReadBufferSize = DEFAULT_READ_BUFFER_SIZE;
		mOpenCount = 0;
		mLastReadLoc = 0;
		for (physx::PxU32 i=0; i<(MAX_STACK+1); i++)
		{
			mStack[i] = NULL;
			mStackAllocated[i] = false;
		}
	}

	char *processClose(char c, const char *element, char *scan, physx::PxI32 argc, const char **argv, FastXml::Callback *iface,bool &isError)
	{
		AttributePairs attr(argc, argv);
		isError = true; // by default, if we return null it's due to an error.
		if ( c == '/' || c == '?' )
		{
			char *slash = (char *)strchr(element, c);
			if( slash )
				*slash = 0;

			if( c == '?' && strcmp(element, "xml") == 0 )
			{
				if( !iface->processXmlDeclaration(attr, 0, mLineNo) )
					return NULL;
			}
			else
			{
				if ( !iface->processElement(element, 0, attr, mLineNo) )
				{
					mError = "User aborted the parsing process";
					return NULL;
				}

				pushElement(element);

				const char *close = popElement();

				if( !iface->processClose(close,mStackIndex,isError) )
				{
					return NULL;
				}
			}

			if ( !slash )
				++scan;
		}
		else
		{
			scan = skipNextData(scan);
			char *data = scan; // this is the data portion of the element, only copies memory if we encounter line feeds
			char *dest_data = 0;
			while ( *scan && *scan != '<' )
			{
				if ( getCharType(scan) == CT_END_OF_LINE )
				{
					if ( *scan == '\r' ) mLineNo++;
					dest_data = scan;
					*dest_data++ = ' '; // replace the linefeed with a space...
					scan = skipNextData(scan);
					while ( *scan && *scan != '<' )
					{
						if ( getCharType(scan) == CT_END_OF_LINE )
						{
							if ( *scan == '\r' ) mLineNo++;
							*dest_data++ = ' '; // replace the linefeed with a space...
							scan = skipNextData(scan);
						}
						else
						{
							*dest_data++ = *scan++;
						}
					}
					break;
				} 
				else if ( '&' == *scan )
				{
					dest_data = scan;
					while ( *scan && *scan != '<' )
					{
						if ( '&' == *scan )
						{
							if ( *(scan+1) && *(scan+1) == '#' && *(scan+2) )
							{
								if( *(scan+2) == 'x' )
								{
									// Hexadecimal.
									if ( !*(scan+3) ) 
										break;
									
									char* q = scan+3;
									q = strchr( q, ';' );
									
									if ( !q || !*q ) 
										 PX_ASSERT(0);
									
									--q;
									char ch =  char(*q>'9' ? (tolower(*q)-'a'+10) : *q-'0');
									if(*(--q) != tolower('x'))
										ch |= char(*q>'9' ? (tolower(*q)-'a'+10) : *q-'0') << 4;
									
									*dest_data++ = ch;								
									
								}
								else
								{
									// Decimal.
									if ( !*(scan+2) ) 
										break;
									
									const char* q = scan+2;
									q = strchr( q, ';' );
									
									if ( !q || !*q ) 
										 PX_ASSERT(0);
									
									--q;
									char ch = *q -'0';
									if(*(--q) != '#')
										ch |= (*q-'0') *10;

									*dest_data++ = ch;
								}

								char *start = scan;
								char *end = strchr(start, ';');
								if ( end )
								{
									*end = 0;
									scan = end+1;
								}

								continue;
							}
							
							for(int i=0; i<NUM_ENTITY; ++i )
							{
								if ( strncmp( entity[i].str, scan, entity[i].strLength ) == 0 )
								{
									*dest_data++ = entity[i].chr;
									scan += entity[i].strLength;
									break;
								}
							}
						}
						else
						{
							*dest_data++ = *scan++;
						}
					}
					break;

				}
				else
					++scan;
			}

			if ( *scan == '<' )
			{
				if ( scan[1] != '/' )
				{
					PX_ASSERT(mOpenCount>0);
					mOpenCount--;
				}
				if ( dest_data )
				{
					*dest_data = 0;
				}
				else
				{
					*scan = 0;
				}

				scan++; // skip it..

				if ( *data == 0 ) data = 0;

				if ( !iface->processElement(element, data, attr, mLineNo) )
				{
					mError = "User aborted the parsing process";
					return 0;
				}

				pushElement(element);

				// check for the comment use case...
				if ( scan[0] == '!' && scan[1] == '-' && scan[2] == '-' )
				{
					scan+=3;
					while ( *scan && *scan == ' ' )
						++scan;

					char *comment = scan;
					char *comment_end = strstr(scan, "-->");
					if ( comment_end )
					{
						*comment_end = 0;
						scan = comment_end+3;
						if( !iface->processComment(comment) )
						{
							mError = "User aborted the parsing process";
							return 0;
						}
					}
				}
				else if ( *scan == '/' )
				{
					scan = processClose(scan, iface, isError);
					if( scan == NULL ) 
					{
						return NULL;
					}
				}
			}
			else
			{
				mError = "Data portion of an element wasn't terminated properly";
				return NULL;
			}
		}

		if ( mOpenCount < MIN_CLOSE_COUNT )
		{
			scan = readData(scan);
		}

		return scan;
	}

	char *processClose(char *scan, FastXml::Callback *iface,bool &isError)
	{
		const char *start = popElement(), *close = start;
		if( scan[1] != '>')
		{
			scan++;
			close = scan;
			while ( *scan && *scan != '>' ) scan++;
			*scan = 0;
		}

		if( 0 != strcmp(start, close) )
		{
			mError = "Open and closing tags do not match";
			return 0;
		}

		if( !iface->processClose(close,mStackIndex,isError) )
		{
			// we need to set the read pointer!
			physx::PxU32 offset = (physx::PxU32)(mReadBufferEnd-scan)-1;
			physx::PxU32 readLoc = mLastReadLoc-offset;
			mFileBuf->seek(readLoc);			
			return NULL;
		}
		++scan;

		return scan;
	}

	virtual bool processXml(physx::PxInputData &fileBuf,bool streamFromMemory)
	{
		releaseMemory();
		mFileBuf = &fileBuf;
		mStreamFromMemory = streamFromMemory;
		return processXml(mCallback);
	}

	// if we have finished processing the data we had pending..
	char * readData(char *scan)
	{
		for (physx::PxU32 i=0; i<(mStackIndex+1); i++)
		{
			if ( !mStackAllocated[i] )
			{
				const char *text = mStack[i];
				if ( text )
				{
					physx::PxU32 tlen = (physx::PxU32)strlen(text);
					mStack[i] = (const char *)mCallback->allocate(tlen+1);
					memcpy((void *)mStack[i],text,tlen+1);
					mStackAllocated[i] = true;

				}
			}
		}

		if ( !mStreamFromMemory )
		{
			if ( scan == NULL )
			{
				physx::PxU32 seekLoc = mFileBuf->tell();
				mReadBufferSize = (mFileBuf->getLength()-seekLoc);
			}
			else
			{
				return scan;
			}
		}

		if ( mReadBuffer == NULL )
		{
			mReadBuffer = (char *)mCallback->allocate(mReadBufferSize+1);
		}
		physx::PxU32 offset = 0;
		physx::PxU32 readLen = mReadBufferSize;

		if ( scan )
		{
			offset = (physx::PxU32)(scan - mReadBuffer );
			physx::PxU32 copyLen = mReadBufferSize-offset;
			if ( copyLen )
			{
				PX_ASSERT(scan >= mReadBuffer);
				memmove(mReadBuffer,scan,copyLen);
				mReadBuffer[copyLen] = 0;
				readLen = mReadBufferSize - copyLen;
			}
			offset = copyLen;
		}

		physx::PxU32 readCount = mFileBuf->read(&mReadBuffer[offset],readLen);

		while ( readCount > 0 )
		{

			mReadBuffer[readCount+offset] = 0; // end of string terminator...
			mReadBufferEnd = &mReadBuffer[readCount+offset];

			const char *scan_ = &mReadBuffer[offset];
			while ( *scan_ )
			{
				if ( *scan_ == '<' && scan_[1] != '/' )
				{
					mOpenCount++;
				}
				scan_++;
			}

			if ( mOpenCount < MIN_CLOSE_COUNT )
			{
				physx::PxU32 oldSize = (physx::PxU32)(mReadBufferEnd-mReadBuffer);
				mReadBufferSize = mReadBufferSize*2;
				char *oldReadBuffer = mReadBuffer;
				mReadBuffer = (char *)mCallback->allocate(mReadBufferSize+1);
				memcpy(mReadBuffer,oldReadBuffer,oldSize);
				mCallback->deallocate(oldReadBuffer);
				offset = oldSize;
				physx::PxU32 readSize = mReadBufferSize - oldSize;
				readCount = mFileBuf->read(&mReadBuffer[offset],readSize);
				if ( readCount == 0 )
					break;
			}
			else
			{
				break;
			}
		}
		mLastReadLoc = mFileBuf->tell();

		return mReadBuffer;
	}
	
	bool processXml(FastXml::Callback *iface)
	{
		bool ret = true;

		const int MAX_ATTRIBUTE = 2048; // can't imagine having more than 2,048 attributes in a single element right?

		mLineNo = 1;

		char *element, *scan = readData(0);

		while( *scan )
		{

			scan = skipNextData(scan);

			if( *scan == 0 ) break;

			if( *scan == '<' )
			{

				if ( scan[1] != '/' )
				{
					PX_ASSERT(mOpenCount>0);
					mOpenCount--;
				}
				scan++;

				if( *scan == '?' ) //Allow xml declarations
				{
					scan++;
				}
				else if ( scan[0] == '!' && scan[1] == '-' && scan[2] == '-' )
				{
					scan+=3;
					while ( *scan && *scan == ' ' )
						scan++;
					char *comment = scan, *comment_end = strstr(scan, "-->");
					if ( comment_end )
					{
						*comment_end = 0;
						scan = comment_end+3;
						if( !iface->processComment(comment) )
						{
							mError = "User aborted the parsing process";
							return false;
						}
					}
					continue;
				}
				else if ( scan[0] == '!' ) //Allow doctype
				{
					scan++;

					//DOCTYPE syntax differs from usual XML so we parse it here

					//Read DOCTYPE
					const char *tag = "DOCTYPE";
					if( !strstr(scan, tag) )
					{
						mError = "Invalid DOCTYPE";
						return false;
					}

					scan += strlen(tag);

					//Skip whites
					while(  CT_SOFT == getCharType(scan) )
						++scan;

					//Read rootElement
					const char *rootElement = scan;
					while( CT_DATA == getCharType(scan) )
						++scan;

					char *endRootElement = scan;

					//TODO: read remaining fields (fpi, uri, etc.)
					while( CT_END_OF_ELEMENT != getCharType(scan++) )
						;

					*endRootElement = 0;

					if( !iface->processDoctype(rootElement, 0, 0, 0) )
					{
						mError = "User aborted the parsing process";
						return false;
					}

					continue; //Restart loop
				}
			}


			if( *scan == '/' )
			{
				bool isError;
				scan = processClose(scan, iface, isError);
				if( !scan )
				{
					if ( isError )
					{
						mError = "User aborted the parsing process";						
					}
					return !isError;
				}
			}
			else
			{
				if( *scan == '?' )
					scan++;
				element = scan;
				physx::PxI32 argc = 0;
				const char *argv[MAX_ATTRIBUTE];
				bool close;
				scan = nextSoftOrClose(scan, close);
				if( close )
				{
					char c = *(scan-1);
					if ( c != '?' && c != '/' )
					{
						c = '>';
					}
					*scan++ = 0;
					bool isError;
					scan = processClose(c, element, scan, argc, argv, iface, isError);
					if ( !scan )
					{
						if ( isError )
						{
							mError = "User aborted the parsing process";							
						}
						return !isError;
					}
				}
				else
				{
					if ( *scan == 0 )
					{
						return ret;
					}

					*scan = 0; // place a zero byte to indicate the end of the element name...
					scan++;

					while ( *scan )
					{
						scan = skipNextData(scan); // advance past any soft seperators (tab or space)

						if ( getCharType(scan) == CT_END_OF_ELEMENT )
						{
							char c = *scan++;
							if( '?' == c )
							{
								if( '>' != *scan ) //?>
								{
								    PX_ASSERT(0);
									return false;
								}

								scan++;
							}
							bool isError;
							scan = processClose(c, element, scan, argc, argv, iface, isError);
							if ( !scan )
							{
								if ( isError )
								{
									mError = "User aborted the parsing process";
								}
								return !isError;
							}
							break;
						}
						else
						{
							if( argc >= MAX_ATTRIBUTE )
							{
								mError = "encountered too many attributes";
								return false;
							}
							argv[argc] = scan;
							scan = nextSep(scan);  // scan up to a space, or an equal
							if( *scan )
							{
								if( *scan != '=' )
								{
									*scan = 0;
									scan++;
									while ( *scan && *scan != '=' ) scan++;
									if ( *scan == '=' ) scan++;
								}
								else
								{
									*scan=0;
									scan++;
								}

								if( *scan ) // if not eof...
								{
									scan = skipNextData(scan);
									if( *scan == '"' )
									{
										scan++;
										argc++;
										argv[argc] = scan;
										argc++;
										while ( *scan && *scan != 34 ) scan++;
										if( *scan == '"' )
										{
											*scan = 0;
											scan++;
										}
										else
										{
											mError = "Failed to find closing quote for attribute";
											return false;
										}
									}
									else
									{
										//mError = "Expected quote to begin attribute";
										//return false;
										// PH: let's try to have a more graceful fallback
										argc--;
										while(*scan != '/' && *scan != '>' && *scan != 0)
											scan++;
									}
								}
							} //if( *scan )
						} //if ( mTypes[*scan]
					} //if( close )
				} //if( *scan == '/'
			} //while( *scan )
		}

		if( mStackIndex )
		{
			mError = "Invalid file format";
			return false;
		}

		return ret;
	}

	const char *getError(physx::PxI32 &lineno)
	{
		const char *ret = mError;
		lineno = mLineNo;
		mError = 0;
		return ret;
	}

	virtual void release(void)
	{
		Callback *c = mCallback;	// get the user allocator interface
		MyFastXml *f = this;		// cast the this pointer
		f->~MyFastXml();			// explicitely invoke the destructor for this class
		c->deallocate(f);			// now free up the memory associated with it.
	}

private:	
	virtual ~MyFastXml(void)
	{
		releaseMemory();
	}

	PX_INLINE void releaseMemory(void)
	{
		mFileBuf = NULL;
		mCallback->deallocate(mReadBuffer);
		mReadBuffer = NULL;
		mStackIndex = 0;
		mReadBufferEnd = NULL;
		mOpenCount = 0;
		mLastReadLoc = 0;
		mError = NULL;
		for (physx::PxU32 i=0; i<(mStackIndex+1); i++)
		{
			if ( mStackAllocated[i] )
			{
				mCallback->deallocate((void *)mStack[i]);
				mStackAllocated[i] = false;
			}
			mStack[i] = NULL;
		}
	}

	PX_INLINE CharType getCharType(char* scan) const
	{
		return mTypes[(unsigned char)(*scan)];
	}
	
	PX_INLINE char *nextSoftOrClose(char *scan, bool &close)
	{
		while ( *scan && getCharType(scan) != CT_SOFT && *scan != '>' ) scan++;
		close = *scan == '>';
		return scan;
	}

	PX_INLINE char *nextSep(char *scan)
	{
		while ( *scan && getCharType(scan) != CT_SOFT && *scan != '=' ) scan++;
		return scan;
	}

	PX_INLINE char *skipNextData(char *scan)
	{
		// while we have data, and we encounter soft seperators or line feeds...
		while ( *scan && (getCharType(scan) == CT_SOFT || getCharType(scan) == CT_END_OF_LINE) )
		{
			if ( *scan == '\n' ) mLineNo++;
			scan++;
		}
		return scan;
	}

	void pushElement(const char *element)
	{
		PX_ASSERT( mStackIndex < (physx::PxU32)MAX_STACK );
		if( mStackIndex < (physx::PxU32)MAX_STACK )
		{
			if ( mStackAllocated[mStackIndex] )
			{
				mCallback->deallocate((void *)mStack[mStackIndex]);
				mStackAllocated[mStackIndex] = false;
			}
			mStack[mStackIndex++] = element;
		}
	}

	const char *popElement(void)
	{
		PX_ASSERT(mStackIndex>0);
		if ( mStackAllocated[mStackIndex] )
		{
			mCallback->deallocate((void*)mStack[mStackIndex]);
			mStackAllocated[mStackIndex] = false;
		}
		mStack[mStackIndex] = NULL;
		return mStackIndex ? mStack[--mStackIndex] : NULL;
	}

	static const int MAX_STACK = 2048;

	CharType mTypes[256];

	physx::PxInputData *mFileBuf;

	char			*mReadBuffer;
	char			*mReadBufferEnd;

	physx::PxU32	mOpenCount;
	physx::PxU32	mReadBufferSize;
	physx::PxU32	mLastReadLoc;

	physx::PxI32 mLineNo;
	const char *mError;
	physx::PxU32 mStackIndex;
	const char *mStack[MAX_STACK+1];
	bool		mStreamFromMemory;
	bool		mStackAllocated[MAX_STACK+1];
	Callback	*mCallback;
};

}

namespace physx
{
namespace shdfnd
{

FastXml * createFastXml(FastXml::Callback *iface)
{
	MyFastXml *m = (MyFastXml *)iface->allocate(sizeof(MyFastXml));
	if ( m )
	{		
		new ( m ) MyFastXml(iface);
	}
	return static_cast< FastXml *>(m);
}

}}
