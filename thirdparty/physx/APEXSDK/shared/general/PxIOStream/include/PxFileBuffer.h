/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PX_FILE_BUFFER_H
#define PX_FILE_BUFFER_H

#include "Ps.h"
#include "PxFileBuf.h"
#include "PsFile.h"
#include "PsUserAllocated.h"

namespace physx
{
namespace general_PxIOStream2
{
	using namespace shdfnd;

//Use this class if you want to use your own allocator
class PxFileBufferBase : public PxFileBuf
{
public:
	PxFileBufferBase(const char *fileName,OpenMode mode)
	{
		mOpenMode = mode;
		mFph = NULL;
		mFileLength = 0;
		mSeekRead   = 0;
		mSeekWrite  = 0;
		mSeekCurrent = 0;
		switch ( mode )
		{
			case OPEN_READ_ONLY:
				physx::shdfnd::fopen_s(&mFph,fileName,"rb");
				break;
			case OPEN_WRITE_ONLY:
				physx::shdfnd::fopen_s(&mFph,fileName,"wb");
				break;
			case OPEN_READ_WRITE_NEW:
				physx::shdfnd::fopen_s(&mFph,fileName,"wb+");
				break;
			case OPEN_READ_WRITE_EXISTING:
				physx::shdfnd::fopen_s(&mFph,fileName,"rb+");
				break;
			case OPEN_FILE_NOT_FOUND:
				break;
		}
		if ( mFph )
		{
			fseek(mFph,0L,SEEK_END);
			mFileLength = (physx::PxU32)ftell(mFph);
			fseek(mFph,0L,SEEK_SET);
		}
		else
		{
			mOpenMode = OPEN_FILE_NOT_FOUND;
		}
    }

	virtual						~PxFileBufferBase()
	{
		close();
	}

	virtual void close()
	{
		if( mFph )
		{
			fclose(mFph);
			mFph = 0;
		}
	}

	virtual SeekType isSeekable(void) const
	{
		return mSeekType;
	}

	virtual		PxU32			read(void* buffer, PxU32 size)	
	{
		PxU32 ret = 0;
		if ( mFph )
		{
			setSeekRead();
			ret = (PxU32)::fread(buffer,1,size,mFph);
			mSeekRead+=ret;
			mSeekCurrent+=ret;
		}
		return ret;
	}

	virtual		PxU32			peek(void* buffer, PxU32 size)
	{
		PxU32 ret = 0;
		if ( mFph )
		{
			PxU32 loc = tellRead();
			setSeekRead();
			ret = (PxU32)::fread(buffer,1,size,mFph);
			mSeekCurrent+=ret;
			seekRead(loc);
		}
		return ret;
	}

	virtual		PxU32		write(const void* buffer, PxU32 size)
	{
		PxU32 ret = 0;
		if ( mFph )
		{
			setSeekWrite();
			ret = (PxU32)::fwrite(buffer,1,size,mFph);
			mSeekWrite+=ret;
			mSeekCurrent+=ret;
			if ( mSeekWrite > mFileLength )
			{
				mFileLength = mSeekWrite;
			}
		}
		return ret;
	}

	virtual PxU32 tellRead(void) const
	{
		return mSeekRead;
	}

	virtual PxU32 tellWrite(void) const
	{
		return mSeekWrite;
	}

	virtual PxU32 seekRead(PxU32 loc) 
	{
		mSeekRead = loc;
		if ( mSeekRead > mFileLength )
		{
			mSeekRead = mFileLength;
		}
		return mSeekRead;
	}

	virtual PxU32 seekWrite(PxU32 loc)
	{
		mSeekWrite = loc;
		if ( mSeekWrite > mFileLength )
		{
			mSeekWrite = mFileLength;
		}
		return mSeekWrite;
	}

	virtual void flush(void)
	{
		if ( mFph )
		{
			::fflush(mFph);
		}
	}

	virtual OpenMode	getOpenMode(void) const
	{
		return mOpenMode;
	}

	virtual PxU32 getFileLength(void) const
	{
		return mFileLength;
	}

private:
	// Moves the actual file pointer to the current read location
	void setSeekRead(void) 
	{
		if ( mSeekRead != mSeekCurrent && mFph )
		{
			if ( mSeekRead >= mFileLength )
			{
				::fseek(mFph,0L,SEEK_END);
			}
			else
			{
				::fseek(mFph,(long)mSeekRead,SEEK_SET);
			}
			mSeekCurrent = mSeekRead = (physx::PxU32)::ftell(mFph);
		}
	}
	// Moves the actual file pointer to the current write location
	void setSeekWrite(void)
	{
		if ( mSeekWrite != mSeekCurrent && mFph )
		{
			if ( mSeekWrite >= mFileLength )
			{
				::fseek(mFph,0L,SEEK_END);
			}
			else
			{
				::fseek(mFph,(long)mSeekWrite,SEEK_SET);
			}
			mSeekCurrent = mSeekWrite = (physx::PxU32)::ftell(mFph);
		}
	}


	FILE		*mFph;
	PxU32		mSeekRead;
	PxU32		mSeekWrite;
	PxU32		mSeekCurrent;
	PxU32		mFileLength;
	SeekType	mSeekType;
	OpenMode	mOpenMode;
};

//Use this class if you want to use PhysX memory allocator
class PxFileBuffer: public PxFileBufferBase, public UserAllocated
{
public:
	PxFileBuffer(const char *fileName,OpenMode mode): PxFileBufferBase(fileName, mode) {}
};

}
using namespace general_PxIOStream2;
}

#endif // PX_FILE_BUFFER_H
