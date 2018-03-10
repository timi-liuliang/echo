/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PX_FILE_BUF_H
#define PX_FILE_BUF_H

#include "foundation/Px.h"

/** \addtogroup foundation
  @{
*/

namespace physx
{

namespace general_PxIOStream2
{

PX_PUSH_PACK_DEFAULT

/**
\brief Callback class for data serialization.

The user needs to supply an PxFileBuf implementation to a number of methods to allow the SDK to read or write
chunks of binary data. This allows flexibility for the source/destination of the data. For example the PxFileBuf
could store data in a file, memory buffer or custom file format.

\note It is the users responsibility to ensure that the data is written to the appropriate offset.

*/
class PxFileBuf
{
public:

	enum EndianMode
	{
		ENDIAN_NONE		= 0, // do no conversion for endian mode
		ENDIAN_BIG		= 1, // always read/write data as natively big endian (Power PC, etc.)
		ENDIAN_LITTLE	= 2, // always read/write data as natively little endian (Intel, etc.) Default Behavior!
	};

	PxFileBuf(EndianMode mode=ENDIAN_LITTLE)
	{
		setEndianMode(mode);
	}

	virtual ~PxFileBuf(void)
	{

	}

	/**
	\brief Declares a constant to seek to the end of the stream.
	*
	* Does not support streams longer than 32 bits
	*/
	static const PxU32 STREAM_SEEK_END=0xFFFFFFFF;

	enum OpenMode
	{
		OPEN_FILE_NOT_FOUND,
		OPEN_READ_ONLY,					// open file buffer stream for read only access
		OPEN_WRITE_ONLY,				// open file buffer stream for write only access
		OPEN_READ_WRITE_NEW,			// open a new file for both read/write access
		OPEN_READ_WRITE_EXISTING		// open an existing file for both read/write access
	};

	virtual OpenMode	getOpenMode(void) const  = 0;

	bool isOpen(void) const
	{
		return getOpenMode()!=OPEN_FILE_NOT_FOUND;
	}

	enum SeekType
	{
		SEEKABLE_NO			= 0,
		SEEKABLE_READ		= 0x1,
		SEEKABLE_WRITE		= 0x2,
		SEEKABLE_READWRITE 	= 0x3
	};

	virtual SeekType isSeekable(void) const = 0;

	void	setEndianMode(EndianMode e)
	{
		mEndianMode = e;
		if ( (e==ENDIAN_BIG && !isBigEndian() ) ||
			 (e==ENDIAN_LITTLE && isBigEndian() ) )
		{
			mEndianSwap = true;
		}
 		else
		{
			mEndianSwap = false;
		}
	}

	EndianMode	getEndianMode(void) const
	{
		return mEndianMode;
	}

	virtual PxU32 getFileLength(void) const = 0;

	/**
	\brief Seeks the stream to a particular location for reading
	*
	* If the location passed exceeds the length of the stream, then it will seek to the end.
	* Returns the location it ended up at (useful if you seek to the end) to get the file position
	*/
	virtual PxU32	seekRead(PxU32 loc) = 0;

	/**
	\brief Seeks the stream to a particular location for writing
	*
	* If the location passed exceeds the length of the stream, then it will seek to the end.
	* Returns the location it ended up at (useful if you seek to the end) to get the file position
	*/
	virtual PxU32	seekWrite(PxU32 loc) = 0;

	/**
	\brief Reads from the stream into a buffer.

	\param[out] mem  The buffer to read the stream into.
	\param[in]  len  The number of bytes to stream into the buffer

	\return Returns the actual number of bytes read.  If not equal to the length requested, then reached end of stream.
	*/
	virtual PxU32	read(void *mem,PxU32 len) = 0;


	/**
	\brief Reads from the stream into a buffer but does not advance the read location.

	\param[out] mem  The buffer to read the stream into.
	\param[in]  len  The number of bytes to stream into the buffer

	\return Returns the actual number of bytes read.  If not equal to the length requested, then reached end of stream.
	*/
	virtual PxU32	peek(void *mem,PxU32 len) = 0;

	/**
	\brief Writes a buffer of memory to the stream

	\param[in] mem The address of a buffer of memory to send to the stream.
	\param[in] len  The number of bytes to send to the stream.

	\return Returns the actual number of bytes sent to the stream.  If not equal to the length specific, then the stream is full or unable to write for some reason.
	*/
	virtual PxU32	write(const void *mem,PxU32 len) = 0;

	/**
	\brief Reports the current stream location read aqccess.

	\return Returns the current stream read location.
	*/
	virtual PxU32	tellRead(void) const = 0;

	/**
	\brief Reports the current stream location for write access.

	\return Returns the current stream write location.
	*/
	virtual PxU32	tellWrite(void) const = 0;

	/**
	\brief	Causes any temporarily cached data to be flushed to the stream.
	*/
	virtual	void	flush(void) = 0;

	/**
	\brief	Close the stream.
	*/
	virtual void close(void) {}

	void release(void)
	{
		delete this;
	}

    static PX_INLINE bool isBigEndian()
     {
       PxI32 i = 1;
        return *((char*)&i)==0;
    }

    PX_INLINE void swap2Bytes(void* _data) const
    {
		char *data = (char *)_data;
		char one_byte;
		one_byte = data[0]; data[0] = data[1]; data[1] = one_byte;
    }

    PX_INLINE void swap4Bytes(void* _data) const
    {
		char *data = (char *)_data;
		char one_byte;
		one_byte = data[0]; data[0] = data[3]; data[3] = one_byte;
		one_byte = data[1]; data[1] = data[2]; data[2] = one_byte;
    }

    PX_INLINE void swap8Bytes(void *_data) const
    {
		char *data = (char *)_data;
		char one_byte;
		one_byte = data[0]; data[0] = data[7]; data[7] = one_byte;
		one_byte = data[1]; data[1] = data[6]; data[6] = one_byte;
		one_byte = data[2]; data[2] = data[5]; data[5] = one_byte;
		one_byte = data[3]; data[3] = data[4]; data[4] = one_byte;
    }


	PX_INLINE void storeDword(PxU32 v)
	{
		if ( mEndianSwap )
		    swap4Bytes(&v);

		write(&v,sizeof(v));
	}

	PX_INLINE void storeFloat(PxF32 v)
	{
		if ( mEndianSwap )
			swap4Bytes(&v);
		write(&v,sizeof(v));
	}

	PX_INLINE void storeDouble(PxF64 v)
	{
		if ( mEndianSwap )
			swap8Bytes(&v);
		write(&v,sizeof(v));
	}

	PX_INLINE  void storeByte(PxU8 b)
	{
		write(&b,sizeof(b));
	}

	PX_INLINE void storeWord(PxU16 w)
	{
		if ( mEndianSwap )
			swap2Bytes(&w);
		write(&w,sizeof(w));
	}

	PxU8 readByte(void) 
	{
		PxU8 v=0;
		read(&v,sizeof(v));
		return v;
	}

	PxU16 readWord(void) 
	{
		PxU16 v=0;
		read(&v,sizeof(v));
		if ( mEndianSwap )
		    swap2Bytes(&v);
		return v;
	}

	PxU32 readDword(void) 
	{
		PxU32 v=0;
		read(&v,sizeof(v));
		if ( mEndianSwap )
		    swap4Bytes(&v);
		return v;
	}

	PxF32 readFloat(void) 
	{
		PxF32 v=0;
		read(&v,sizeof(v));
		if ( mEndianSwap )
		    swap4Bytes(&v);
		return v;
	}

	PxF64 readDouble(void) 
	{
		PxF64 v=0;
		read(&v,sizeof(v));
		if ( mEndianSwap )
		    swap8Bytes(&v);
		return v;
	}

private:
	bool		mEndianSwap;	// whether or not the endian should be swapped on the current platform
	EndianMode	mEndianMode;  	// the current endian mode behavior for the stream
};

PX_POP_PACK

}; // end of namespace
using namespace general_PxIOStream2;

namespace general_PxIOStream = general_PxIOStream2;

}; // end of namespace

#endif // PX_FILE_BUF_H
