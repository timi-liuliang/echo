/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PX_MEMORY_BUFFER_H
#define PX_MEMORY_BUFFER_H

#include "Ps.h"
#include "PxFileBuf.h"
#include "PsUserAllocated.h"
#include "PsAlignedMalloc.h"

namespace physx
{
namespace general_PxIOStream2
{
	using namespace shdfnd;

	const physx::PxU32 BUFFER_SIZE_DEFAULT = 4096;

//Use this class if you want to use your own allocator
template<class Allocator>
class PxMemoryBufferBase : public PxFileBuf, public Allocator
{
	void init(const void *readMem, PxU32 readLen)
	{
		mAllocator = this;

		mReadBuffer = mReadLoc = (const PxU8 *)readMem;
		mReadStop   = &mReadLoc[readLen];

		mWriteBuffer = mWriteLoc = mWriteStop = NULL;
		mWriteBufferSize = 0;
		mDefaultWriteBufferSize = BUFFER_SIZE_DEFAULT;

		mOpenMode = OPEN_READ_ONLY;
		mSeekType = SEEKABLE_READ;
	}

	void init(PxU32 defaultWriteBufferSize)
	{
		mAllocator = this;

		mReadBuffer = mReadLoc = mReadStop = NULL;

		mWriteBuffer = mWriteLoc = mWriteStop = NULL;
		mWriteBufferSize = 0;
		mDefaultWriteBufferSize = defaultWriteBufferSize;

		mOpenMode = OPEN_READ_WRITE_NEW;
		mSeekType = SEEKABLE_READWRITE;
	}

public:
	PxMemoryBufferBase(const void *readMem,PxU32 readLen)
	{
		init(readMem, readLen);
    }

	PxMemoryBufferBase(const void *readMem,PxU32 readLen, const Allocator &alloc): Allocator(alloc)
	{
		init(readMem, readLen);
    }

	PxMemoryBufferBase(PxU32 defaultWriteBufferSize = BUFFER_SIZE_DEFAULT)
    {
		init(defaultWriteBufferSize);
	}

	PxMemoryBufferBase(PxU32 defaultWriteBufferSize, const Allocator &alloc): Allocator(alloc)
    {
		init(defaultWriteBufferSize);
	}

	virtual ~PxMemoryBufferBase(void)
	{
		reset();
	}

	void setAllocator(Allocator *allocator)
	{
		mAllocator = allocator;
	}

	void initWriteBuffer(PxU32 size)
	{
		if ( mWriteBuffer == NULL )
		{
			if ( size < mDefaultWriteBufferSize ) size = mDefaultWriteBufferSize;
			mWriteBuffer = (PxU8 *)mAllocator->allocate(size);
			PX_ASSERT( mWriteBuffer );
    		mWriteLoc    = mWriteBuffer;
    		mWriteStop	= &mWriteBuffer[size];
    		mWriteBufferSize = size;
    		mReadBuffer = mWriteBuffer;
    		mReadStop	= &mWriteBuffer[size];
    		mReadLoc    = mWriteBuffer;
		}
    }

	void reset(void)
	{
		mAllocator->deallocate(mWriteBuffer);
		mWriteBuffer = NULL;
		mWriteBufferSize = 0;
		mWriteLoc = NULL;
		mWriteStop = NULL;
		mReadBuffer = NULL;
		mReadStop = NULL;
		mReadLoc = NULL;
    }

	virtual OpenMode	getOpenMode(void) const
	{
		return mOpenMode;
	}


	SeekType isSeekable(void) const
	{
		return mSeekType;
	}

	virtual		PxU32			read(void* buffer, PxU32 size)
	{
		if ( (mReadLoc+size) > mReadStop )
		{
			size = (PxU32)(mReadStop - mReadLoc);
		}
		if ( size != 0 )
		{
			memmove(buffer,mReadLoc,size);
			mReadLoc+=size;
		}
		return size;
	}

	virtual		PxU32			peek(void* buffer, PxU32 size)
	{
		if ( (mReadLoc+size) > mReadStop )
		{
			size = (PxU32)(mReadStop - mReadLoc);
		}
		if ( size != 0 )
		{
			memmove(buffer,mReadLoc,size);
		}
		return size;
	}

	virtual		PxU32		write(const void* buffer, PxU32 size)
	{
		PX_ASSERT( mOpenMode ==	OPEN_READ_WRITE_NEW );
		if ( mOpenMode == OPEN_READ_WRITE_NEW )
		{
    		if ( (mWriteLoc+size) > mWriteStop )
    		    growWriteBuffer(size);
    		memmove(mWriteLoc,buffer,size);
    		mWriteLoc+=size;
    		mReadStop = mWriteLoc;
    	}
    	else
    	{
    		size = 0;
    	}
		return size;
	}

	PX_INLINE const PxU8 * getReadLoc(void) const { return mReadLoc; };
	PX_INLINE void advanceReadLoc(PxU32 len)
	{
		PX_ASSERT(mReadBuffer);
		if ( mReadBuffer )
		{
			mReadLoc+=len;
			if ( mReadLoc >= mReadStop )
			{
				mReadLoc = mReadStop;
			}
		}
	}

	virtual PxU32 tellRead(void) const
	{
		PxU32 ret=0;

		if ( mReadBuffer )
		{
			ret = (PxU32) (mReadLoc-mReadBuffer);
		}
		return ret;
	}

	virtual PxU32 tellWrite(void) const
	{
		return (PxU32)(mWriteLoc-mWriteBuffer);
	}

	virtual PxU32 seekRead(PxU32 loc)
	{
		PxU32 ret = 0;
		PX_ASSERT(mReadBuffer);
		if ( mReadBuffer )
		{
			mReadLoc = &mReadBuffer[loc];
			if ( mReadLoc >= mReadStop )
			{
				mReadLoc = mReadStop;
			}
			ret = (PxU32) (mReadLoc-mReadBuffer);
		}
		return ret;
	}

	virtual PxU32 seekWrite(PxU32 loc)
	{
		PxU32 ret = 0;
		PX_ASSERT( mOpenMode ==	OPEN_READ_WRITE_NEW );
		if ( mWriteBuffer )
		{
    		if ( loc > mWriteBufferSize )
			{
				mWriteLoc = mWriteStop;
    		    growWriteBuffer(loc - mWriteBufferSize);
			}
    		mWriteLoc = &mWriteBuffer[loc];
			ret = (PxU32)(mWriteLoc-mWriteBuffer);
		}
		return ret;
	}

	virtual void flush(void)
	{

	}

	virtual PxU32 getFileLength(void) const
	{
		PxU32 ret = 0;
		if ( mReadBuffer )
		{
			ret = (PxU32) (mReadStop-mReadBuffer);
		}
		else if ( mWriteBuffer )
		{
			ret = (PxU32)(mWriteLoc-mWriteBuffer);
		}
		return ret;
	}

	PxU32	getWriteBufferSize(void) const
	{
		return (PxU32)(mWriteLoc-mWriteBuffer);
	}

	void setWriteLoc(PxU8 *writeLoc)
	{
		PX_ASSERT(writeLoc >= mWriteBuffer && writeLoc < mWriteStop );
		mWriteLoc = writeLoc;
		mReadStop = mWriteLoc;
	}

	const PxU8 * getWriteBuffer(void) const
	{
		return mWriteBuffer;
	}

	/**
	 * Attention: if you use aligned allocator you cannot free memory with PX_FREE macros instead use deallocate method from base
	 */
	PxU8 * getWriteBufferOwnership(PxU32 &dataLen) // return the write buffer, and zero it out, the caller is taking ownership of the memory
	{
		PxU8 *ret = mWriteBuffer;
		dataLen = (PxU32)(mWriteLoc-mWriteBuffer);
		mWriteBuffer = NULL;
		mWriteLoc = NULL;
		mWriteStop = NULL;
		mWriteBufferSize = 0;
		return ret;
	}


	void alignRead(physx::PxU32 a)
	{
		physx::PxU32 loc = tellRead();
		physx::PxU32 aloc = ((loc+(a-1))/a)*a;
		if ( aloc != loc )
		{
			seekRead(aloc);
		}
	}

	void alignWrite(physx::PxU32 a)
	{
		physx::PxU32 loc = tellWrite();
		physx::PxU32 aloc = ((loc+(a-1))/a)*a;
		if ( aloc != loc )
		{
			seekWrite(aloc);
		}
	}

private:

	// default copy constructor forbidden
	PxMemoryBufferBase(const PxMemoryBufferBase&) {}

	// double the size of the write buffer or at least as large as the 'size' value passed in.
	void growWriteBuffer(PxU32 size)
	{
		if ( mWriteBuffer == NULL )
		{
			if ( size < mDefaultWriteBufferSize ) size = mDefaultWriteBufferSize;
			initWriteBuffer(size);
		}
		else
		{
			PxU32 oldWriteIndex = (PxU32) (mWriteLoc - mWriteBuffer);
			PxU32 newSize =	mWriteBufferSize*2;
			PxU32 avail = newSize-oldWriteIndex;
			if ( size >= avail ) newSize = newSize+size;
			PxU8 *writeBuffer = (PxU8 *)mAllocator->allocate(newSize);
			PX_ASSERT( writeBuffer );
			memmove(writeBuffer,mWriteBuffer,mWriteBufferSize);
			mAllocator->deallocate(mWriteBuffer);
			mWriteBuffer = writeBuffer;
			mWriteBufferSize = newSize;
			mWriteLoc = &mWriteBuffer[oldWriteIndex];
			mWriteStop = &mWriteBuffer[mWriteBufferSize];
			PxU32 oldReadLoc = (PxU32)(mReadLoc-mReadBuffer);
			mReadBuffer = mWriteBuffer;
			mReadStop   = mWriteLoc;
			mReadLoc = &mReadBuffer[oldReadLoc];
		}
	}

	const	PxU8	*mReadBuffer;
	const	PxU8	*mReadLoc;
	const	PxU8	*mReadStop;

			PxU8	*mWriteBuffer;
			PxU8	*mWriteLoc;
			PxU8	*mWriteStop;

			PxU32	mWriteBufferSize;
			PxU32	mDefaultWriteBufferSize;
			Allocator	*mAllocator;
			OpenMode	mOpenMode;
			SeekType	mSeekType;

};

class PxMemoryBufferAllocator
{
public:
	PxMemoryBufferAllocator(PxU32 alignment = 0) : alignment(alignment) {}

	virtual void * allocate(PxU32 size)
	{
		switch(alignment)
		{
		case 0:
			return PX_ALLOC(size, PX_DEBUG_EXP("PxMemoryBufferAllocator"));			
		case 16 :
			return physx::AlignedAllocator<16>().allocate(size, __FILE__, __LINE__);			
		case 32 :
			return physx::AlignedAllocator<32>().allocate(size, __FILE__, __LINE__);			
		case 64 :
			return physx::AlignedAllocator<64>().allocate(size, __FILE__, __LINE__);			
		case 128 :
			return physx::AlignedAllocator<128>().allocate(size, __FILE__, __LINE__);			
		default :
			PX_ASSERT(0);
		}
		return NULL;
	}
	virtual void deallocate(void *mem)
	{
		switch(alignment)
		{
		case 0:
			PX_FREE(mem);
			break;
		case 16 :
			physx::AlignedAllocator<16>().deallocate(mem);			
			break;
		case 32 :
			physx::AlignedAllocator<32>().deallocate(mem);
			break;
		case 64 :
			physx::AlignedAllocator<64>().deallocate(mem);
			break;
		case 128 :
			physx::AlignedAllocator<128>().deallocate(mem);
			break;
		default :
			PX_ASSERT(0);
		}
	}
private:
	PxMemoryBufferAllocator& operator=(const PxMemoryBufferAllocator&);

	const PxU32 alignment;
};

//Use this class if you want to use PhysX memory allocator
class PxMemoryBuffer: public PxMemoryBufferBase<PxMemoryBufferAllocator>, public UserAllocated
{
	typedef PxMemoryBufferBase<PxMemoryBufferAllocator> BaseClass;

public:
	PxMemoryBuffer(const void *readMem,PxU32 readLen): BaseClass(readMem, readLen) {}	
	PxMemoryBuffer(const void *readMem,PxU32 readLen, PxU32 alignment): BaseClass(readMem, readLen, PxMemoryBufferAllocator(alignment)) {}

	PxMemoryBuffer(PxU32 defaultWriteBufferSize=BUFFER_SIZE_DEFAULT): BaseClass(defaultWriteBufferSize) {}
	PxMemoryBuffer(PxU32 defaultWriteBufferSize,PxU32 alignment): BaseClass(defaultWriteBufferSize, PxMemoryBufferAllocator(alignment)) {}
};

}
using namespace general_PxIOStream2;
}

#endif // PX_MEMORY_BUFFER_H

