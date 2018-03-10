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
#ifndef PS_RING_BUFFER_H

#define PS_RING_BUFFER_H




#include "foundation/PxSimpleTypes.h"
#include "PsUserAllocated.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

namespace physx
{
	namespace shdfnd
	{

class PxRingBuffer : public UserAllocated
{
public:
	static const PxU32 MIN_PAD=16; // just pad the ring buffer to the end if there are less than 16 bytes to the end after a send

	// possible error codes returned by the ring buffer send/receive routines
	enum ErrorCode
	{
		EC_OK,		// no error
		EC_FAIL,	// general failure case
		EC_MULTI_PART_DONE, 	// indicates that a multi-part packet send completed.
		EC_SEND_DATA_TOO_LARGE,
		EC_MULTI_PART_SEND_PENDING,
		EC_SEND_DATA_EXCEEDS_MAX_BUFFER,
		EC_NO_RECEIVE_PENDING, // tried to acknowledge a receive packet when no recieve was pending.
		EC_PENDING_SEND_BUFFER_FULL // too many buffered sends pending
	};

	// message types.  Applications should uses MT_APP and above.  The otehr send types are reserved for use by the
	// ring buffer code.  MT_NONE indicates no receive message was pending.
	enum MessageType
	{
		MT_NONE,
		MT_PAD,
		MT_MULTI_PART_HEADER,
		MT_MULTI_PART_PACKET,
		MT_APP = 100			// message enum from the application
	};

	PxRingBuffer(void 	*baseAddress,		// the base address of the ring buffer. If this pointer is null, then a buffer will be allocated locally
				PxU32 	bufferSize,		// total size of the ring buffer.
				bool	sendLongPackets=true,
				bool	bufferSends=false,
				PxU32	maxBufferSendSize=1024*512, // don't buffer more than 512k worth of pending sends.
				PxU32 	*writeAddress=NULL,	// (optional) The address in memory of the write index (if NULL, a local variable will be used).  Ths address passed in is usually for when dealing with memory mapped files.
				PxU32 	*readAddress=NULL) 	// (optional) The address in memory of the read index
	{
		init(baseAddress,bufferSize,sendLongPackets,bufferSends,maxBufferSendSize,writeAddress,readAddress);
	}

	PxRingBuffer(void)
	{
		mPendingSendSize		= 0;
		mSendLongPackets		= false;
		mBufferSends			= false;
		mMaxBufferSendSize		= 0;
		mMaxSendSize			= 0;
		mBufferSize 			= 0;
		mBaseAddress			= NULL;
		mLocalWriteAddress  	= 0;
		mLocalReadAddress		= 0;
		mSendPacket				= NULL;
		mReceiveData			= NULL;
	}

	void init(void 	*baseAddress,		// the base address of the ring buffer.
				PxU32 	bufferSize,		// total size of the ring buffer.
				bool	sendLongPackets=true,
				bool	bufferSends=true,
				PxU32	maxBufferSendSize=1024*512, // don't buffer more than 512k worth of pending sends.
				PxU32 	*writeAddress=NULL,	// (optional) The address in memory of the write index (if NULL, a local variable will be used).  Ths address passed in is usually for when dealing with memory mapped files.
				PxU32 	*readAddress=NULL) 	// (optional) The address in memory of the read index
	{
		PX_ASSERT(bufferSize>0);
		mPendingSendSize		= 0;
		mSendLongPackets		= sendLongPackets;
		mBufferSends			= bufferSends;
		mMaxBufferSendSize		= maxBufferSendSize;
		mMaxSendSize			= (bufferSize/2)-(sizeof(PacketHeader)+1); // the maximum sized packet that can be sent is 1/2 the size of the ring buffer.
		mBufferSize 			= bufferSize;
		if ( baseAddress )
		{
			mBaseAddress			= (char *)baseAddress;
			mLocalAlloc = false; // set a flag indicating that we do not own this memory and are not reresponsible for freeing
		}
		else
		{
			mBaseAddress = (char *)PX_ALLOC(bufferSize, PX_DEBUG_EXP("char"));
			mLocalAlloc	= true; // set a flag indicating that we allocated this buffer and, therefore, are responsible for freeing it up.
		}
		mLocalWriteAddress  	= 0;
		mLocalReadAddress		= 0;
		mSendPacket				= NULL;
		mReceiveData			= NULL;
		if ( writeAddress )
		{
			mWriteAddress 	= writeAddress;
		}
		else
		{
			mWriteAddress	= &mLocalWriteAddress;
		}
		if ( readAddress )
		{
			mReadAddress		= readAddress;
		}
		else
		{
			mReadAddress	= &mLocalReadAddress;
		}
	}

	~PxRingBuffer(void)
	{
  		MultiPartPacket *scan = mSendPacket;
		MultiPartPacket *next = scan ? scan->getNext() : NULL;
  		while ( scan )
  		{
  			delete scan;
  			scan = next;
  			if ( scan )
  				next = scan->getNext();
  		}
  		if ( mLocalAlloc )
  		{
  			PX_FREE(mBaseAddress);
  		}
	}

	PX_INLINE PxRingBuffer::ErrorCode canSend(PxU32 len) const
	{
		// the room available is
		PxRingBuffer::ErrorCode ret = EC_OK;

		len+=sizeof(PacketHeader);

		if ( len > (mMaxSendSize+sizeof(PacketHeader)) ) // ok the packet will fit inside the ring buffer..
		{
			ret = PxRingBuffer::EC_SEND_DATA_EXCEEDS_MAX_BUFFER;
		}
		else if ( getWriteAddress() >= getReadAddress() )
		{
			PxU32 avail1 = mBufferSize - getWriteAddress();
			PxU32 avail2 = getReadAddress();
			PxU32 avail = (avail1 > avail2) ? avail1 : avail2;
			if ( len >= avail )
			{
				ret = PxRingBuffer::EC_SEND_DATA_TOO_LARGE;
			}
		}
		else
		{
			PxU32 avail = getReadAddress()-getWriteAddress();
			if ( len >= avail )
			{
				ret = PxRingBuffer::EC_SEND_DATA_TOO_LARGE;
			}
		}

		if ( ret == EC_OK )
		{
			// if writing this packet, will require it to be padded to the end *and*...
			// if the read-address is currently at zero.. we can't write it
			PxU32 remain = mBufferSize - (getWriteAddress()+len); // compute the remainder bytes to the end of the ring buffer
			if ( (getWriteAddress()+len) <= mBufferSize )
			{
				if ( remain <= MIN_PAD )
				{
					if ( getReadAddress() == 0 )
					{
						ret = EC_SEND_DATA_TOO_LARGE;
					}
				}
			}
		}


		return ret;
	}


	PX_INLINE PxRingBuffer::ErrorCode	sendData(PxRingBuffer::MessageType type,const void *data,PxU32 dataLen,bool bufferIfFull)
	{
		PxRingBuffer::ErrorCode ret = PxRingBuffer::EC_FAIL;

		PxU32 packetLen = sizeof(PacketHeader)+dataLen;
		ret = canSend(dataLen);
		if ( ret == PxRingBuffer::EC_OK && mSendPacket == NULL) // since we guarantee in-order delivery, we can't post this message while a multi-part packet is still sending
		{
			writeData(type,data,packetLen);
		}
		else if ( (mSendPacket == NULL && mSendLongPackets) || (mBufferSends && bufferIfFull && (mPendingSendSize+packetLen) < mMaxBufferSendSize ) )
		{
   			mPendingSendSize+=dataLen;
			MultiPartPacket *packet = PX_NEW(MultiPartPacket)(type,data,dataLen,mMaxSendSize,this);
			if ( mSendPacket )
			{
				MultiPartPacket *prev = mSendPacket;
				while ( prev->getNext() )
				{
					prev = prev->getNext();
				}
				if ( prev )
				{
					prev->setNext(packet);
				}
			}
			else
			{
				mSendPacket = packet;
			}
			ret = EC_OK; // processed
		}
		else if ( ret == EC_OK )
		{
			ret = mBufferSends ? EC_PENDING_SEND_BUFFER_FULL : EC_MULTI_PART_SEND_PENDING;
		}

		return ret;
	}

	PX_INLINE PxRingBuffer::MessageType receiveData(const void *&data,PxU32 &data_len)
	{
		PxRingBuffer::MessageType ret = PxRingBuffer::MT_NONE;

		if ( mReceiveData && mReceiveData->isFinished() ) // if we have a multi-part message already pending to be received....
		{
			ret = mReceiveData->getMessageType();		// set the return message type
			data = mReceiveData->getData();				// set the return message data pointer
			data_len = mReceiveData->getMessageSize();		// set the return message data length
			PX_ASSERT(data_len!=0);
		}
		else
		{
			// while we have data to receive, and that data isn't padding...
    		do
    		{
    			if ( getReadAddress() == getWriteAddress() ) break; // if there is nothing to read, then exit.
       			PxU32 readAddress = getReadAddress();
    			const PacketHeader *ph = (const PacketHeader *)readData(readAddress,sizeof(PacketHeader));
    			data_len = ph->getDataLen() - sizeof(PacketHeader);
       			data = readData(readAddress,data_len);
       			ret = ph->getMessageType();

       			switch ( ret )
       			{
       				case PxRingBuffer::MT_PAD:
       					acknowledgeInternal();  // if it is padding, then acknowledge it and move on to the data
       					break;
       				case PxRingBuffer::MT_MULTI_PART_HEADER:
       					// if it is a multi-part message header, then allocate it.
       					{
							PX_ASSERT( mReceiveData == NULL );
       						const MultiPartPacketHeader *pph = (const MultiPartPacketHeader *)data;
       						mReceiveData = PX_NEW(MultiPartData)(pph);
       						acknowledgeInternal();
       						ret = MT_NONE;
       					}
       					break;
       				case PxRingBuffer::MT_MULTI_PART_PACKET:
       					{
       						acknowledgeInternal();
       						PX_ASSERT(mReceiveData);
       						bool finished = mReceiveData->process(data,data_len);
       						if ( finished )
       						{
       							ret = mReceiveData->getMessageType();
       							data = mReceiveData->getData();
       							data_len = mReceiveData->getMessageSize();
								PX_ASSERT(data_len != 0 );
       						}
       						else
       						{
       							ret = MT_NONE;
       						}
       					}
       					break;
					case MT_APP:
					case MT_NONE:
					default:
						break;
       			}
       		} while ( ret == PxRingBuffer::MT_PAD );
       		if ( ret != MT_NONE )
       		{
       			PX_ASSERT(ret==MT_APP);
				PX_ASSERT(data_len!=0);
       		}
       	}

		return ret;
	}



	PX_INLINE PxRingBuffer::ErrorCode	receiveAcknowledge(void)
	{
		PxRingBuffer::ErrorCode ret = PxRingBuffer::EC_NO_RECEIVE_PENDING;


		if ( mReceiveData ) // if we are acknowledging the receipt of a multi-part message, then free it up.
		{
			PX_ASSERT( mReceiveData->isFinished() );
			ret = EC_OK;
			delete mReceiveData;
			mReceiveData = NULL;
		}
		else if ( getReadAddress() != getWriteAddress() )
		{
			acknowledgeInternal();
			ret = PxRingBuffer::EC_OK;
		}

		return ret;
	}

	bool	pumpPendingSends(void) // give up a time slice to pump pending multi-part packet sends.
	{
		if ( mSendPacket )
		{
			ErrorCode ret = mSendPacket->process();
			if ( ret == EC_MULTI_PART_DONE )
			{
				MultiPartPacket *next = mSendPacket->getNext();
				PxU32 sentLen = mSendPacket->getMessageSize();
				mPendingSendSize-=sentLen;
				delete mSendPacket;
				mSendPacket = next;
			}
		}
		return mSendPacket ? true : false;
	}

	PX_INLINE bool	isSendBufferEmpty(void) const
	{
		return getWriteAddress() == getReadAddress();
	}

	PX_INLINE PxU32	getPendingSendSize(void) const // reports the amount of memory consumed by pending send messages.
	{
		return mPendingSendSize;
	}

	PX_INLINE bool hasSendsPending(void) const
	{
		return mSendPacket ? true : false;
	}

private:
	void acknowledgeInternal(void)
	{
		PxU32 readAddress = getReadAddress();
		const PacketHeader *ph = (const PacketHeader *)readData(readAddress,sizeof(PacketHeader));
		
		if ( ph->isPadded() ) // if padded to the end, just wrap..
		{
			setReadAddress(0);
		}
		else if ( (ph->getDataLen()+getReadAddress()) < mBufferSize )
		{
			setReadAddress( getReadAddress()+ph->getDataLen() );
		}
		else
		{
			setReadAddress( ph->getDataLen() - (mBufferSize-getReadAddress())); // new read address is the remainder..
		}
	}

	PX_INLINE PxRingBuffer::ErrorCode	sendDataInternal(PxRingBuffer::MessageType type,const void *data,PxU32 dataLen)
	{
		PxRingBuffer::ErrorCode ret = PxRingBuffer::EC_FAIL;

		assert( type != MT_APP );

		PxU32 packetLen = sizeof(PacketHeader)+dataLen;
		ret = canSend(dataLen);
		if ( ret == PxRingBuffer::EC_OK ) // since we guarentee in-order delivery, we can't post this message while a multi-part packet is still sending
		{
			writeData(type,data,packetLen);
		}

		return ret;
	}

	class MultiPartPacketHeader
	{
		public:
		PxRingBuffer::MessageType	mMessageType;
		PxU32						mMessageSize;
	};

	class MultiPartData : public UserAllocated
	{
	public:
		MultiPartData(const MultiPartPacketHeader *ph)
		{
			mMessageType = ph->mMessageType;
			mMessageSize = ph->mMessageSize;
			mData = (char *)PX_ALLOC(mMessageSize, PX_DEBUG_EXP("char"));
			mDataWrite = mData;
			mDataLen = mMessageSize;
		}

		~MultiPartData(void)
		{
			PX_FREE(mData);
		}

		bool process(const void *data,PxU32 dataLen)
		{
			PX_ASSERT( dataLen <= mDataLen );
			PX_ASSERT( mDataWrite );
			memcpy(mDataWrite,data,dataLen);
			mDataWrite+=dataLen;
			mDataLen-=dataLen;
			if ( mDataLen == 0 )
			{
				mDataWrite = NULL;
			}
			return mDataLen == 0;
		}

		bool isFinished(void) const
		{
			return mDataLen == 0;
		}

		PxRingBuffer::MessageType getMessageType(void) const { return mMessageType; }
		const void * getData(void) const { return mData; }
		PxU32 getMessageSize(void) const { return mMessageSize; }

	private:
		PxRingBuffer::MessageType	mMessageType;
		PxU32						mMessageSize;
		char						*mData;
		char						*mDataWrite;
		PxU32						mDataLen;
	};

	class MultiPartPacket : public UserAllocated
	{
		public:
		MultiPartPacket(PxRingBuffer::MessageType type,
						const void *data,
						PxU32 dataLen,
						PxU32 maxSendSize,
						PxRingBuffer *parent)
		{
			mNext		= NULL;
			mMessageType=type;
			mMaxSendSize = maxSendSize;
			mMessageSize = dataLen;
			mParent		= parent;
			mData		= (char *)PX_ALLOC(dataLen, PX_DEBUG_EXP("char"));
			mSendData	= mData; // current send data address pointer.
			memcpy(mData,data,dataLen);
			mDataLen	= dataLen;
			mHeaderSent = false;
			mErrorCode = EC_OK;
		}

		ErrorCode sendHeader(void)
		{
			if ( !mHeaderSent && (mParent->canSend(sizeof(MultiPartPacketHeader)) == EC_OK) )
			{
    			MultiPartPacketHeader m;
    			m.mMessageType 	= mMessageType;
    			m.mMessageSize 	= mDataLen;
    			mErrorCode = mParent->sendDataInternal(MT_MULTI_PART_HEADER,&m,sizeof(MultiPartPacketHeader));
    			assert( mErrorCode == EC_OK );
    			mHeaderSent = true;
    		}
			return mErrorCode;
		}

		~MultiPartPacket(void)
		{
			PX_FREE(mData);
		}

		ErrorCode process(void)
		{
			if ( !mHeaderSent )
			{
				sendHeader();
			}
			else if ( mSendData ) // if we still have data to send.
			{
				PxU32 sendLen = mDataLen;
				if ( sendLen > mMaxSendSize )
					sendLen = mMaxSendSize;
				if ( mParent->canSend(sendLen) == EC_OK )
				{
					mErrorCode = mParent->sendDataInternal(PxRingBuffer::MT_MULTI_PART_PACKET,mSendData,sendLen);
					assert( mErrorCode == EC_OK );
					mSendData+=sendLen;
					mDataLen-=sendLen;
					if ( mDataLen == 0 )
					{
						mSendData = NULL;
						if ( mErrorCode == EC_OK )
						{
							mErrorCode = EC_MULTI_PART_DONE;
						}
					}
				}
			}
			return mErrorCode;
		}

		ErrorCode getErrorCode(void) const
		{
			return mErrorCode;
		}

		PxU32	getMessageSize(void) const { return mMessageSize; }

		MultiPartPacket * getNext(void) const { return mNext; }
		void				setNext(MultiPartPacket *p) { mNext = p; }

		private:
		MultiPartPacket	*mNext;
		PxRingBuffer	*mParent;
		MessageType		mMessageType;
		PxU32			mMaxSendSize;
		char			*mSendData;
		PxU32			mMessageSize;
		PxU32			mDataLen;
		char			*mData;
		ErrorCode		mErrorCode;		// current error state
		bool			mHeaderSent;	// whether or not the header packet has been sent yet.
	};

	class PacketHeader
	{
		public:

		bool isPadded(void) const
		{
			return (mDataType&0x80000000) ? true : false;
		}

		MessageType getMessageType(void) const
		{
			return (MessageType)(mDataType&0x7FFFFFFF);
		}

		PxU32 getDataLen(void) const
		{
			return mDataLen;
		}

		void init(PxU32 dataLen,PxU32 messageType,PxU32 remain)
		{
			mDataLen = dataLen;
			mDataType = messageType;
			if ( remain <= MIN_PAD )
				mDataType|=0x80000000;
		}
		private:
		PxU32	mDataLen;
		PxU32	mDataType;
	};

	// Write data into this ring buffer.  If it doesn't wrap, just copy it.
	// if it wraps, then copy the first part up to the end, and roll over the rest to the beginning of the ring buffer.
	PX_INLINE void writeData(PxRingBuffer::MessageType type,const void *_data,PxU32 packetLen)
	{
		PxU32 writeAddress = getWriteAddress();

		PX_ASSERT(writeAddress < (mBufferSize-sizeof(PacketHeader)));

		const char *data = (const char *)_data;
		char *dest = &mBaseAddress[writeAddress];

		PxU32 dataLen = packetLen-sizeof(PacketHeader);
		if ( (writeAddress+packetLen) <= mBufferSize )
		{
			PxU32 remain = mBufferSize - (writeAddress+packetLen); // compute the remainder bytes to the end of the ring buffer
			PacketHeader *ph = (PacketHeader *)dest;
			ph->init(packetLen,type,remain);
			dest+=sizeof(PacketHeader);
			memcpy(dest,data,dataLen);
			writeAddress+=packetLen;
			if ( writeAddress >= mBufferSize || ph->isPadded() ) // pad and/or wrap
			{
				writeAddress = 0;
			}
		}
		else
		{
			// pad to the end of the buffer.
			PacketHeader *ph = (PacketHeader *)dest;
			PxU32 padLen = mBufferSize-writeAddress;
			ph->init(padLen,MT_PAD,0);
			writeAddress = 0;
			dest = mBaseAddress;
			ph = (PacketHeader *)dest;
			PxU32 remain = mBufferSize - packetLen;
			ph->init(packetLen,type,remain);
			dest+=sizeof(PacketHeader);
			memcpy(dest,data,dataLen);
			writeAddress = packetLen;
			if ( writeAddress >= mBufferSize || ph->isPadded() )
			{
				writeAddress = 0;
			}
		}

		PX_ASSERT(writeAddress < (mBufferSize-sizeof(PacketHeader)));

		setWriteAddress(writeAddress);

	}

	// read data packet from the ring buffer.

	PX_INLINE const void * readData(PxU32 &readAddress,PxU32 dataLen)
	{
		PX_ASSERT( (readAddress+dataLen) <= mBufferSize ); // must always be contiguous!
		const void *ret = &mBaseAddress[readAddress];
		readAddress+=dataLen; // advance the read address
		return ret;
	}


	// get the current read address in the ring buffer.
	PX_INLINE PxU32 getReadAddress(void) const
	{
		PX_ASSERT( *mReadAddress < mBufferSize );
		return *mReadAddress;
	}

	// Update the current read address in the ring buffer
	PX_INLINE void setReadAddress(PxU32 adr)
	{
		PX_ASSERT( *mReadAddress < mBufferSize );
		*mReadAddress = adr;
		PX_ASSERT( *mReadAddress < mBufferSize );
	}

	// Retrieve the current write address in the ring buffer
	PX_INLINE PxU32 getWriteAddress(void) const
	{
		PX_ASSERT( *mWriteAddress < mBufferSize );
		return *mWriteAddress;
	}

	// Update then current write address in the ring buffer
	PX_INLINE void setWriteAddress(PxU32 adr)
	{
		PX_ASSERT( *mWriteAddress < mBufferSize );
		*mWriteAddress = adr;
		PX_ASSERT( *mWriteAddress < mBufferSize );
	}


	char			*mBaseAddress; 		// base address of the ring buffer.
	PxU32			mBufferSize;		// the total size of the ring buffer
	PxU32			mLocalWriteAddress; //holds the read and write addresses locally if they are not in shared memory
	PxU32			mLocalReadAddress;  // if not used memory mapped files, this holds the local read address
	PxU32			*mWriteAddress;		// a pointer to the write buffer index address; can be in shared memory
	PxU32			*mReadAddress;		// a pointer to the read buffer index address; can be in shared memory
	PxU32			mMaxSendSize;		// the maximum sized packet you can send is 1/4 the size of the ring buffer.
	MultiPartPacket *mSendPacket;
	MultiPartData  	*mReceiveData;
	bool			mSendLongPackets;	// true if we are allowed to send packets longer than 1/2 the ring buffer size.  Causes memory allocations to occur.
	bool			mBufferSends;		// allows sends to be buffered up if the current send buffer (communications channel) is full.
	PxU32			mMaxBufferSendSize;	// The maximum amount of memory allowed to be used to buffer up pending sends.
	PxU32			mPendingSendSize;	// The number of bytes waiting to be sent.
	bool			mLocalAlloc;		// a flag indicating whether or not we allocated the ring buffer and are responsible for freeing it on release
};

} // end of namespace
} // end of namespace 
#endif
