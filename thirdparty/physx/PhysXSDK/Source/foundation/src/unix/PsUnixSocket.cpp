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
  

#include "PsFoundation.h"
#include "PsSocket.h"
#include "PxMemory.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#ifndef PX_PS4
#include <netdb.h>
#include <arpa/inet.h>
#else
#include <netinet/in.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#define INVALID_SOCKET -1

namespace physx
{
namespace shdfnd
{

#ifdef PX_PS4
in_addr_t resolveName(const char* host, int timeout);
#endif

const PxU32 Socket::DEFAULT_BUFFER_SIZE  = 32768;

class SocketImpl
{
public:
	SocketImpl();
	virtual ~SocketImpl();

			bool	connect(const char* host, PxU16 port, PxU32 timeout);
			void	disconnect();
			bool	listen(PxU16 /*port*/, Socket::Connection * /*callback*/)
			{
				return false; // not implemented on this platform
			}

			void	setBlocking(bool blocking);

	virtual PxU32	write(const PxU8* data, PxU32 length);
	virtual bool	flush();

	PxU32			read(PxU8* data, PxU32 length);

	PX_FORCE_INLINE	bool		isBlocking() const	{ return mIsBlocking; }
	PX_FORCE_INLINE	bool		isConnected() const	{ return mIsConnected; }
	PX_FORCE_INLINE	const char*	getHost() const	    { return mHost; }
	PX_FORCE_INLINE	PxU16		getPort() const	    { return mPort; }

protected:
	PxI32				mSocket;
	const char*			mHost;
	PxU16				mPort;
	bool				mIsConnected;
	bool				mIsBlocking;
};


class BufferedSocketImpl: public SocketImpl
{
public:
	BufferedSocketImpl();
	virtual ~BufferedSocketImpl() {}
	bool flush();
	PxU32 write(const PxU8* data, PxU32 length);

private:
	PxU32	mBufferPos;
	PxU8	mBuffer[Socket::DEFAULT_BUFFER_SIZE];
};

BufferedSocketImpl::BufferedSocketImpl()
	: SocketImpl() ,
	  mBufferPos(0)
{}


SocketImpl::SocketImpl()
	: mSocket(INVALID_SOCKET)
	, mHost(NULL)
	, mPort(0)
	, mIsConnected(false)
	, mIsBlocking(true)
{
}


SocketImpl::~SocketImpl()
{
}


void SocketImpl::setBlocking(bool blocking)
{
	if (blocking != mIsBlocking)
	{
		int mode = fcntl(mSocket, F_GETFL, 0);
		if(!blocking)
			mode |= O_NONBLOCK;
		else
			mode &= ~O_NONBLOCK;
		int ret = fcntl(mSocket, F_SETFL, mode);
		if(ret !=  -1)
			mIsBlocking = blocking;
	}
}


bool SocketImpl::flush()
{
	return true;
}


bool SocketImpl::connect(const char* host, PxU16 port, PxU32 timeout)
{
	sockaddr_in	socketAddress;
	PxMemSet(&socketAddress, 0, sizeof(sockaddr_in));
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_port = htons(port);

#ifdef PX_PS4
	socketAddress.sin_addr.s_addr = resolveName(host, timeout);
#else
	//get host
	hostent* hp = gethostbyname(host);
	if(!hp)
	{
		in_addr a; a.s_addr = inet_addr(host);
		hp = gethostbyaddr((const char*)&a, sizeof(in_addr), AF_INET);
		if(!hp) 
			return false;
	}
	PxMemCopy(&socketAddress.sin_addr, hp->h_addr_list[0], hp->h_length);
#endif
	//connect
	mSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mSocket == INVALID_SOCKET) 
		return false;

	bool prevBlockingMode = mIsBlocking;
	setBlocking(false);

	int connectRet = ::connect(mSocket, (sockaddr*)&socketAddress, sizeof(socketAddress));	
	if(connectRet < 0)
	{
		if(errno != EINPROGRESS)
		{
			disconnect();
			return false;
		}

		//Setup select function call to monitor the connect call.
		fd_set writefs;
		fd_set exceptfs;
		FD_ZERO(&writefs);
		FD_ZERO(&exceptfs);
		FD_SET(mSocket, &writefs);
		FD_SET(mSocket, &exceptfs);
		timeval timeout_;
		timeout_.tv_sec = timeout / 1000;
		timeout_.tv_usec = (timeout % 1000) * 1000;
		int selret = ::select(mSocket+1, NULL, &writefs, &exceptfs, &timeout_);
		int excepted = FD_ISSET(mSocket, &exceptfs);
		int canWrite = FD_ISSET(mSocket, &writefs);
		if (selret != 1 || excepted || !canWrite)
		{
			disconnect();
			return false;
		}
		
		// check if we are really connected, above code seems to return 
		// true if host is a unix machine even if the connection was
		// not accepted.
		char buffer;
		if(recv(mSocket, &buffer, 0, 0) < 0)
		{
			if(errno != EWOULDBLOCK)
			{
				disconnect();
				return false;
			}
		}
	}

	setBlocking(prevBlockingMode);
	
#ifdef PX_APPLE
	int noSigPipe = 1;
	setsockopt(mSocket, SOL_SOCKET, SO_NOSIGPIPE, &noSigPipe, sizeof(int));
#endif

	mIsConnected = true;
	mPort = port;
	mHost = host;
	return true;
}


void SocketImpl::disconnect()
{
	if (mSocket != INVALID_SOCKET)
	{
		if(mIsConnected)
		{
			setBlocking(true);
			shutdown(mSocket, SHUT_RDWR);
		}
		close(mSocket);
		mSocket = INVALID_SOCKET;
	}
	mIsConnected = false;
	mPort = 0;
	mHost = NULL;
}


PxU32 SocketImpl::write(const PxU8* data, PxU32 length)
{
	ssize_t sent = 0;
	while((sent = send(mSocket, (const char*)data, (PxI32)length, 0)) < 0)
	{
		if(errno != EWOULDBLOCK)
		{
			mIsConnected = false;
			return 0;
		}
	}
	return (PxU32)sent;
}


PxU32 SocketImpl::read(PxU8* data, PxU32 length)
{
	PxI32 bytesReceived = 0;
	// If out of receive buffer, increase it
	while((bytesReceived = (PxI32)recv(mSocket, (char*)data, (PxI32)length, 0)) < 0 && (errno == ENOBUFS))
	{
		PxI32 iBuffSize = 0;
		socklen_t optLen = sizeof(PxI32);

		getsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (char*)&iBuffSize, &optLen);
		iBuffSize = length;

		// terminate the loop if we cannot increase the buffer size
		if(setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (char*)&iBuffSize, sizeof(PxI32)) != 0)
			break;
	}

	if(bytesReceived <= 0)
	{
		bytesReceived = 0;
		mIsConnected = false;
	}

	return PxU32(bytesReceived);
}


bool BufferedSocketImpl::flush()
{
	PxU32 totalBytesWritten = 0;
	PxI32 bytesWritten = 1;
	while(totalBytesWritten < mBufferPos && bytesWritten > 0)
	{
		bytesWritten = SocketImpl::write(mBuffer+totalBytesWritten, mBufferPos-totalBytesWritten);
		if(bytesWritten > 0)
			totalBytesWritten += bytesWritten;
	}
	bool ret = (totalBytesWritten == mBufferPos);
	mBufferPos = 0;
	return ret;
}


PxU32 BufferedSocketImpl::write(const PxU8* data, PxU32 length)
{
	PxU32 bytesWritten = 0;
	while(length > (Socket::DEFAULT_BUFFER_SIZE - mBufferPos))
	{
		PxI32 currentChunk = Socket::DEFAULT_BUFFER_SIZE - mBufferPos;
		PxMemCopy(mBuffer+mBufferPos, data+bytesWritten, currentChunk);
		mBufferPos = Socket::DEFAULT_BUFFER_SIZE;
		if(!flush())
		{
			disconnect();
			return bytesWritten;
		}
		bytesWritten += currentChunk;
		length -= currentChunk;
	}
	if(length > 0) 
	{
		PxMemCopy(mBuffer+mBufferPos, data+bytesWritten, length);
		bytesWritten += length;
		mBufferPos += length;
	}
	if(mBufferPos == Socket::DEFAULT_BUFFER_SIZE)
	{
		if (!flush())
		{
			disconnect();
			return bytesWritten;
		}
	}
	return bytesWritten;
}

Socket::Socket( bool inIsBuffering )
{
	if ( inIsBuffering )
	{
		void* mem = PX_ALLOC(sizeof(BufferedSocketImpl), PX_DEBUG_EXP("BufferedSocketImpl"));
		mImpl = PX_PLACEMENT_NEW(mem, BufferedSocketImpl)();
	}
	else
	{
		void* mem = PX_ALLOC(sizeof(SocketImpl), PX_DEBUG_EXP("SocketImpl"));
		mImpl = PX_PLACEMENT_NEW(mem, SocketImpl)();
	}
}

Socket::~Socket()
{
	mImpl->flush();
	mImpl->disconnect();
	mImpl->~SocketImpl();
	PX_FREE(mImpl);
}


bool Socket::connect(const char* host, PxU16 port, PxU32 timeout)
{
	return mImpl->connect(host, port, timeout);
}

bool Socket::listen(PxU16 port, Connection *callback)
{
	return mImpl->listen(port, callback);
}


void Socket::disconnect()
{
	mImpl->disconnect();
}


bool Socket::isConnected() const
{
	return mImpl->isConnected();
}


const char* Socket::getHost() const
{
	return mImpl->getHost();
}


PxU16 Socket::getPort() const
{
	return mImpl->getPort();
}


bool Socket::flush()
{
	if(!mImpl->isConnected())
		return false;
	return mImpl->flush();
}


PxU32 Socket::write(const PxU8* data, PxU32 length)
{
	if(!mImpl->isConnected())
		return 0;
	return mImpl->write(data, length);
}


PxU32 Socket::read(PxU8* data, PxU32 length)
{
	if(!mImpl->isConnected())
		return 0;
	return mImpl->read(data, length);
}


void Socket::setBlocking(bool blocking)
{
	if(!mImpl->isConnected())
		return;
	mImpl->setBlocking(blocking);
}


bool Socket::isBlocking() const
{
	if(!mImpl->isConnected())
		return true;
	return mImpl->isBlocking();
}

} // namespace shdfnd
} // namespace physx
