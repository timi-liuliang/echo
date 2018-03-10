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


#include "windows/PsWindowsInclude.h"
#include "PsFoundation.h"
#include "PsSocket.h"
#include "PsThread.h"
#include "PsArray.h"
#include "PxMemory.h"

#include <Winsock2.h>
#pragma comment( lib, "Ws2_32" )

namespace physx
{
namespace shdfnd
{

const PxU32 Socket::DEFAULT_BUFFER_SIZE  = 32768;

class SocketImpl : public Thread
{
public:
	SocketImpl();
	virtual ~SocketImpl();

			bool	connect(const char* host, PxU16 port, PxU32 timeout);
			bool	listen(PxU16 port, Socket::Connection *callback);
			void	disconnect();

			void	setBlocking(bool blocking);

	virtual PxU32	write(const PxU8* data, PxU32 length);
	virtual bool	flush();

	PxU32			read(PxU8* data, PxU32 length);

	PX_FORCE_INLINE	bool		isBlocking() const	{ return mIsBlocking; }
	PX_FORCE_INLINE	bool		isConnected() const	{ return mIsConnected; }
	PX_FORCE_INLINE	const char*	getHost() const		{ return mHost; }
	PX_FORCE_INLINE	PxU16		getPort() const		{ return mPort; }

	virtual void execute(void);


protected:
	SOCKET			mSocket;
	SOCKET			mListenSocket;
	const char*		mHost;
	PxU16			mPort;
	bool			mIsConnected;
	bool			mIsBlocking;
	bool			mSocketLayerIntialized;
	bool				mExit;
	Socket::Connection	*mConnection;
};


class BufferedSocketImpl: public SocketImpl
{
public:
	BufferedSocketImpl();
	virtual ~BufferedSocketImpl() {};
	bool flush();
	PxU32 write(const PxU8* data, PxU32 length);

private:
	PxU32	mBufferPos;
	PxU8	mBuffer[Socket::DEFAULT_BUFFER_SIZE];
};

BufferedSocketImpl::BufferedSocketImpl()
	: mBufferPos(0)
	, SocketImpl()
{}


SocketImpl::SocketImpl()
	: mSocket(INVALID_SOCKET)
	, mListenSocket(INVALID_SOCKET)
	, mPort(0)
	, mHost(NULL)
	, mIsConnected(false)
	, mIsBlocking(true)
	, mSocketLayerIntialized(false)
	, mConnection(NULL)
	, mExit(false)
{
	WORD vreq;
	WSADATA wsaData;
	vreq = MAKEWORD(2,2);
	mSocketLayerIntialized = (WSAStartup(vreq, &wsaData) == 0);
}


SocketImpl::~SocketImpl()
{
	if(mSocketLayerIntialized)
		WSACleanup();
}


void SocketImpl::setBlocking(bool blocking)
{
	if (blocking != mIsBlocking)
	{
		mIsBlocking = blocking;
		PxU32 mode = PxU32(mIsBlocking ? 0 : 1);
		ioctlsocket(mSocket, FIONBIO, (u_long*)&mode);
	}
}


bool SocketImpl::flush()
{
	return true;
};

bool SocketImpl::connect(const char* host, PxU16 port, PxU32 timeout)
{
	if(!mSocketLayerIntialized)
		return false;

	sockaddr_in	socketAddress;
	hostent		*hp;

	PxMemSet(&socketAddress, 0, sizeof(sockaddr_in));
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_port = htons(port);

	//get host
	hp = gethostbyname(host);
	if(!hp)
	{
		in_addr a; a.s_addr = inet_addr(host);
		hp = gethostbyaddr((const char*)&a, sizeof(in_addr), AF_INET);
		if(!hp) 
			return false;
	}
	PxMemCopy(&socketAddress.sin_addr, hp->h_addr_list[0], (PxU32)hp->h_length);

	//connect
	mSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mSocket == INVALID_SOCKET) return false;

	bool prevBlockingMode = mIsBlocking;
	setBlocking(false);

	::connect(mSocket, (sockaddr*)&socketAddress, sizeof(socketAddress));
	//Setup select function call to monitor the connect call.
	fd_set writefs;
	fd_set exceptfs;
	FD_ZERO(&writefs);
	FD_ZERO(&exceptfs);
#pragma warning(push)
#pragma warning(disable : 4127 4548)
	FD_SET(mSocket, &writefs);
	FD_SET(mSocket, &exceptfs);
#pragma warning(pop)
	timeval timeout_;
	timeout_.tv_sec = long(timeout / 1000);
	timeout_.tv_usec = long(((timeout % 1000) * 1000));
	int selret = ::select(1, NULL, &writefs, &exceptfs, &timeout_);
	int excepted = FD_ISSET(mSocket, &exceptfs);
	int canWrite = FD_ISSET(mSocket, &writefs);
	if (selret != 1 || excepted || !canWrite)
	{
		disconnect();
		return false;
	}

	setBlocking(prevBlockingMode);
	mIsConnected = true;
	mPort = port;
	mHost = host;
	return true;
}

bool	SocketImpl::listen(PxU16 port, Socket::Connection *callback)
{
	bool ret = false;

	if( mSocketLayerIntialized)
	{
		mListenSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		if ( mListenSocket != INVALID_SOCKET )
		{
			sockaddr_in addr      = {0};
			addr.sin_family       = AF_INET;
			addr.sin_port         = htons(port);
			addr.sin_addr.s_addr  = htonl(INADDR_ANY);
			if(bind(mListenSocket, (sockaddr*)&addr, sizeof(addr)) == 0)
			{
				if(::listen(mListenSocket, SOMAXCONN) == 0)
				{
					mPort = port;
					mConnection = callback;
					Thread::start(Thread::getDefaultStackSize()); // start the listener thread.
					ret = true;
				}
			}
		}
	}
	return ret;
}

void SocketImpl::execute(void)
{
	while(!mExit && !Thread::quitIsSignalled())
	{
		SOCKET clientSocket = accept(mListenSocket,0,0);
		if ( clientSocket != INVALID_SOCKET )
		{
			mSocket = clientSocket; // first client we have encountered
			mIsConnected = true;
			if ( mConnection )
			{
				//mConnection->notifySocketConnection(guid); // notify the server of a new client connection.
			}
		}
		Thread::sleep(0);
	}
	Thread::quit();
}

void SocketImpl::disconnect()
{
	if(mListenSocket != INVALID_SOCKET)
	{
		closesocket(mListenSocket);
		Thread::signalQuit();
		Thread::waitForQuit();
		mListenSocket = INVALID_SOCKET;
	}
	if (mSocket != INVALID_SOCKET)
	{
		WSASendDisconnect(mSocket, NULL);
		closesocket(mSocket);
		mSocket = INVALID_SOCKET;
	}
	mIsConnected = false;
	mPort = 0;
	mHost = NULL;
}


PxU32 SocketImpl::write(const PxU8* data, PxU32 length)
{
	int sent = 0;
	while((sent = send(mSocket, (const char*)data, (PxI32)length, 0)) == SOCKET_ERROR)
	{
		if(WSAGetLastError() != WSAEWOULDBLOCK)
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
	while((bytesReceived = recv(mSocket, (char*)data, (PxI32)length, 0)) < 0  && WSAGetLastError() == WSAENOBUFS)
	{
		PxI32 iBuffSize = 0;
		PxI32 optLen = sizeof(PxI32);

		getsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (char*)&iBuffSize, &optLen);
		iBuffSize = (PxI32)length;

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
		bytesWritten = (PxI32)SocketImpl::write(mBuffer+totalBytesWritten, mBufferPos-totalBytesWritten);
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
		PxI32 currentChunk = PxI32(Socket::DEFAULT_BUFFER_SIZE - mBufferPos);
		PxMemCopy(mBuffer+mBufferPos, data+bytesWritten, (PxU32)currentChunk);
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
