/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "PxFileRenderDebug.h"
#include "PxFileBuffer.h"
#include "PxProcessRenderDebug.h"
#include "PsArray.h"

#pragma warning(disable:4100)

namespace physx
{
	namespace general_renderdebug4
	{

char magic[4] = { 'D', 'E', 'B', 'G' };
#define MAGIC_VERSION 1



struct FrameHeader
{
	void init(void)
	{
		mSeekLocation = 0;
		mItemCount = 0;
		mItemSize = 0;
	}
	PxU32	mSeekLocation;
	PxU32	mItemCount;
	PxU32	mItemSize;
};

struct PrimitiveBatch
{
	PxU32	mPrimitiveType;
	PxU32	mPrimitiveCount;
	PxU32	mPrimitiveIndex;
};

struct TypeHeader
{
	TypeHeader(void)
	{
		mSeekLocation = 0;
		mDisplayType = PxProcessRenderDebug::DT_LAST;
		mPrimitiveCount = 0;
	}

	void init(PxProcessRenderDebug::DisplayType type)
	{
		mSeekLocation = 0;
		mDisplayType = type;
		mPrimitiveCount = 0;
	}

	PxU32	mSeekLocation;
	PxU32	mDisplayType;
	PxU32	mPrimitiveCount;
};


class FileRenderDebug : public PxFileRenderDebug, public UserAllocated
{
public:
	FileRenderDebug(const char *fileName,bool readAccess,bool echoLocally)
	{
		mLastDisplayType = PxProcessRenderDebug::DT_LAST;
		mData = NULL;
		mPrimitives = NULL;
		mPrimitiveCount = 0;
		mCurrentFrame = 0;
		mFrameItemCount = 0;
		mFrameStart = 0;
		mFrameCount = 0;
		mReadAccess = readAccess;
		mEchoLocally = NULL;

		if ( echoLocally )
		{
			mEchoLocally = createProcessRenderDebug();
		}
		mFileBuffer = PX_NEW(PxFileBuffer)(fileName, readAccess ? PxFileBuf::OPEN_READ_ONLY : PxFileBuf::OPEN_WRITE_ONLY);
		if ( mFileBuffer->isOpen() )
		{
			if ( mReadAccess )
			{
				char temp[4];
				PxU32 r = mFileBuffer->read(temp,4);
				PxU32 version = mFileBuffer->readDword();
				if ( r == 4 && magic[0] == temp[0] && magic[1] == temp[1] && magic[2] == temp[2] && magic[3] == temp[3] && version == MAGIC_VERSION )
				{
					// it's valid!
					FrameHeader h;
					while ( readFrameHeader(h,*mFileBuffer ) )
					{
						mFrameHeaders.pushBack(h);
					}
					mFrameCount = mFrameHeaders.size();
				}
				else
				{
					delete mFileBuffer;
					mFileBuffer = NULL;
				}
			}
			else
			{
				mFileBuffer->write(magic, 4 );
				mFileBuffer->storeDword(MAGIC_VERSION);
				mFileBuffer->flush();
			}

		}
		else
		{
			delete mFileBuffer;
			mFileBuffer = NULL;
		}

	}

	bool readFrameHeader(FrameHeader &h,PxFileBuffer &fph)
	{
		bool ret = false;

		h.init();
		h.mItemCount = fph.readDword();
		h.mItemSize  = fph.readDword();
		h.mSeekLocation = fph.tellRead();
		if ( h.mItemCount > 0 && h.mItemSize > 0 )
		{
			fph.seekRead(h.mSeekLocation+h.mItemSize);
			ret = true;
		}
		return ret;
	}

	virtual ~FileRenderDebug(void)
    {
		reset();
    	delete mFileBuffer;
		if ( mEchoLocally )
		{
			mEchoLocally->release();
		}
    };

	virtual PxProcessRenderDebug * getEchoLocal(void) const
	{
		return mEchoLocally;
	}

	bool isOk(void) const
	{
		return mFileBuffer ? true : false;
	}

	virtual void processRenderDebug(const DebugPrimitive **dplist,
									PxU32 pcount,
									RenderDebugInterface *iface,
									PxProcessRenderDebug::DisplayType type)
	{
		if ( !mReadAccess )
		{
			PX_ASSERT(pcount);
			if ( mFrameStart == 0 )
			{
				PxU32 pv = 0;
				mFrameStart = mFileBuffer->tellWrite();
				mFileBuffer->storeDword(pv);
				mFileBuffer->storeDword(pv);
			}

			if ( type != mLastDisplayType )
			{
				flushDisplayType(mLastDisplayType);
				beginDisplayType(type);
			}

			mTypeHeaders[type].mPrimitiveCount+=pcount;

			for (PxU32 i=0; i<pcount; i++)
			{
				const DebugPrimitive *dp = dplist[i];
				PxU32 plen = DebugCommand::getPrimtiveSize(*dp);
				mFileBuffer->write(dp,plen);
			}
			mFrameItemCount+=pcount;
		}
		if ( mEchoLocally )
		{
			mEchoLocally->processRenderDebug(dplist,pcount,iface,type);
		}
	}

	void flushDisplayType(DisplayType type)
	{
		if ( type != PxProcessRenderDebug::DT_LAST && mTypeHeaders[type].mDisplayType != PxProcessRenderDebug::DT_LAST )
		{
			PxU32 loc = mFileBuffer->tellWrite();
			mFileBuffer->seekWrite( mTypeHeaders[type].mSeekLocation );
			mFileBuffer->storeDword( mTypeHeaders[type].mDisplayType );
			mFileBuffer->storeDword( mTypeHeaders[type].mPrimitiveCount );
			mFileBuffer->seekWrite( loc );
			mTypeHeaders[type].init(PxProcessRenderDebug::DT_LAST);
			mLastDisplayType = PxProcessRenderDebug::DT_LAST;
		}
	}

	void beginDisplayType(DisplayType type)
	{
		mTypeHeaders[type].init(type);
		mTypeHeaders[type].mSeekLocation = mFileBuffer->tellWrite();
		mFileBuffer->storeDword( mTypeHeaders[type].mDisplayType );
		mFileBuffer->storeDword( mTypeHeaders[type].mPrimitiveCount );
		mLastDisplayType = type;
	}

	virtual void flush(RenderDebugInterface *iface,PxProcessRenderDebug::DisplayType type)
	{
		if ( !mReadAccess )
		{
			flushDisplayType(type);
		}
		if ( mEchoLocally )
		{
			mEchoLocally->flush(iface,type);
		}
	}

	virtual void release(void)
	{
		delete this;
	}

	virtual void flushFrame(RenderDebugInterface *iface)
	{
		if ( mFileBuffer )
		{
			if ( mReadAccess )
			{
				for (PxU32 i=0; i<mPrimitiveBatch.size(); i++)
				{
					PrimitiveBatch &b = mPrimitiveBatch[i];
					processRenderDebug(&mPrimitives[b.mPrimitiveIndex],b.mPrimitiveCount,iface,(PxProcessRenderDebug::DisplayType)b.mPrimitiveType);
					flush(iface,(PxProcessRenderDebug::DisplayType)b.mPrimitiveType);
				}
			}
			else if ( mFrameItemCount )
			{
				PxU32 current = mFileBuffer->tellWrite();
				PxU32 frameLength = (current-mFrameStart)-( sizeof(PxU32)*2 );

				mFileBuffer->seekWrite(mFrameStart);
				mFileBuffer->storeDword(mFrameItemCount);
				mFileBuffer->storeDword(frameLength);
				mFileBuffer->seekWrite(current);
				mFileBuffer->flush();

				mFrameStart = 0;
				mFrameItemCount = 0;
				mFrameCount++;
			}
		}

	}

	virtual PxU32 getFrameCount(void) const
	{
		return mFrameCount;
	}

	void reset(void)
	{
		PX_FREE(mPrimitives);
		PX_FREE(mData);
		mData = NULL;
		mPrimitives = NULL;
	}

	virtual void setFrame(PxU32 frameNo)
	{
		if ( mReadAccess && (frameNo+1) < mFrameCount && (frameNo+1) != mCurrentFrame )
		{
			mPrimitiveBatch.resize(0);

			FrameHeader &h = mFrameHeaders[frameNo];
			mCurrentFrame = frameNo+1;
			reset();
			mPrimitiveCount = h.mItemCount;
			mPrimitives = (const DebugPrimitive **)PX_ALLOC( sizeof(DebugPrimitive *)*mPrimitiveCount, PX_DEBUG_EXP("DebugPrimitive"));
			mData = (PxU8 *)PX_ALLOC(h.mItemSize, PX_DEBUG_EXP("FrameItemSize"));
			mFileBuffer->seekRead(h.mSeekLocation);
			PxU32 bcount = mFileBuffer->read(mData,h.mItemSize);
			if ( bcount == h.mItemSize )
			{

				PxU32 index = 0;

				const PxU8 *scan = mData;

				while ( index < h.mItemCount )
				{
    				PrimitiveBatch b;

					const PxU32 *uscan = (const PxU32 *)scan;
    				b.mPrimitiveType = uscan[0];
    				b.mPrimitiveCount = uscan[1];
    				b.mPrimitiveIndex = index;

					mPrimitiveBatch.pushBack(b);

    				uscan+=2;
    				scan = (const PxU8 *)uscan;

					for (PxU32 i=0; i<b.mPrimitiveCount; i++)
					{
						const DebugPrimitive *prim = (const DebugPrimitive *)scan;
						PX_ASSERT( prim->mCommand >= 0 && prim->mCommand < DebugCommand::LAST );
						mPrimitives[i+index] = prim;
						PxU32 plen = DebugCommand::getPrimtiveSize(*prim);
						scan+=plen;
					}
					index+=b.mPrimitiveCount;
				}
			}
			else
			{
				reset();
			}



		}

	}

private:
	PxFileBuffer	   		*mFileBuffer;
	bool					mReadAccess;
	PxProcessRenderDebug	*mEchoLocally;
	PxU32					mFrameStart;
	PxU32					mFrameItemCount;
	PxU32					mFrameCount;
	PxU32					mCurrentFrame;

	PxU32					mPrimitiveCount;
	const DebugPrimitive	**mPrimitives;
	PxU8					*mData;
	Array< PrimitiveBatch >  mPrimitiveBatch;

	Array< FrameHeader >	mFrameHeaders;

	PxProcessRenderDebug::DisplayType				mLastDisplayType;
	TypeHeader				mTypeHeaders[PxProcessRenderDebug::DT_LAST];
};


PxFileRenderDebug * createFileRenderDebug(const char *fileName,
										  bool readAccess,
										  bool echoLocally)
{
	FileRenderDebug *f = PX_NEW(FileRenderDebug)(fileName,readAccess,echoLocally);
	if (!f->isOk() )
	{
		delete f;
		f = NULL;
	}
	return static_cast< PxFileRenderDebug *>(f);
}



	};
};
